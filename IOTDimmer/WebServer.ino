/* 
 * IOTDimmer - WebServer
 * Webserver for accessing IOTDimmerl and change settings 
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#include "WebServer.h"
#include "Index.h"
#include "Json.h"

WebServer server(WEB_PORT);
String cWebServer::menuIndex = "0";

cWebServer::cWebServer() { // constructor
}

void cWebServer::init() {
  /* Setup web pages: root, wifi config pages, so captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/dimmer", handleDimmer);
  server.on("/mqtt", handleMqtt);
  server.on("/log", handleLog);
  server.on("/reboot", handleReboot);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/menuload", handleMenuLoad);
  server.on("/homeupdate", handleHomeUpdate);
  server.on("/dimmercommand", handleDimmerCommand);
  server.on("/dimmerctrl", handleDimmerCtrl);
  server.on("/wifiload", handleWifiLoad);
  server.on("/wifilist", handleWifiList);
  server.on("/wifiupdate", handleWifiUpdate);
  server.on("/wifisave", handleWifiSave);
  server.on("/wifimiscsave", handleWifiMiscSave);
  server.on("/wifiupdateota", HTTP_POST, handleWifiUpdateOTAResult, handleWifiUpdateOTA);
  server.on("/logenable", handleWifiLogEnable);
  server.on("/loglevel", handleWifiLogLevel);
  server.on("/logsave", handleWifiLogSave);
  server.on("/logtexts",handleWifiLogTexts);
  server.on("/dimmerload", handleDimmerLoad);
  server.on("/dimmersave", handleDimmerSave);
  server.on("/mqttload", handleMqttLoad);
  server.on("/mqttupdate", handleMqttUpdate);
  server.on("/mqttsave", handleMqttSave);
  server.on("/logload", handleLogLoad);
  server.on("/logupdate", handleLogUpdate);
  server.on("/doreboot", handleDoReboot);
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  logger.printf(LOG_WEBSERVER, "HTTP server started");
}
    
void cWebServer::handle() {
  server.handleClient();
}

///////////// PRIVATES ///////////////////////////

boolean cWebServer::isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String cWebServer::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

 /** Handle root or redirect to captive portal */
