/* 
 * IOTDimmer - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#include "mqtt.h"

portMUX_TYPE cMqtt::mux = portMUX_INITIALIZER_UNLOCKED;
boolean cMqtt::doPub = false;

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
  connected = true;
}

void cMqtt::init() {
  IPAddress ip;  
  if (ip.fromString(settings.getString(settings.brokerAddress))) {
    logger.printf(LOG_MQTT, "mqtt server set from IP");
    client.setServer(ip, settings.getShort(settings.mqttPort));
  } else {
    logger.printf(LOG_MQTT, "mqtt server set from hostname");
    client.setServer(settings.getString(settings.brokerAddress).c_str(), settings.getShort(settings.mqttPort));
  }
  client.setCallback(callback);
  clientId = "IOTDimmer_" + iotWifi.MacPart(6);
  conTimer = xTimerCreateStatic("", pdMS_TO_TICKS(MQTT_RECONNECT_TIME), pdFALSE, (void *)CONNECT_TIMER, timerCallback, &conTimerBuffer);
  connecting = false;
  pubTimer = xTimerCreateStatic("", pdMS_TO_TICKS(MQTT_PUBLISH_TIME), pdTRUE, (void *)PUBLISH_TIMER, timerCallback, &pubTimerBuffer);
  xTimerStart(pubTimer, portMAX_DELAY);
}

void cMqtt::handle() {
  if (!client.connected()) {
    if ((iotWifi.connected) && ((boolean)settings.getByte(settings.UseMqtt))) {
      if (!connecting) {
        xTimerStart(conTimer, portMAX_DELAY);
        connecting = true;
      } else if (xTimerIsTimerActive(conTimer) == pdFALSE) {
        connecting = false;        
        reconnect();
      }
    }
  } else { // Client connected
    if ((!iotWifi.connected) || (!(boolean)settings.getByte(settings.UseMqtt))) {
      client.disconnect();
      connected = false;
    } else {
      client.loop();
      sendStatus();
    }
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

boolean cMqtt::reconnect() {
  boolean connAttempt = false;
  String username = settings.getString(settings.mqttUsername);
  String password = settings.getString(settings.mqttPassword);
  if (username.length() > 0) {
    connAttempt = client.connect(clientId.c_str(), username.c_str(), password.c_str());
  } else {
    connAttempt = client.connect(clientId.c_str());
  }  
  
  if (connAttempt) {
    if (!connected) {
      logger.printf(LOG_MQTT, "MQTT connected");
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
  } else {
    if (connected) {
      logger.printf(LOG_MQTT, "MQTT connection failed, rc=" + String(client.state()) + " try again in 5 seconds");
    }
  }
  connected = client.connected();
  return connected;
}

String cMqtt::getTag(String topic) {
  String tag = "";
  int slashPos = topic.lastIndexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(slashPos+1);
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

void cMqtt::timerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == PUBLISH_TIMER) {
    portENTER_CRITICAL(&mux);
    doPub = true;
    portEXIT_CRITICAL(&mux);
  }
}

cMqtt mqtt;
