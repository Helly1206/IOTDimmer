/* 
 * Clock
 * semi real time clock using NTP time
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 30-5-2021
 * Copyright: Ivo Helwegen
 */

#include "Clock.h"

cClock::cClock() { // constructor
  port = NTP_DEFAULT_LOCAL_PORT;
  timeOffset = 0;
  updateInterval = UPDATE_INTERVAL;
  currentEpoc = 0;
  rebootEpoc = 0;
  poolServerName = "";
  useDST = false;
  udpSetup = false;
  timeSet = false;
  minuteBit = false;
  minuteChanged = false;
}

void cClock::init() {
  if (!udpSetup) {
    ntpUDP = new WiFiUDP();
  } else {
    ntpUDP->stop(); 
  }
  begin();
}

void cClock::handle() {
  update();
  if ((getEpochTime() % 60) < 30) {
    if (!minuteBit) {
      minuteBit = true;
      minuteChanged = true;
    } else {
      minuteChanged = false;
    }
  } else {
    minuteBit = false; 
  }
}

void cClock::updateSettings() {
  signed char zone = 0;
  settings.get(settings.NtpServer, poolServerName);
  zone = (signed char)settings.getByte(settings.NtpZone);
  useDST = (boolean)settings.getByte(settings.UseDST);
  timeOffset = (long)zone * SECONDS_HOUR;
}

boolean cClock::isTimeSet() {
  return ((lastUpdate != 0) && (timeSet)); // returns true if the time has been set, else false
}

unsigned long cClock::getTime() {
  unsigned long dstOffset = 0;
  unsigned long timeNow = getEpochTime();
  if (useDST) {
    dstOffset = getDST(timeNow) ? 3600 : 0;
  }
  return timeNow + dstOffset;
}

String cClock::getFormattedDate() {
  return formatDate(this->getTime());
}

String cClock::getFormattedTime() {
  return formatTime(this->getTime());
}

String cClock::getFormattedBootDate() {
  unsigned long dstOffset = 0;
  unsigned long reboootTime =timeOffset + rebootEpoc;
  if (useDST) {
    dstOffset = getDST(reboootTime) ? 3600 : 0;
  }
  return formatDate(reboootTime + dstOffset);
}

String cClock::getFormattedBootTime() {
  unsigned long dstOffset = 0;
  unsigned long reboootTime =timeOffset + rebootEpoc;
  if (useDST) {
    dstOffset = getDST(reboootTime) ? 3600 : 0;
  }
  return formatTime(reboootTime + dstOffset);
}

boolean cClock::changedMOD() {
  return minuteChanged;
}

///////////// PRIVATES ///////////////////////////

void cClock::begin() {
  lastUpdate = 0;
  updateSettings();
  ntpUDP->begin(port);
  udpSetup = true;
}

void cClock::sendNTPPacket() {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now you can send a packet requesting a timestamp:
  ntpUDP->beginPacket(poolServerName.c_str(), 123);
  ntpUDP->write(packetBuffer, NTP_PACKET_SIZE);
  ntpUDP->endPacket();
}

boolean cClock::update() {
  boolean retval = false;
  // Update after updateInterval or Update if there was no update yet.
  if ((iotWifi.connected) && ((millis() - lastUpdate >= updateInterval) || (lastUpdate == 0))) { 
    if (!udpSetup || port != NTP_DEFAULT_LOCAL_PORT) begin(); // setup the UDP client if needed
    retval = forceUpdate();
    timeSet = (millis() - lastUpdate < UNSET_INTERVAL);
  }
  return retval;   // return false if update does not occur
}

boolean cClock::forceUpdate() {
  logger.printf(LOG_NTP, "Update from NTP Server");

  // flush any existing packets
  while(ntpUDP->parsePacket() != 0) {
    ntpUDP->flush();
  }
  sendNTPPacket();

  // Wait till data is there or timeout...
  byte timeout = 0;
  int cb = 0;
  do {
    delay ( 10 );
    cb = ntpUDP->parsePacket();
    if (timeout > 100) return false; // timeout after 1000 ms
    timeout++;
  } while (cb == 0);

  lastUpdate = millis() - (10 * (timeout + 1)); // Account for delay in reading the time
  ntpUDP->read(packetBuffer, NTP_PACKET_SIZE);
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer, this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  currentEpoc = secsSince1900 - SEVENZYYEARS;
  if (rebootEpoc == 0) {
    rebootEpoc = currentEpoc;
  }
  return true;  // return true after successful update
}

unsigned long cClock::getEpochTime() {
  // User offset + Epoch returned by the NTP server + Time since last update
  return timeOffset + currentEpoc + ((millis() - lastUpdate) / 1000);
}

int cClock::getStartDSTDay(int y) {
  double yd = (double)y;
  int f;
  double y1,y2,y3;
  y1 = floor((5.0*yd) / 4.0);
  y2 = floor(yd / 100.0);
  y3 = floor(yd / 400.0);
  f = (int)(y1 - y2 + y3);
  f %= 7;
  return (31 - ((f+5) % 7));
}

int cClock::getStopDSTDay(int y) {
  double yd = (double)y;
  int f;
  double y1,y2,y3;
  y1 = floor((5.0*yd) / 4.0);
  y2 = floor(yd / 100.0);
  y3 = floor(yd / 400.0);
  f = (int)(y1 - y2 + y3);
  f %= 7;
  return (31 - ((f+2) % 7));
}

boolean cClock::getDST(unsigned long nowTime) {
  int y = year(nowTime);
  int m = month(nowTime);
  int d = day(nowTime);
  int lt = hour(nowTime)*60 + minute(nowTime);
  int g = 0;
  boolean retval = false;
  
  switch (m) {
    case 3: // 0->1
      g = getStartDSTDay(y);
      if ( ((d==g) && (lt>=120)) || (d>g)) {
        retval = true;
      } else {
        retval = false;
      }
      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      retval = true;
      break;
    case 10: // 1<-0
      g = getStopDSTDay(y);
      if ( ((d==g) && (lt>=180)) || (d>g)){
        retval = false;
      } else {
        retval = true;
      }
      break;
    default:
      retval = false;
      break;
  }
  
  return (retval);
}

String cClock::formatDate(unsigned long rawTime) {
  unsigned long y = year(rawTime);
  String yearStr = String(y);

  unsigned long m = month(rawTime);
  String monthStr = m < 10 ? "0" + String(m) : String(m);

  unsigned long d = day(rawTime);
  String dayStr = d < 10 ? "0" + String(d) : String(d);

  return yearStr + "-" + monthStr + "-" + dayStr;
}

String cClock::formatTime(unsigned long rawTime) {
  unsigned long h = hour(rawTime);
  String hoursStr = h < 10 ? "0" + String(h) : String(h);

  unsigned long m = minute(rawTime);
  String minuteStr = m < 10 ? "0" + String(m) : String(m);

  unsigned long s = second(rawTime);
  String secondStr = s < 10 ? "0" + String(s) : String(s);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

cClock Clock;
