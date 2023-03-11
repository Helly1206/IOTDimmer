/*
 * IOTDimmer - Defaults -- To test
 * Webpage content
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 1-5-2022
 * Copyright: Ivo Helwegen
 */

#include "Triac.h"
#include "Waveform.h"

#ifndef DEFAULTS_h
#define DEFAULTS_h

//#define FORCE_DEFAULTS

#define DEF_MODE                (byte)waveform.instant
#define DEF_MODE100             2000 //[ms]
#define DEF_EFFECT              (byte)waveform.enone
#define DEF_EFFECT_MAG          20  //[%]
#define DEF_EFFECT_GAIN         1 //[%/int]
#define DEF_EFFECT_TIME         10000 //[ms]

#define DEF_TRIAC_MODE          (byte)triac.timed
#define DEF_LEVEL_OFF           0
#define DEF_LEVEL_ON            100
#define DEF_LEVEL_LOUNGE        30

#define DEF_SSID                ""
#define DEF_PASSWORD            ""
#define DEF_HOSTNAME            "iotdimmer"
#define DEF_NTPSERVER           "pool.ntp.org"
#define DEF_NTPZONE             1
#define DEF_USEDST              true
#define DEF_LOGPORT             6310
#define DEF_LOGENABLE           true
#define DEF_LOGDEBUG            0

#define DEF_BROKERADDRESS       "mqtt.broker.com"
#define DEF_MQTTPORT            1883
#define DEF_MQTTUSERNAME        ""
#define DEF_MQTTPASSWORD        ""
#define DEF_MAINTOPIC           "myhome/iotdimmer"
#define DEF_MQTTQOS             1
#define DEF_MQTTRETAIN          false
#define DEF_USEMQTT             true

#endif