void cWebServer::handleRoot() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }
  String Page;
  sendHeader();
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webHome;
  Page += webEnd;
  menuIndex = "1";
  server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean cWebServer::captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(iotWifi.hostname) + ".local")) {
    logger.printf(LOG_WEBSERVER, "Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void cWebServer::handleWifi() {
  String Page;
  sendHeader();
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webWifi;
  Page += webEnd;
  menuIndex = "2";
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleDimmer() {
  sendHeader();
  String Page;
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webDimmer;
  Page += webEnd;
  menuIndex = "3";
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleMqtt() {
  sendHeader();
  String Page;
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webMqtt;
  Page += webEnd;
  menuIndex = "4";
  server.send(200, "text/html", Page); 
  server.client().stop(); // Stop is needed because we sent no content length   
}

void cWebServer::handleLog() {
  sendHeader();
  String Page;
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webLog;
  Page += webEnd;
  menuIndex = "5";
  server.send(200, "text/html", Page);   
  server.client().stop(); // Stop is needed because we sent no content length 
}

void cWebServer::handleReboot() {
  sendHeader();
  String Page;
  Page = webStart; 
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webReboot;
  Page += webEnd;
  menuIndex = "6";
  server.send(200, "text/html", Page);  
  server.client().stop(); // Stop is needed because we sent no content length  
}

void cWebServer::handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(" ") + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  sendHeader();
  server.send(404, "text/plain", message);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::sendHeader() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

void cWebServer::handleMenuLoad() {
  JSON jString;
  jString.AddItem("index", menuIndex);
  jString.AddItem("ap", iotWifi.isAccessPoint());
  server.send(200, "text/plane", jString.GetJson());
}

String cWebServer::getMode() {
  String mode = "Unknown";
  switch (waveform.getModeEnum()) {
    case waveform.instant:
      mode = "Instant";
      break;
    case waveform.linear:
      mode = "Linear";
      break;
    case waveform.sine:
      mode = "Sine";
      break;
    case waveform.qsine:
      mode = "Qsine";
      break;
  }
  return mode;
}

String cWebServer::getEffect() {
  String effect = "Unknown";
  switch (waveform.getEffectEnum()) {
    case waveform.enone:
      effect = "None";
      break;
    case waveform.eramp:
      effect = "Ramp";
      break;
    case waveform.esine:
      effect = "Sine";
      break;
    case waveform.erandom:
      effect = "Random";
      break;
    case waveform.einput:
      effect = "Input";
      break;
  }
  return effect;
}

String cWebServer::getTimeStatus() {
  String status = "Unknown";
  if (Clock.isTimeSet()) {
    status = "";
  } else {
    status = "NTP Time not set";
  }
  return status;
}

void cWebServer::handleHomeUpdate() {
  JSON jString;
  jString.AddItem("time", Clock.getFormattedDate() + " " + Clock.getFormattedTime());
  jString.AddItem("timestatus", getTimeStatus());  
  jString.AddItem("mqttstatus", getMqttStatus((boolean)settings.getByte(settings.UseMqtt)));
  jString.AddItem("mainsfreq", String(triac.getFreq()));
  jString.AddItem("level", waveform.getPower());
  jString.AddItem("waveformmode", waveform.getMode());
  jString.AddItem("effect", waveform.getEffect());
  jString.AddItem("effectinput", waveform.getInput());
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleDimmerCommand() {
  short Cmd = (short)server.arg("cmd").toInt();
  if (Cmd == -1) { // Off command
    logger.printf(LOG_WEBSERVER, "Dimmer Command: OFF");
    waveform.setPower(settings.getByte(settings.LevelOff));
  } else if (Cmd == 101) { // On command
    logger.printf(LOG_WEBSERVER, "Dimmer Command: ON");
    waveform.setPower(settings.getByte(settings.LevelOn));
  } else if (Cmd == 110) { // Lounge command
    logger.printf(LOG_WEBSERVER, "Dimmer Command: LOUNGE");
    waveform.setPower(settings.getByte(settings.LevelLounge));
  } else { // directly set power
    logger.printf(LOG_WEBSERVER, "Dimmer Command: " + String(Cmd));
    waveform.setPower((byte)Cmd);
  }
  LED.Command();
  server.send(200, "text/plane", "Ok");
}

void cWebServer::handleDimmerCtrl() {
  byte Type = (byte)server.arg("type").toInt();
  int Ctrl = (int)server.arg("ctrl").toInt();
  if (Type == CTRLMODE) {
    waveform.setMode((byte)Ctrl);
  } else if (Type == CTRLEFFECT) {
    waveform.setEffect((byte)Ctrl);
  } else if (Type == CTRLINPUT) {
    waveform.setInput(Ctrl);
  } 
  server.send(200, "text/plane", "Ok");
}

void cWebServer::handleWifiLoad() {
  JSON jString;
  if (server.client().localIP() == *(iotWifi.apIP)) {
    jString.AddItem("network", "Soft Access Point");
    jString.AddItem("ssid", iotWifi.APssid);
    jString.AddItem("hostname", "N/A");
    jString.AddItem("ip", toStringIp(WiFi.softAPIP()));
  } else {
    jString.AddItem("network", "Wifi network");
    jString.AddItem("ssid", WiFi.SSID());
    jString.AddItem("whostname", String(iotWifi.hostname) + ".local");
    jString.AddItem("ip", toStringIp(WiFi.localIP()));
  }
  jString.AddItem("mac", WiFi.macAddress());
  jString.AddItem("apssid", iotWifi.APssid);
  jString.AddItem("wlanssid", iotWifi.ssid);
  jString.AddItem("wlanrssi", (int)WiFi.RSSI());
  jString.AddItem("hostname", settings.getString(settings.hostname));
  jString.AddItem("ntpserver", settings.getString(settings.NtpServer));
  jString.AddItem("timezone", (signed char)settings.getByte(settings.NtpZone));
  jString.AddItem("usedst", (boolean)settings.getByte(settings.UseDST));
  jString.AddItem("appversion", String(APPVERSION));
  jString.AddItem("reboottime", Clock.getFormattedBootDate() + " " + Clock.getFormattedBootTime());
  jString.AddItem("rebootreason0", chiller.getResetReason(0));
  jString.AddItem("rebootreason1", chiller.getResetReason(1));
  jString.AddItem("heapmem", chiller.getHeapMem());
  jString.AddItem("progmem", chiller.getProgramMem());
  jString.AddItem("sdkversion", chiller.getVersion());
  jString.AddItem("cpufreq", chiller.getCPUFreq());
  jString.AddItem("udpport", settings.getShort(settings.UdpPort));
  jString.AddItem("udpenable", logger.isEnabled());
  jString.AddItem("udpdebug", logger.getDebug());
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleWifiList() {
  JSON jString;
  int n = WiFi.scanNetworks();
  if (n > 0) {
    String WLANlist[n];
    for (int i = 0; i < n; i++) {
      JSON jItem;
      jItem.AddItem("ssid", WiFi.SSID(i));
      jItem.AddItem("content", WiFi.SSID(i) + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*") + " (" + WiFi.RSSI(i) + " dBm)");
      jItem.AddItem("select", WiFi.SSID(i) == iotWifi.ssid);
      WLANlist[i] = jItem.GetJson();
    }
    jString.AddArray("", WLANlist, n);
  }
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleWifiUpdate() {
  JSON jString;
  jString.AddItem("wlanrssi", (int)WiFi.RSSI());
  server.send(200, "text/plane", jString.GetJson());
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void cWebServer::handleWifiSave() {
  iotWifi.ssid = server.arg("inetwork");
  String password = server.arg("ipassword");
  settings.set(settings.ssid, iotWifi.ssid);
  settings.set(settings.password, password);
  settings.update();
  server.sendHeader("Location", "wifi", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  iotWifi.connect();
}

void cWebServer::handleWifiMiscSave() {
  byte bval;
  String sval;
  sval = server.arg("hostname");
  settings.set(settings.hostname, sval);
  sval = server.arg("ntpserver");
  settings.set(settings.NtpServer, sval);
  bval = (byte)server.arg("timezone").toInt();
  settings.set(settings.NtpZone, bval);
  bval = (byte)(server.arg("usedst")=="on");
  settings.set(settings.UseDST, bval);
  settings.update();
  Clock.updateSettings();
  server.sendHeader("Location", "wifi", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleWifiUpdateOTA() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    logger.printf(LOG_WEBSERVER, "Update: " + String(upload.filename));
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
      logger.printf("Update error: update cannot start");
      //Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      logger.printf("Update error: sizes differ");
      //Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      logger.printf(LOG_WEBSERVER, "Update Success: " + String(upload.totalSize));
      logger.printf(LOG_WEBSERVER, "Let's reboot");
    } else {
      logger.printf("Update error: update failed");
      //Update.printError(Serial);
    }
  }
}

void cWebServer::handleWifiUpdateOTAResult() {
  if (Update.hasError()) {
    server.send(400, "text/plain", Update.errorString());
    server.client().stop();
  } else {
    handleDoReboot();
  }
}

void cWebServer::handleWifiLogEnable() {
  bool ena = (bool)server.arg("ena").toInt();
  logger.enable(ena);
  server.send(200, "text/plane", "Ok");
}

void cWebServer::handleWifiLogLevel() {
  short lvl = (short)server.arg("lvl").toInt();
  logger.setDebug(lvl);
  server.send(200, "text/plane", "Ok");
}

void cWebServer::handleWifiLogSave() {
  byte bval;
  unsigned short val;
  val = (short)server.arg("udpport").toInt();
  settings.set(settings.UdpPort, val);
  bval = logger.isEnabled();
  settings.set(settings.UdpEnabled, bval);
  val = logger.getDebug();
  settings.set(settings.UpdDebugLevel, val);
  settings.update();
  server.sendHeader("Location", "wifi", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleWifiLogTexts() {
  JSON jString;
  int arrayLen = (sizeof(levelTexts) / sizeof(String));
  jString.AddArray("", (String*)levelTexts, arrayLen);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleDimmerLoad() {
  JSON jString;

  jString.AddItem("waveformmode", settings.getByte(settings.WaveMode));
  jString.AddItem("mode100", settings.getShort(settings.WaveMode100Percent));
  jString.AddItem("effect", settings.getByte(settings.WaveEffect));
  jString.AddItem("effectmagnitude", settings.getByte(settings.WaveEffectMagnitude));
  jString.AddItem("effectgain", settings.getFloat(settings.WaveEffectGain));
  jString.AddItem("effecttime", settings.getShort(settings.WaveEffectTime));
  jString.AddItem("triacmode", settings.getByte(settings.TriacMode));
  jString.AddItem("leveloff", settings.getByte(settings.LevelOff));
  jString.AddItem("levelon", settings.getByte(settings.LevelOn));
  jString.AddItem("levellounge", settings.getByte(settings.LevelLounge));

  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleDimmerSave() {
  unsigned short val;
  float fval;
  byte bval;

  bval = (byte)server.arg("waveformmode").toInt();
  settings.set(settings.WaveMode, bval);
  val = (unsigned short)server.arg("mode100").toInt();
  settings.set(settings.WaveMode100Percent, val);
  bval = (byte)server.arg("effect").toInt();
  settings.set(settings.WaveEffect, bval);
  bval = (byte)server.arg("effectmagnitude").toInt();
  settings.set(settings.WaveEffectMagnitude, bval);
  fval = server.arg("effectgain").toFloat();
  settings.set(settings.WaveEffectGain, fval);
  val = (unsigned short)server.arg("effecttime").toInt();
  settings.set(settings.WaveEffectTime, val);
  bval = (byte)server.arg("triacmode").toInt();
  settings.set(settings.TriacMode, bval);
  bval = (byte)server.arg("leveloff").toInt();
  settings.set(settings.LevelOff, bval);
  bval = (byte)server.arg("levelon").toInt();
  settings.set(settings.LevelOn, bval);
  bval = (byte)server.arg("levellounge").toInt();
  settings.set(settings.LevelLounge, bval);
  settings.update();
  server.sendHeader("Location", "dimmer", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

String cWebServer::getMqttStatus(boolean UseMqtt) {
  String status = "Disabled";
  if (UseMqtt) {
    if (mqtt.connected) {
      status = "Connected";
    } else {
      status = "Disconnected";
    }
  }
  return status;
}

void cWebServer::handleMqttLoad() {
  JSON jString;
  boolean UseMqtt = (boolean)settings.getByte(settings.UseMqtt);
  jString.AddItem("clientid", mqtt.clientId);
  jString.AddItem("status", getMqttStatus(UseMqtt));
  jString.AddItem("brokeraddress", settings.getString(settings.brokerAddress));
  jString.AddItem("mqttport", settings.getShort(settings.mqttPort));
  jString.AddItem("mqttusername", settings.getString(settings.mqttUsername));
  jString.AddItem("mqttpassword", settings.getString(settings.mqttPassword));
  jString.AddItem("maintopic", settings.getString(settings.mainTopic));
  jString.AddItem("mqttqos", settings.getByte(settings.mqttQos));
  jString.AddItem("mqttretain", (boolean)settings.getByte(settings.mqttRetain));
  jString.AddItem("usemqtt", UseMqtt);
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  int subscribeLen = (sizeof(SubscribeTopics) / sizeof(topics));
  String arraystr[publishLen + subscribeLen];
  for (int i = 0; i < publishLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", PublishTopics[i].tag);
    jArrItem.AddItem("topic", mqtt.buildTopic(PublishTopics[i].tag));
    jArrItem.AddItem("value", mqtt.getValue(PublishTopics[i].tag));
    jArrItem.AddItem("description", PublishTopics[i].description);
    arraystr[i] = jArrItem.GetJson();
  }
  for (int i = 0; i < subscribeLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", SubscribeTopics[i].tag);
    jArrItem.AddItem("topic", mqtt.buildTopic(SubscribeTopics[i].tag));
    jArrItem.AddItem("value", String(""));
    jArrItem.AddItem("description", SubscribeTopics[i].description);
    arraystr[publishLen + i] = jArrItem.GetJson();
  }
  jString.AddArray("topics", arraystr, publishLen + subscribeLen);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleMqttUpdate() {
  JSON jString;
  boolean UseMqtt = (boolean)settings.getByte(settings.UseMqtt);
  jString.AddItem("clientid", mqtt.clientId);
  jString.AddItem("status", getMqttStatus(UseMqtt));
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  String arraystr[publishLen];
  for (int i = 0; i < publishLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", PublishTopics[i].tag);
    jArrItem.AddItem("value", mqtt.getValue(PublishTopics[i].tag));
    arraystr[i] = jArrItem.GetJson();
  }
  jString.AddArray("topics", arraystr, publishLen);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleMqttSave() {
  String sval;
  String sval2;
  byte bval;
  unsigned short val;
  sval = server.arg("brokeraddress");
  settings.set(settings.brokerAddress, sval);
  val = (unsigned short)server.arg("mqttport").toInt();
  settings.set(settings.mqttPort, val);
  sval = server.arg("mqttusername");
  settings.set(settings.mqttUsername, sval);
  sval = server.arg("mqttpassword");
  settings.set(settings.mqttPassword, sval);
  sval = server.arg("maintopic");
  sval2 = mqtt.fixTopic(sval);
  settings.set(settings.mainTopic, sval2);
  bval = (byte)server.arg("mqttqos").toInt();
  settings.set(settings.mqttQos, bval);
  bval = (byte)(server.arg("mqttretain")=="on");
  settings.set(settings.mqttRetain, bval);
  bval = (byte)(server.arg("usemqtt")=="on");
  settings.set(settings.UseMqtt, bval);
  settings.update();
  server.sendHeader("Location", "mqtt", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleLogLoad() {
  JSON jString;
  String datastr[2];

  datastr[0] = "Logging started @ " + Clock.getFormattedDate() + " " + Clock.getFormattedTime();
  datastr[1] = "Time, Power (Power Setpoint), Input, Mode, Effect";
  jString.AddArray("", datastr, 2);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleLogUpdate() {
  JSON jString;
  String datastr[1];
  datastr[0] = Clock.getFormattedTime() + ", " + String(triac.getPower()) + " (" + String(waveform.getPower()) + "), " + String(waveform.getInput()) + ", " + getMode() + ", " + getEffect();
  jString.AddArray("", datastr, 1);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleDoReboot() {
  logger.printf("Rebooting ...");
  sendHeader();
  String Page;
  Page = String(webRebooting);
  server.send(200, "text/html", Page);    
  server.client().stop(); // Stop is needed because we sent no content length
  ESP.restart();
}

cWebServer webServer;
