//#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>

// === Pin Assignments ===
#define DHTPIN 13
#define DHTTYPE DHT11
#define BUZZER_PIN 16
#define FAN_PIN 18
#define RGB_R 19
#define RGB_G 5
#define RGB_B 23
#define BUTTON_PIN 32
#define BUTTON2_PIN 33
#define LIGHT_PIN 34
#define WATER_PIN 36
#define SOIL_PIN 35
#define ESPLED 2

// === Constraints ===
int tempMin = 10;
int tempMax = 40;
int SOIL_MIN = 50;
int WATER_MIN = 0;
//DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

// === Global Variables ===
Preferences preferences;
WebServer server(80);
WiFiManager wifiManager;
unsigned long lastUpdate;
bool ESPLED_ON = false;
bool manualOverride = false;
bool soilSensors = false;
unsigned long lastButtonPressMO = 0;
unsigned long lastButtonPressSOIL = 0;
const unsigned long debounceDelay = 1000;
#define LOG_FILE "/log.csv"
#define MAX_ENTRIES 8640
int logIndex = 0;

// Example sensor readings
DateTime currenttime = "2025-06-22 21:00:00";
float temp = 24.5;
float hum = 55.2;
float light = 128.4;
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

void handleToggle() {
  manualOverride = !manualOverride;
  digitalWrite(FAN_PIN, manualOverride ? HIGH : LOW);
  digitalWrite(ESPLED, manualOverride ? HIGH : LOW);
  server.send(200, "text/plain", manualOverride ? "ON" : "OFF");
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
  json += "\"time\":\"" + currenttime.timestamp() + "\",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"hum\":" + String(hum, 1) + ",";
  json += "\"light\":" + String(light, 1) + ",";
  json += "\"water\":" + String(water) + ",";
  json += "\"soil\":" + String(soil);
  json += "}";

  server.send(200, "application/json", json);
  Serial.println(json);
}

// === RGB LED Control ===
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
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
  String line = currenttime.timestamp() + "," +
              String(temp, 1) + "," +
              String(hum, 1) + "," +
              String(light, 1) + "," +
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
    setRGB(true, false, false); // Red
  } else if (temp < tempMin) {
    Serial.println("Activating HEATER!");
    digitalWrite(FAN_PIN, LOW);
    setRGB(false, false, true); // Blue
  } else {
    digitalWrite(FAN_PIN, LOW);
  }
}

void checkMoisture() {
  if (water < WATER_MIN || soil < SOIL_MIN) {
    setRGB(true, false, true); // Violet
    ledcAttach(BUZZER_PIN, 1000, 8);
    ledcWriteTone(BUZZER_PIN,1000);
    delay(500);
    ledcWriteTone(BUZZER_PIN,0);
    Serial.println("Warning: Low soil moisture or water level!");
  }
}

// == uses analog read to redefine global variables
void updateSensorReadings()
{
  currenttime = rtc.now();
  temp = 25; //dht.readTemperature();
  hum = 50; //dht.readHumidity();
  light = analogRead(LIGHT_PIN);
  if(soilSensors)
  {
  water = analogRead(WATER_PIN);
  soil = analogRead(SOIL_PIN);
  }

  //convert to real units
  //waterMM = map(water, 6, 348, 0, 40); //0 to 4cm
  //soilPercent = map(soil, 1023, 530, 0, 100); // up to 100%
  float a = 10325.54; float b = 0.553; //tune based on real measured values 
  float voltage = light * (3.3 / 4095.0);
  float resistance = (3.3 - voltage)*10000/voltage;
  light = a / pow(resistance/1000.0, b);  // Rough estimate in lux
  light = 0.0654 * pow(light, 1.05); // corrected
}


// === Button Toggle for Manual Override ===
void checkButtonMO() {
  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastButtonPressMO > debounceDelay) {
    lastButtonPressMO = millis();
    manualOverride = !manualOverride;
    Serial.print("Manual Override: ");
    Serial.println(manualOverride ? "ON" : "OFF");
  }
}

// === Button Toggle for Soil Sensors ===
void checkButtonSoil() {
  if (digitalRead(BUTTON2_PIN) == LOW && millis() - lastButtonPressSOIL > debounceDelay) {
    lastButtonPressSOIL = millis();
    soilSensors = !soilSensors;
    Serial.print("Soil Sensors: ");
    Serial.println(soilSensors ? "ON" : "OFF");
  }
}

void initLogFile() {
  if (!SPIFFS.exists(LOG_FILE)) {
    File file = SPIFFS.open(LOG_FILE, "w");
    if (file) {
      String placeholder = "0000-00-00 00:00:00,0.0,0.0,0.0,0,0\n";
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
  Serial.begin(115200);
  preferences.begin("settings", true); // read-only
  tempMin = preferences.getInt("tempMin", 10); // 10 is the default
  tempMax = preferences.getInt("tempMax", 40);
  logIndex = preferences.getInt("index", 0); // default to 0 if unset
  preferences.end();
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  wifiManager.autoConnect("ESP32-Setup");
  Serial.print("Connected to Wi-Fi IP: ");
  Serial.println(WiFi.localIP());
  initLogFile();

  //define pins
  Wire.begin(21,22);
  //dht.begin();
  rtc.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ESPLED, OUTPUT);
  digitalWrite(ESPLED, LOW);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  setRGB(false, false, false);
  

  //define routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/data", handleData);
  server.on("/set-constraints", handleSetConstraints);
  server.on("/log", handleDownloadLog);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
  checkButtonMO();
  checkButtonSoil();

  //update data every 15 seconds
  if (millis() - lastUpdate > 15000) {
    setRGB(true, true, true); //white-reading
    lastUpdate = millis();
  
    //run logic
    updateSensorReadings();
    logData();
    if(!manualOverride) controlClimateAuto();
    if(soilSensors) checkMoisture();
  
    //send data to server
    String url = "http://api.thingspeak.com/update?api_key=TK9X4Z5W6RQHZ70P";
    url += "&field1=" + String(temp, 1);
    url += "&field2=" + String(hum, 1);
    url += "&field3=" + String(light, 1);
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
