/* 
 * IOTDimmer
 * Dimmer control software with IOT (web and MQTT communication)
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 29-5-2022
 * Copyright: Ivo Helwegen
 */

#define APPVERSION       "v1.1.1"

#define BUTTON_PIN       37 // GPIO37 //interrupt
#define FLASH_PIN         0 // GPIO0
#define SLED_PIN         15 // GPIO15 // system LED
#define ELED_PIN         16 // GPIO16 // external LED
#define SEED_PIN          3 // ADC1_CH2 // keep
#define ZEROCROSS_PIN     5 // GPIO5 //interrupt
#define TRIGGER_PIN      33 // GPIO33

#include "udplogger.h"
#include "IOTWifi.h"
#include "WebServer.h"
#include "Chiller.h"
#include "LED.h"
#include "Button.h"
#include "Settings.h"
#include "Triac.h"
#include "Waveform.h"
#include "Clock.h"
#include "mqtt.h"

void setup() {
  settings.init();
  LED.init();
  button.init();
  triac.init();
  waveform.init();
  iotWifi.init();
  webServer.init();
  Clock.init();
  mqtt.init();
  chiller.init();
}

void loop() {
  LED.handle();
  button.handle();
  triac.handle();
  waveform.handle();
  iotWifi.handle();
  webServer.handle();
  Clock.handle();
  mqtt.handle();
  chiller.handle(); 
}
