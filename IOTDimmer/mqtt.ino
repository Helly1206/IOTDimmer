/* 
 * IOTDimmer - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#include "mqtt.h"
#include "Json.h"

portMUX_TYPE cMqtt::mux = portMUX_INITIALIZER_UNLOCKED;
boolean cMqtt::doPub = false;
cMqtt::hastatus cMqtt::statusHa = cMqtt::unknown;
boolean cMqtt::discoUpdate = false;

WiFiClient espClient;
PubSubClient client(espClient);

cMqtt::cMqtt() { // constructor
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  publishMem = new valueMem[publishLen];
  for (int i = 0; i < publishLen; i++) {
    publishMem[i].value = "";
    publishMem[i].updateCounter = 0;
  }
  clientId = "";
  connected = false;
}

void cMqtt::init() {
  IPAddress ip;
  client.setBufferSize(512);  
  if (ip.fromString(settings.getString(settings.brokerAddress))) {
    logger.printf(LOG_MQTT, "mqtt server set from IP");
    client.setServer(ip, settings.getShort(settings.mqttPort));
  } else {
    logger.printf(LOG_MQTT, "mqtt server set from hostname");
    client.setServer(settings.getString(settings.brokerAddress).c_str(), settings.getShort(settings.mqttPort));
  }
  client.setCallback(callback);
  clientId = String(dev_mdl) + "_" + iotWifi.MacPart(6);
  conTimer = xTimerCreateStatic("", pdMS_TO_TICKS(MQTT_RECONNECT_TIME), pdFALSE, (void *)CONNECT_TIMER, timerCallback, &conTimerBuffer);
  reconnect_wait = false;
  pubTimer = xTimerCreateStatic("", pdMS_TO_TICKS(MQTT_PUBLISH_TIME), pdTRUE, (void *)PUBLISH_TIMER, timerCallback, &pubTimerBuffer);
  xTimerStart(pubTimer, portMAX_DELAY);
}

void cMqtt::handle() {
  if ((iotWifi.connected) && ((boolean)settings.getByte(settings.UseMqtt))) {
    connected = client.connected();
    if (!connected) {
      if (!reconnect_wait) {
        xTimerStart(conTimer, portMAX_DELAY);
        reconnect_wait = true;
      } else if (xTimerIsTimerActive(conTimer) == pdFALSE) {
        reconnect_wait = false;        
        reconnect();
      }
    } else { // Client connected
      if (discoUpdate) {
        portENTER_CRITICAL(&mux);
        discoUpdate = false;
        portEXIT_CRITICAL(&mux);
        homeAssistantDiscovery();
      }   
      client.loop();
      sendStatus();
    }
  } else {
    connected = false;
  }
}

void cMqtt::update() {
  if ((boolean)settings.getByte(settings.haDisco)) {
    portENTER_CRITICAL(&mux);
    discoUpdate = true;
    portEXIT_CRITICAL(&mux);
  }
}

String cMqtt::fixTopic(String topic) {
  String slash = "/";
  if (topic.endsWith(slash)) {
    topic.remove(topic.length() - 1, 1);
  }
  if (topic.startsWith(slash)) {
    topic.remove(0, 1);
  }
  return topic;
}

String cMqtt::getValue(String tag) {
  String value = "";
  if (tag == dim_status) {
    value = String(waveform.getPower());
  } else if (tag == freq_status) {
    value = String(triac.getFreq());
  } else if (tag == mode_status) {
    value = String(waveform.getMode());
  } else if (tag == effect_status) {
    value = String(waveform.getEffect());
  }
  return value;
}

String cMqtt::buildTopic(String tag) {
  return settings.getString(settings.mainTopic) + "/" + tag;
}

///////////// PRIVATES ///////////////////////////

void cMqtt::callback(char* topic, byte* payload, unsigned int length) {
  String tag = "";
  String payld = "";

  tag = getTag(String(topic));
  payld = bp2string(payload, length);
  logger.printf(LOG_MQTT, "Message received [" +  String(topic) + "] " + String(payld));
  if (tag == dim_offon) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command ON");
      waveform.setPower(settings.getByte(settings.LevelOn));
    } else {
      logger.printf(LOG_MQTTCMD, "Command OFF");
      waveform.setPower(settings.getByte(settings.LevelOff));
    }
    LED.Command();
  } else if (tag == dim_off) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command OFF");
      waveform.setPower(settings.getByte(settings.LevelOff));
    }
    LED.Command();
  } else if (tag == dim_on) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command ON");
      waveform.setPower(settings.getByte(settings.LevelOn));
    }
    LED.Command();
  } else if (tag == dim_lounge) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command LOUNGE");
      waveform.setPower(settings.getByte(settings.LevelLounge));
    }
    LED.Command();
  } else if (tag == dim_dim) {
    logger.printf(LOG_MQTTCMD, "Command POWER");
    waveform.setPower(getPercentage(payld));
    LED.Command();
  } else if (tag == dim_mode) {
    logger.printf(LOG_MQTTCMD, "Command MODE");
    waveform.setMode(getByte(payld));
  } else if (tag == dim_effect) {
    logger.printf(LOG_MQTTCMD, "Command EFFECT");
    waveform.setEffect(getByte(payld));
  } else if (tag == dim_input) {
    logger.printf(LOG_MQTTCMD, "Command INPUT");
    waveform.setInput(getInt(payld));
  } else if ((getMain(topic) == settings.getString(settings.haTopic)) && (tag == ha_status)) { //homeassistant/status
    if (payld == ha_online) {
      logger.printf(LOG_MQTTCMD, "HA online");
      portENTER_CRITICAL(&mux);
      statusHa = online;
      discoUpdate = true;
      portEXIT_CRITICAL(&mux);
    } else if (payld == ha_offline) {
      logger.printf(LOG_MQTTCMD, "HA offline");
      portENTER_CRITICAL(&mux);
      statusHa = offline;
      portEXIT_CRITICAL(&mux);
    }
  } 
}

void cMqtt::sendStatus() { // publish on connected or (every ten minutes or) when value changes (5 seconds for temperature and light, 1 second for pos)
  if ((connected) && (doPub)) {
    portENTER_CRITICAL(&mux);
    doPub = false;
    portEXIT_CRITICAL(&mux);
    int publishLen = (sizeof(PublishTopics) / sizeof(topics));
    for (int i = 0; i < publishLen; i++) {
      String val = getValue(PublishTopics[i].tag);
      if (PublishTopics[i].tag == dim_status) {
        if (val != publishMem[i].value) {
          client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
          publishMem[i].value = val;
          publishMem[i].updateCounter = 0;
          logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
        }
      } else {
        if ((publishMem[i].updateCounter >= 5) && (val != publishMem[i].value)) {
          client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
          publishMem[i].value = val;
          publishMem[i].updateCounter = 0;
          logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
        }
      }
      publishMem[i].updateCounter++;
    }
  }
}

void cMqtt::reconnect() {
  boolean connAttempt = false;
  String username = settings.getString(settings.mqttUsername);
  String password = settings.getString(settings.mqttPassword);
  if (username.length() > 0) {
    connAttempt = client.connect(clientId.c_str(), username.c_str(), password.c_str());
  } else {
    connAttempt = client.connect(clientId.c_str());
  }  
  
  if (connAttempt) {
    logger.printf(LOG_MQTT, "MQTT connected");
    logger.printf(logger.l13, "MQTT connected");
    if ((boolean)settings.getByte(settings.haDisco)) {
      String hatopic = settings.getString(settings.haTopic) + "/" + ha_status;
      client.subscribe(hatopic.c_str(), (int)settings.getByte(settings.mqttQos));
    }
    int publishLen = (sizeof(PublishTopics) / sizeof(topics));
    int subscribeLen = (sizeof(SubscribeTopics) / sizeof(topics));
    for (int i = 0; i < subscribeLen; i++) {
      client.subscribe(buildTopic(SubscribeTopics[i].tag).c_str(), (int)settings.getByte(settings.mqttQos));
    }
    for (int i = 0; i < publishLen; i++) {
      String val = getValue(PublishTopics[i].tag);
      client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
      publishMem[i].value = val;
      publishMem[i].updateCounter = 1;
      logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
    }
    update();
  } else {
    logger.printf(LOG_MQTT, "MQTT connection failed, rc=" + String(client.state()) + " try again in" + String(MQTT_RECONNECT_TIME/1000) + "seconds");
    logger.printf(logger.l13, "MQTT connection failed, rc=" + String(client.state()) + " try again in" + String(MQTT_RECONNECT_TIME/1000) + "seconds");
  }
  return;
}

void cMqtt::homeAssistantDiscovery() {
  logger.printf(LOG_MQTT, "Home Assistant Discovery");
  JSON jString;
  JSON jDeviceString;
  String arraystr[1];
  String devName = getTag(settings.getString(settings.mainTopic));
  String topic;

  arraystr[0] = iotWifi.MacPart(6);
  jDeviceString.AddArray("ids", arraystr, 1);
  jDeviceString.AddItem("name", devName);
  jDeviceString.AddItem("mf", String(dev_mf));
  jDeviceString.AddItem("mdl", String(dev_mdl));
  //logger.printf(jDeviceString.GetJson());

  jString.Clear();

  jString.AddItem("name", ha_light.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/offon");
  jString.AddItem("pl_off", "0");
  jString.AddItem("bri_cmd_t", "~/dim");
  jString.AddItem("bri_stat_t", "~/dim_status"); 
  jString.AddItem("bri_scl", "100"); 
  jString.AddItem("on_cmd_type", "brightness");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_light.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_light.type), devName + us(ha_light.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_off.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/off");
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_off.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_off.type), devName + us(ha_off.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_on.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/on");
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_on.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_on.type), devName + us(ha_on.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_lounge.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/lounge"); 
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_lounge.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_lounge.type), devName + us(ha_lounge.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  /*jString.Clear();

  jString.AddItem("name", String("Input"));
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", String("~/input")); 
  jString.AddItem("uniq_id", arraystr[1] + "input");
  jString.AddItem("dev", jDeviceString);
  logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), "number"), devName + "_input"), "config");
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  //this one doesn't work yet

  jString.AddItem("name", String("Mode"));
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", String("~/mode"));
  jString.AddItem("stat_t", String("~/dim_status"));
  jString.AddItem("cmd_tpl", String("{% set values = { '" + dim_modes[0] + "':0, '" + dim_modes[1] + "':1, '" + dim_modes[2] + "':2, " + dim_modes[3] + "':3} %} {{ values[value] if value in values.keys() else 0 }}"));
  jString.AddItem("stat_tpl", String("{% set values = { 0:'" + dim_modes[0] + "', 1:'" + dim_modes[1] + "', 2:'" + dim_modes[2] + "', 3:" + dim_modes[3] + "'} %} {{ values[value] if value in values.keys() else '" + dim_modes[0] + "' }}"));
  jString.AddArray("options", (String *) dim_modes, sizeof(dim_modes));
  jString.AddItem("uniq_id", arraystr[1] + "mode");
  jString.AddItem("dev", jDeviceString);
  logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), "select"), devName + "_mode"), "config");
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);
  */
  jString.Clear();

  jString.AddItem("name", ha_freq.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("dev_cla", ha_freq.cla);
  jString.AddItem("stat_t", "~/freq_status"); //suggested_display_precision
  jString.AddItem("unit_of_meas", "Hz");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_freq.id));
  jString.AddItem("dev", jDeviceString);
  
  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_freq.type), devName + us(ha_freq.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);
}

