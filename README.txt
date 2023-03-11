IOTDimmer v1.0.0

IOTDimmer - Smarthome controller for dimmer lights
========= = ========= ========== === ====== ======

This project includes a simple triac dimmer, together with advanced control
software and IOT functionality for integration with my domotics system 
The code is written on an ESP32 chip with WiFi and MQTT for controlling the light.

Hardware:
---------
- Manual control: 1 push button with LED
- Dimmer control: Triac board generating zeor crossing input and trigger output.
- Microcontroller: In my case an ESP32-S2, but another ESP32 or even ESP8266
                   will also work with some small code modifications.

Functions:
----------
- Settings are stored in EEPROM. Lots of settings can be modified.
- Webinterface for controlling the light, reading values, logging and read/
  modify settings (IOTdimmer.local by default but can be changed).
- Setup as access point if network cannot be found or logged in.
- In access point mode, captive portal (website) is loaded to select network
  from and/ or modify settings.
- MQTT access (on/ off and percentage settings) and sensors readout.
- NTP clock to do time related stuff.
- Low power: Implementation of auto modem sleep and 80 MHz CPU frequency.
             Current is about 30mA in idle. Other sleep modes are not
             possible as the system should listen to commands from WiFi.
             It is not assumed to be a battery powered system.
- Ability to use power or timed mode for dimmer percentage.          
- Use of modes to smoothly switch on/ off lights.
- Use of effects to alter light level and even use it as a ligth organ.

Installation:
-------------
- Clone from github or download zip and unpack it
- Open IOTDimmer in Arduino IDE (I used Arduino IDE 2.0.3)
- Select correct microcontroller (howto's can be found online)
- Build and download code

If you upload the code, you can update to a new version via the web interface
(OTA = Over The Air). Just download the bin file in the bin folder, select it
and press the upload button. No USB connection required.

You can view/ store logging over UDP. Install udplogger to view logging live or
store logging in the background.

That's all for now ...

Please send Comments and Bugreports to hellyrulez@home.nl
