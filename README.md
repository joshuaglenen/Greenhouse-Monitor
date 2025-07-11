# Greenhouse-Monitor
An embedded system for greenhouse climate control, designed to wirelessly monitor temperature and humidity and regulate fan and heater activation based on user-defined thresholds. The device connects to Wi-Fi to display live sensor data, offers manual overrides, and fetches localized weather updates including frost and heatwave alerts.

## Background

### Purpose

This project began as a personal challenge to improve my skills in embedded systems and software development. It required relearning key concepts from my electrical engineering degree and integrating new ones. Initially, the idea was to develop a system that could wirelessly report real-time temperature data from a small greenhouse. Over time, this goal expanded to include a variety of sensor inputs, a local dashboard, and eventually a full-stack solution involving an online interface and data logging system with weather alert integration.
  
### Implementation
  
I chose the ESP32 microcontroller for its built-in Wi-Fi and Bluetooth capabilities. The initial prototype (Fig. 1) included all relevant sensors I had on hand. Over time, I refined the system to a more practical and deployable design, balancing simplicity with essential features. I integrated a local dashboard served via SPIFFS, implemented automated data uploads to ThingSpeak, and developed a lightweight web app backed by an SQL database for persistent logging and remote control.

### Parts List 

The following parts for the second prototype not including the optional soil moisture and water level sensors are listed below.
  <li>HLK_PM01 120VAC 0.2A to 5VDC 0.6A </li>
  <li>3.5A Slow Blow Fuse</li>
  <li>10A Slow Blow Fuse</li>
  <li>1n4148 Diode</li>
  <li>2 2N7000 NPN Transistors</li>
  <li>220, 2x10k, 2x1k Ohm 0.25W Resistors</li>
  <li>0.22uF X2 Capacitor</li>
  <li>22pF Capacitor</li>
  <li>100 Ohm 1W Resistor</li>
  <li>DHT22</li>
  <li>ESP32 Devkit</li>
  
## Methods

### Embedded Programming

The MCU code was developed for the ESP32 DevKit. Libraries used included <ArduinoOTA.h> for over-the-air updates, <WiFiManager.h> and <WiFi.h> for network handling, <WebServer.h> for HTTP interface routing, and <HTTPClient.h> for external data uploads.

### Core features:
<li>Periodic logging of temperature and humidity (every 15 seconds)</li>
<li>Relay control based on thresholds (auto with manual overrides)</li>
<li>User-defined min/max temperature constraints (persistent via Preferences)</li>
<li>Local SPIFFS-based dashboard</li>
<li>External logging to ThingSpeak and later a custom SQL server</li>
<li>OTA updates for easy remote reprogramming</li>

  
### Designing the Prototype 

Prototype 1 was a proof-of-concept breadboard build as seen in figure 1. It included:
<li>Water level and soil moisture sensors</li>
<li>Light level sensor</li>
<li>RGB status LED</li>
<li>Warning buzzer</li>
<li>Real-time clock (RTC) with a shift register</li>
<li>DHT11 sensor (later upgraded to DHT22)</li>
As fan/heater control required AC interfacing, the design transitioned from battery power to mains power. AC wires were fused and isolated from the logic side. A borrowed PSU from a keyboard charger was used to convert AC to 5 VDC.

 ><img src="https://github.com/user-attachments/assets/27479760-3850-4679-929b-592f24989540" width="300" />

Fig 1: Completed Prototype 1
  
### Improving the Prototype 

The second prototype focused on being compact, weather-resistant, and deployable. I repurposed a metal outdoor dual-outlet box and tested for size as shown in figure 2. I desoldered, rewired and resoldered a cleaner and compact circuit and added it to the enclosure. Additionally I updated the circuit diagram as shown in diagram 1 and completed a pcb layout complete with gerber files as in figure 4.

Key changes:
<li>RTC, light sensor, RGB LED, and buzzer removed</li>
<li>Smaller perfboard cut to fit enclosure</li>
<li>Replaced PSU with HLK-PM01 module</li>
<li>Two relays - one for the fan and one for the heater</li>
<li>Fully enclosed and sealed design with passive air access and drain hole at the bottom</li>

><img src="https://github.com/user-attachments/assets/c010b5f1-a3dc-4ce9-8e89-2c99297a522f" width="300" />

Fig 2: Prototype 2 Testing For Fit of Components in Enclosure

>![IMG_1ACED76C-B3CF-402B-A785-0908097FE2AA](https://github.com/user-attachments/assets/e8aa3f8f-5f34-4bb3-8bb0-af1a2303a12f)

Fig 3: Finished Prototype 2

>![IMG](https://github.com/joshuaglenen/Greenhouse-Monitor/blob/main/prototype_2/Prototype_2_Circuit_Diagram.png)

Diagram 1: Prototype 2 Circuit Diagram

![IMG](https://github.com/joshuaglenen/Greenhouse-Monitor/blob/main/prototype_2/PCB.PNG)

Fig. 4: Prototype 2 PCB


### Web App Development

I migrated from a SPIFFS-hosted local dashboard to a Flask-based web app. 

This new app:
<li>Allows users to register a device</li>
<li>Displays live sensor data and historical logs</li>
<li>Fetches weather forecasts via OpenWeatherMap API</li>
<li>Sends the user alerts for frost and heatwave warnings</li>
<li>Offers manual relay control via the internet</li>
The app connects via the ESP32's HTTP server and to an SQL cloud database. Data can be retrieved from the HTTP server directly over a 24-hour period or taken from the SQL or ThingSpeak databases.


## Results

### Testing 

TODO Describe testing protocol and validation steps.
  
### Conclusion 

TODO Summarize performance, limitations, and future directions.