String cMqtt::getTag(String topic) {
  String tag = "";
  int slashPos = topic.lastIndexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(slashPos+1);
  }
  return tag;
}

String cMqtt::getMain(String topic) {
  String tag = "";
  int slashPos = topic.indexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(0, slashPos);
  }
  return tag;
}

String cMqtt::bp2string(byte *payload, unsigned int length) {
  String payld = "";
  for (int i = 0; i < length; i++) {
    payld.concat((char)payload[i]);
  }
  return payld;
}

boolean cMqtt::getBoolean(String payload) {
  boolean value = false;
  payload.toLowerCase();
  if ((payload == "on") || (payload == "true") || (payload == "1")) {
    value = true;
  }
  return value;
}

byte cMqtt::getPercentage(String payload) {
  float perc = 0;
  perc = payload.toFloat();
  if (perc < 0) {
    perc = 0;
  }
  if (perc > 100) {
    perc = 100;
  }
  return (byte)round(perc);
}

byte cMqtt::getByte(String payload) {
  float b = 0;
  b = payload.toFloat();
  if (b < 0) {
    b = 0;
  }
  if (b > 255) {
    b = 255;
  }
  return (byte)round(b);
}

int cMqtt::getInt(String payload) {
  float f = 0;
  f = payload.toFloat();
  return (int)round(f);
}

String cMqtt::joinTopic(String Topic, String tag) {
  return Topic + "/" + tag;
}

String cMqtt::us(String tag) {
  return "_" + tag;
}

void cMqtt::timerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == PUBLISH_TIMER) {
    portENTER_CRITICAL(&mux);
    doPub = true;
    portEXIT_CRITICAL(&mux);
  }
}

cMqtt mqtt;
