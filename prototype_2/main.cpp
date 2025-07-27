#include <DHT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>

// === Pin Assignments ===
#define DHTPIN 17
#define DHTTYPE DHT22
#define FAN_PIN 21
#define HEATER_PIN 19
#define WATER_PIN 39
#define SOIL_PIN 36  
const String webappURL = "https://greenhouse-monitor.onrender.com/";

// === Constraints ===
int tempMin = 10;
int tempMax = 40;
int SOIL_MIN = 30;
int WATER_MIN = 0;

// === Global Variables ===
Preferences preferences;
WebServer server(80);
WiFiManager wifiManager;
unsigned long lastUpdate;
bool heaterOverride = false;
bool soilSensors = false;
bool fanOverride = false;
DHT dht(DHTPIN, DHTTYPE);

// Example sensor readings
float temp = 20;
float hum = 50;
int water = 50;
int soil = 50;

// === Request Handlers ===

void handleSetConstraints() {
  if (server.hasArg("min")) {
    tempMin = server.arg("min").toInt();
  }
  if (server.hasArg("max")) {
    tempMax = server.arg("max").toInt();
  }
  // dont let the user submit unrealistic values
  if(tempMin<0) tempMin = 0; 
  if(tempMax>50) tempMax = 50; 
  if(tempMin>tempMax) tempMin = tempMax -1; 
  
  preferences.begin("settings", false); // read+write
  preferences.putInt("tempMin", tempMin);
  preferences.putInt("tempMax", tempMax);
  preferences.end();

  String response = "Updated constraints:\n";
  response += "Min Temp = " + String(tempMin) + "\n";
  response += "Max Temp = " + String(tempMax);
  server.send(200, "text/plain", response);
}

void handleToggleFan() {
  fanOverride = !fanOverride;
  digitalWrite(FAN_PIN, fanOverride ? HIGH : LOW);
  server.send(200, "text/plain", fanOverride ? "ON" : "OFF");
}

void handleToggleHeater() {
  heaterOverride = !heaterOverride;
  digitalWrite(HEATER_PIN, heaterOverride ? HIGH : LOW);
  server.send(200, "text/plain", heaterOverride ? "ON" : "OFF");
}

void handleToggleSoil() {
  soilSensors = !soilSensors;
  server.send(200, "text/plain", soilSensors ? "ON" : "OFF");
}


void handlemacAddr() {
  String mac = WiFi.macAddress();
  server.send(200, "text/plain", mac);
}


void handleData() {
  //read data
  updateSensorReadings();
  
  // Create JSON response manually
  String json = "{";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"hum\":" + String(hum, 1) + ",";
  json += "\"water\":" + String(water) + ",";
  json += "\"soil\":" + String(soil);
  json += "}";

  server.send(200, "application/json", json);
  Serial.println(json);
}

// === Climate Control (Auto Mode Only) ===
void controlClimateAuto() {
  if (temp > tempMax) {
    Serial.println("Activating FAN!");
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(HEATER_PIN, LOW);
  } else if (temp < tempMin) {
    Serial.println("Activating HEATER!");
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(HEATER_PIN, LOW);
  }
}

void checkMoisture() {
  if (water < WATER_MIN || soil < SOIL_MIN) {
    Serial.println("Warning: Low soil moisture or water level!");
  }
}

// == uses analog read to redefine global variables
void updateSensorReadings()
{
  static int dhtFailures = 0;
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  //dht22 has instability over many cycles so it needs to reboot ocasionally
  if (isnan(t) || isnan(h)) {
    dhtFailures++;
    if (dhtFailures > 10) {
      Serial.println("Too many DHT22 failures, rebooting...");
      ESP.restart();  // Safe reboot
    }
    return;
  }
  dhtFailures = 0;
  temp = t;
  hum = h;
  
  if(soilSensors)
  {
  water = analogRead(WATER_PIN);
  soil = analogRead(SOIL_PIN);
  water = map(water, 0, 805, 0, 42.31); //0 to 4cm
  //soilPercent = map(soil, 4095, 530, 0, 100); // up to 100%
  }

}

void sendServerData() {
  //ping ThingSpeak
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=TK9X4Z5W6RQHZ70P";
  url += "&field1=" + String(temp, 1);
  url += "&field2=" + String(hum, 1);
  url += "&field4=" + String(water);
  url += "&field5=" + String(soil);

  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("ThingSpeak response: %d\n", httpCode);
  http.end();
}

void sendSensorData() {
  
  updateSensorReadings();

  if (!fanOverride && !heaterOverride)
    controlClimateAuto();
  if (soilSensors)
    checkMoisture();

  HTTPClient http;

  http.begin("https://your-render-url.onrender.com/submit-data");
  http.addHeader("Content-Type", "application/json");

  String json = "{\"mac\":\"" + WiFi.macAddress() + 
                "\",\"temp\":24.5,\"hum\":60,\"water\":45,\"soil\":50}";

  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  }

  http.end();
}

void setup() {
  //debugging
  Serial.begin(115200);

  //settings are loaded and read
  preferences.begin("settings", true); // read-only
  tempMin = preferences.getInt("tempMin", 10); // 10 is the default
  tempMax = preferences.getInt("tempMax", 40);
  preferences.end();

  //connect to wifi
  wifiManager.autoConnect("Greenhouse-Monitor");
  Serial.print("Connected to Wi-Fi IP: ");
  Serial.println(WiFi.localIP());
  ArduinoOTA.setHostname("Greenhouse-Monitor");
  ArduinoOTA.begin();


  //set GPIO
  dht.begin();
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  
  //define routes
  server.on("/toggleFan", handleToggleFan);
  server.on("/toggleHeater", handleToggleHeater);
  server.on("/toggleSoil", handleToggleSoil);
  server.on("/data", handleData);
  server.on("/mac-addr", handlemacAddr);
  server.on("/set-constraints", handleSetConstraints);
  server.begin();
  delay(3000);
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  static unsigned long lastServerSend = 0;
  static unsigned long lastSensorSend = 0;
  if (millis() - lastServerSend > 15000) {
    lastServerSend = millis();
    sendServerData();
  }
  if (millis() - lastSensorSend > 5000) {
    lastSensorSend = millis();
    sendSensorData();
  }
}
