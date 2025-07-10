# Greenhouse-Monitor
Designed an embedded  system that monitors a greenhouse and control a fan and heater to maintain an ideal temperature range. The device easily connects over WiFi to display a record of the data, provide manual controls, and show localized weather information including frost and heat wave warnings.

## Background
  
  Purpose - The project started as a way for me to improve my skills in both coding and embedded systems and involved both relearning many old concepts as well as new ones. I wanted a system that would be able to monitor a small greenhouse and provide a user with real time temperature data wirelessly. The system evolved into testing many different sensors I had availible and using a microcontroller to implement a wireless dashboard to read the data. I wanted a more streamlined and practical system so I redesigned the prototype to be minimalist and still include some extra sensors for field testing. I later implemented a webserver and database which would track data overtime and allow the user to observe and control thier greenhouse system from anywhere as well as warn them of upcoming frost and heatwave advisories.
  
  Implementation - I wanted a small system that would use an esp32 MCU to send wireless sensor updates to an online webpage. To do this I needed to develop a prototype, I assembled a proof of concept, Fig. 1, which included all the relevant sensors I had access to. I later developed a local dashboard, uploaded 15 second interval data reports to thingspeak, and finally implemented an online webserver and sql database.
  
  Parts List - The following parts for the second prototype not including the optional soil moisture and water level sensors are listed below.
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

  Coding the MCU - The MCU i chose is an ESP32 Devkit since it has built in WiFi and Bluetooth capabilities. The MCU is updatable via the webserver using <ArduinoOTA.h>, while <WiFiManager.h>, <WebServer.h>, <WiFi.h>, and <HTTPClient.h> libraries handle the WiFi connection and data transfer. The code needed to be able to periodically log the data, send the data to an online server database, determine if the system was in the correct range, and implement a correction (Fan or Heater) to bring the system back to ideal range. The program also was able to accept manual fan and heater ovverides to turn them on or off by default (but not both). The program was also able to accept user constraints in the form of maximum and minimum temperature values so that the user could define thier ideal greenhouse ecosystem. The program used SPIFFS to save a rotating backup 24 hours of logs with one log every 15 seconds on flash memory. A simple local dashboard was uploaded to the flash memory using SPIFFS for testing purposes.
  
  Designing the Prototype - The inital prototype included a sensor to monitor water level in a single plant pot, a sesnor to monitor water moisture in a single plant pot, a sensor to monitor average light levels, an indicator RGB LED, a warning buzzer, a real time clock with shift logic register and a dht11 to monitor temperature and humidity levels. The DHT11 failed and was replaced with a DHT22 which provides a better data resolution and range. The prototype was initially going to be battery powered until I decided to allow the system to control either a fan or a heater to maintain ideal temperature levels. I decided to connect the device directly to an AC mains power outlet using an extention cord. I had never worked with AC power so close to embedded systems before so it was a learning experience.

  ![IMG_4A27B2FE-A575-4C25-8DCB-BFCA18A9B55C](https://github.com/user-attachments/assets/a51813a0-697a-4338-999f-32065ff3e681)
Fig 1: Breadboard prototype including all modules and features as a proof of concept
  
![IMG_0581](https://github.com/user-attachments/assets/27479760-3850-4679-929b-592f24989540)
Fig 2: Completed Prototype 1

![IMG](https://github.com/joshuaglenen/Greenhouse-Monitor/blob/main/prototype_1/Prototype_1_Circuit_Diagram.png)
Diagram 1: Prototype 1 Circuit Diagram
  
  
  Improving the Prototype - The first prototype was sucessful however it was messy, bulky, and included unnecessary parts. I found an outdoor outlet with some extra space that could just fit the microcontroller, two relays, and a PSU. I removed the old components, cut down a new protoboard to size, drew a new circuit diagram, and soldered each component back onto the protoboard with a space concious apporach. The original PSU which was borrowed from a keyboard charger failed and I had to replace it with a New HLK_PM01 module which has been reliable so far. The second prototype was aiming for a small, waterproof, and practical approach where it could be tested in a real environment.

![IMG_0595](https://github.com/user-attachments/assets/c010b5f1-a3dc-4ce9-8e89-2c99297a522f)
Fig 3: Prototype 2 Testing For Fit of Components in Enclosure

![IMG_1ACED76C-B3CF-402B-A785-0908097FE2AA](https://github.com/user-attachments/assets/e8aa3f8f-5f34-4bb3-8bb0-af1a2303a12f)
Fig 4: Finished Prototype 2

![IMG](https://github.com/joshuaglenen/Greenhouse-Monitor/blob/main/prototype_2/Prototype_2_Circuit_Diagram.png)
Diagram 2: Prototype 2 Circuit Diagram


  Coding the Webserver - The first step was moving from the local web dashboard to an online webapp. This coincided with implementing a database to log past data reports using first thingspeak and later a custom SQL server. TODO

## Results

  Testing - TODO
  
  Conclusion - TODO
