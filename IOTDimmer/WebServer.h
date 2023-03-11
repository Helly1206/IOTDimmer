/* 
 * IOTDimmer - WebServer
 * Webserver for accessing IOTDimmer and change settings 
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef IOTWebServer_h
#define IOTWebServer_h

#include <WebServer.h>
#include <Update.h>

#ifndef APPVERSION
#define APPVERSION       "N.A."
#endif

#define MAXSHORT 65535
#define PERCENT  100
#define MILLI    1000
#define HMIN     60

#define CTRLMODE   0
#define CTRLEFFECT 1
#define CTRLINPUT  2

class cWebServer {
  public:
    cWebServer(); // constructor
    void init();
    void handle();
  private:
    static boolean isIp(String str);
    static String toStringIp(IPAddress ip);
    static boolean captivePortal();
    static void handleRoot();
    static void handleWifi();
    static void handleNotFound();
    static void handleDimmer();
    static void handleMqtt();
    static void handleLog();
    static void handleReboot();
    static void sendHeader();
    static void handleMenuLoad();
    static String getMode();
    static String getEffect();
    static String getTimeStatus();
    static void handleHomeUpdate();
    static void handleDimmerCommand();
    static void handleDimmerCtrl();
    static void handleWifiLoad();
    static void handleWifiList();
    static void handleWifiUpdate();
    static void handleWifiSave();
    static void handleWifiMiscSave();
    static void handleWifiUpdateOTA();
    static void handleWifiUpdateOTAResult();
    static void handleWifiLogEnable();
    static void handleWifiLogLevel();
    static void handleWifiLogSave();
    static void handleWifiLogTexts();
    static void handleDimmerLoad();
    static void handleDimmerSave();
    static String getMqttStatus(boolean UseMqtt);
    static void handleMqttLoad();
    static void handleMqttUpdate();
    static void handleMqttSave();
    static void handleLogLoad();
    static void handleLogUpdate();
    static void handleDoReboot();
    static String menuIndex;
};

extern cWebServer webServer;

#endif
