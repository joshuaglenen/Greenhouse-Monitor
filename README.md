# Greenhouse-Monitor
An embedded system for greenhouse climate control, designed to wirelessly monitor temperature and humidity and regulate the temperature and an automated watering system using relay controlled switches that can deliver upto 250VAC 20A (Max 3A for the prototype). The device connects to a [Render hosted web app](https://greenhouse-monitor.onrender.com/) over Wi-Fi to display live sensor data, allow manual overrides, and display localized weather updates including frost and heatwave alerts.

## Background

### Purpose

This project began as a personal challenge to improve my skills in embedded systems and software development. It required relearning key concepts from my electrical engineering degree and integrating new ones. Initially, the idea was to develop a system that could wirelessly report real-time temperature data from a small greenhouse. Over time, this goal expanded to include a variety of sensor inputs, a local dashboard, and eventually a full-stack solution involving an online interface and data logging system with weather alert integration.
  
### Implementation
  
I chose the ESP32 microcontroller for its built-in Wi-Fi and Bluetooth capabilities. The initial prototype (Fig. 1) included all relevant sensors I had on hand. Over time, I refined the system to a more practical and deployable design, balancing simplicity with essential features. I integrated a local dashboard served via SPIFFS, implemented automated data uploads to ThingSpeak, and developed a lightweight web app backed by an SQL database for persistent logging and remote control. Over time I evolved the prototype into something depolyable (Fig. 3) and tested it in a real greenhouse environment. 

## Methods

### Embedded Programming

The MCU code was developed for the ESP32 DevKit. Libraries used included <ArduinoOTA.h> for over-the-air updates, <WiFiManager.h> and <WiFi.h> for network handling, <WebServer.h> for HTTP interface routing, and <HTTPClient.h> for posting data to the webserver and reading back commands.

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

## Design
  
### Improving the Prototype 

The second prototype focused on being compact, weather-resistant, and deployable. I repurposed a metal outdoor dual-outlet box and tested for size as shown in figure 2. I desoldered, rewired and resoldered a cleaner and compact circuit and added it to the enclosure. I encountered issues with longterm stability using SPIFFS to write and read from flash and found that the DTH22 sensor would fail over time. I removed the SPIFFS code with an option to use a future SD Card if onboard data storage was necessary and implemented a failsafe to power cycle the DHT22 when data retreival failed. I expanded the system to include an optional water tank with a water level sensor, a soil moisture sensor to regulate the watering cycle and a water pump to deliver water from the tank. I implemented a system block diagram (Diagram 2), a final circuit diagram (Diagram 1), and a PCB design (Fig. 4) which includes connectors to optional component upgrades which are more reliable and can deliver power to higher rated loads.

### Upgrading to Prototype 2:

<li>RTC, light sensor, RGB LED, and buzzer removed</li>
<li>Smaller perfboard cut to fit enclosure</li>
<li>Replaced PSU with HLK-PM01 module</li>
<li>Two relays - one for the fan and one for the heater can handle up to 250V 3A with fuse</li>
<li>Fully enclosed and sealed design with passive air access and drain hole at the bottom</li>


### Parts list for Prototype 2:

<li>HLK_PM01 120VAC 0.2A to 5VDC 0.6A </li>
<li>2 3.5A Slow Blow Fuse</li>
<li>1n4148 Diode</li>
<li>2 2N7000 NPN Transistors</li>
<li>220, 2x10k, 2x1k Ohm 0.25W Resistors</li>
<li>0.22uF X2 Capacitor</li>
<li>22pF Capacitor</li>
<li>100 Ohm 1W Resistor</li>
<li>DHT22</li>
<li>ESP32 Devkit</li>


### For the final design:

<li>Updated psu with HLK-20M12 and AP3211K provide a stable 12vdc to drive relays and 3.3vdc for logic</li>
<li>Added external connectors to relays, sensors, and power supply for customizability</li>
<li>Added uart connector to flash the chip</li>
<li>Can handle up to 250V 20A Power supply to heating and cooling system with HF1115F</li>
<li>Inculdes connector for external antenna for a better long range connection</li></li>
<li>Uses an external and waterproof SHT20/30/40 temperature and humidity sensor </li></li>
<li>Connects to enclosure mounted high current rated relays with heatsinks</li></li>
<li>Uses an optional water level sensor to monitor water level in a tank</li></li>
<li>Uses an optional soil moisture sensor to control the automated watering system</li></li>



><img src="https://github.com/user-attachments/assets/c010b5f1-a3dc-4ce9-8e89-2c99297a522f" width="300" />


Fig 2: Prototype 2 Testing For Fit of Components in Enclosure

>![IMG_1ACED76C-B3CF-402B-A785-0908097FE2AA](https://github.com/user-attachments/assets/e8aa3f8f-5f34-4bb3-8bb0-af1a2303a12f)


Fig 3: Finished Prototype 2

<img width="3507" height="2480" alt="Untitled" src="https://github.com/user-attachments/assets/ba4df52e-f29b-4967-b0c7-a1c2d3db91d1" />


Diagram 1: Final Circuit Diagram

![Untitled Diagram](https://github.com/user-attachments/assets/b207ff3a-e589-4b65-a10c-ca3e8d88826b)


Diagram 2: System Block Diagram

<img width="755" height="512" alt="Capture" src="https://github.com/user-attachments/assets/2cb45744-5f0d-4922-8713-bdc8b902c20e" />


Fig. 4: Final PCB
