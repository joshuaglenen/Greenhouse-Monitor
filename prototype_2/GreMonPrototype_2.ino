#include <DHT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>

// === Pin Assignments ===
#define DHTPIN 17
#define DHTTYPE DHT22
#define FAN_PIN 21
#define HEATER_PIN 19
#define WATER_PIN 39 //
#define SOIL_PIN 36  

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
#define LOG_FILE "/log.csv"
#define MAX_ENTRIES 8640
int logIndex = 0;
bool fanOverride = true;
DHT dht(DHTPIN, DHTTYPE);

// Example sensor readings
float temp = 24.5;
float hum = 55.2;
int water = 20;
int soil = 75;

// === Request Handlers ===
void handleRoot() {
  File file = SPIFFS.open("/dashboard.html", "r");
  if (!file) {
    server.send(500, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

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

void handleDownloadLog() {
  File file = SPIFFS.open(LOG_FILE, "r");
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open log file in /log route");
    server.send(500, "text/plain", "Failed to open log file");
    return;
  }

  server.streamFile(file, "text/plain");
  file.close();
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

void logData() {
  File file = SPIFFS.open(LOG_FILE, "a");
  if (!file) {
    Serial.println("Failed to open log file for appending.");
    return;
  }
  
  size_t offset = logIndex * 60;
  if (!file.seek(offset, SeekSet)) {
    Serial.println("Seek failed.");
    file.close();
    return;
  }
  String line =  String(temp, 1) + "," +
              String(hum, 1) + "," +
              String(water) + "," +
              String(soil);
  if (line.length() > 59) {
    line = line.substring(0, 59);
  }   
  while (line.length() < 59) line += " ";  // Pad with spaces
  line += "\n";  // Now exactly 60 characters
  file.print(line);
  file.close();

  // Track the number of entries
  logIndex = (logIndex + 1) % MAX_ENTRIES;
  preferences.begin("log", false);
  preferences.putInt("index", logIndex);
  preferences.end();
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
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  if(isnan(temp)||isnan(hum))
  {
    Serial.println("DHT22 Failure");
  }
  if(soilSensors)
  {
  water = analogRead(WATER_PIN);
  soil = analogRead(SOIL_PIN);
  water = map(water, 0, 805, 0, 42.31); //0 to 4cm
  //soilPercent = map(soil, 4095, 530, 0, 100); // up to 100%
  }

}

void initLogFile() {
  if (!SPIFFS.exists(LOG_FILE)) {
    File file = SPIFFS.open(LOG_FILE, "w");
    if (file) {
      String placeholder = "0.0,0.0,0,0\n";
      for (int i = 0; i < MAX_ENTRIES; i++) {
        file.print(placeholder);
      }
      file.close();
      Serial.println("Log file initialized.");
    } else {
      Serial.println("Failed to create log file.");
    }
  }
}

void setup() {
  //debugging
  Serial.begin(115200);

  //settings are loaded and read
  preferences.begin("settings", true); // read-only
  tempMin = preferences.getInt("tempMin", 10); // 10 is the default
  tempMax = preferences.getInt("tempMax", 40);
  logIndex = preferences.getInt("index", 0); // default to 0 if unset
  preferences.end();

 
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //connect to wifi
  wifiManager.autoConnect("Greenhouse-Monitor");
  Serial.print("Connected to Wi-Fi IP: ");
  Serial.println(WiFi.localIP());
  initLogFile();
  ArduinoOTA.setHostname("Greenhouse-Monitor");
  ArduinoOTA.begin();

  //set GPIO
  dht.begin();
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  
  //define routes
  server.on("/", handleRoot);
  server.on("/toggleFan", handleToggleFan);
  server.on("/toggleHeater", handleToggleHeater);
  server.on("/toggleSoil", handleToggleSoil);
  server.on("/data", handleData);
  server.on("/set-constraints", handleSetConstraints);
  server.on("/log", handleDownloadLog);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  //update data every 15 seconds
  if (millis() - lastUpdate > 15000) {
    lastUpdate = millis();
  
    updateSensorReadings();
    logData();
    if(!fanOverride && !heaterOverride) controlClimateAuto();
    if(soilSensors) checkMoisture();
  
    //send data to thingspeak
    String url = "http://api.thingspeak.com/update?api_key=TK9X4Z5W6RQHZ70P";
    url += "&field1=" + String(temp, 1);
    url += "&field2=" + String(hum, 1);
    url += "&field4=" + String(water);
    url += "&field5=" + String(soil);
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("ThingSpeak response: %d\n", httpCode);
    } else {
      Serial.printf("Failed to connect to ThingSpeak: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    
}
}
