/* 
 * IOTDimmer - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef MQTT_h
#define MQTT_h

#include <PubSubClient.h>

#define MQTT_SERVER "mqtt.broker.com"
#define MQTT_PORT 1883

#define MQTT_PUBLISH_TIME   1000 /* ms */
#define MQTT_RECONNECT_TIME 5000 /* ms */

#define CONNECT_TIMER       0
#define PUBLISH_TIMER       1

typedef struct { 
  String tag;
  String description;
} topics;

typedef struct { 
  String value;
  unsigned long updateCounter;
} valueMem;

const char dim_status[] = "dim_status";
const char freq_status[] = "freq_status";
const char mode_status[] = "mode_status";
const char effect_status[] = "effect_status";
const char dim_status_cmt[] = "publish: current power of the dimmer [%]";
const char freq_status_cmt[] = "publish: current line frequency [Hz]";
const char mode_status_cmt[] = "publish: current mode status [0=inst, 1=lin, 2=sin, 3=qsin]";
const char effect_status_cmt[] = "publish: current effect status [0=none, 1=ramp, 2=sin, 3=rnd, 4=inp]";

const topics PublishTopics[] {
  {dim_status, dim_status_cmt},
  {freq_status, freq_status_cmt},
  {mode_status, mode_status_cmt},
  {effect_status, effect_status_cmt}
};

const char dim_offon[] = "offon";
const char dim_off[] = "off";
const char dim_on[] = "on";
const char dim_lounge[] = "lounge";
const char dim_dim[] = "dim";
const char dim_mode[] = "mode";
const char dim_effect[] = "effect";
const char dim_input[] = "input";
const char dim_offon_cmt[] = "subscribe: switch dimmer off (0) or on (1) [off/ on, false/ true, 0/ 1]";
const char dim_off_cmt[] = "subscribe: switch dimmer off [off/ on, false/ true, 0/ 1]";
const char dim_on_cmt[] = "subscribe: switch dimmer on [off/ on, false/ true, 0/ 1]";
const char dim_lounge_cmt[] = "subscribe: switch dimmer to lounge level [off/ on, false/ true, 0/ 1]";
const char dim_dim_cmt[] = "subscribe: set dimmer power [0-100 %]";
const char dim_mode_cmt[] = "subscribe: set dimmer mode [0..3]";
const char dim_effect_cmt[] = "subscribe: set dimmer effect [0..4]";
const char dim_input_cmt[] = "subscribe: set dimmer input effect signal [integer]";

const topics SubscribeTopics[] {
  {dim_offon, dim_offon_cmt},
  {dim_off, dim_off_cmt},
  {dim_on, dim_on_cmt},
  {dim_lounge, dim_lounge_cmt},
  {dim_dim, dim_dim_cmt},
  {dim_mode, dim_mode_cmt},
  {dim_effect, dim_effect_cmt},
  {dim_input, dim_input_cmt}
};

class cMqtt {
  public:
    cMqtt(); // constructor
    void init();
    void handle();
    String fixTopic(String topic);
    String getValue(String tag);
    String buildTopic(String tag);
    String clientId;
    boolean connected;
  private:
    static void callback(char* topic, byte* payload, unsigned int length);
    void sendStatus();
    boolean reconnect();
    static String getTag(String topic);
    static String bp2string(byte *payload, unsigned int length);
    static boolean getBoolean(String payload);
    static byte getPercentage(String payload);
    static byte getByte(String payload);
    static int getInt(String payload);
    valueMem *publishMem;
    boolean connecting;
    static void timerCallback(TimerHandle_t xTimer);
    TimerHandle_t conTimer;
    StaticTimer_t conTimerBuffer;
    static portMUX_TYPE mux;
    TimerHandle_t pubTimer;
    StaticTimer_t pubTimerBuffer;
    static boolean doPub;
};

extern cMqtt mqtt;

#endif
