/* 
 * IOTDimmer - Settings
 * Reads and writes settings from EEPROM
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef IOTSettings_h
#define IOTSettings_h

#include <EEPROM.h>
#include "mbedtls/aes.h"

#define EEPROM_SIZE   512
#define EEPROM_START  0
#define STANDARD_SIZE 32
#define PASSWORD_SIZE 64

#define DT_BYTE   0
#define DT_SHORT  1
#define DT_LONG   2
#define DT_FLOAT  3
#define DT_STRING 4
#define DT_CYPHER 5

#define IV_LEN    16
#define KEY_BITS  256

//#define DO_ENCRYPT // This can mess up settings

class Item {
  public:
    Item(byte _datatype, unsigned short _start); // constructor
    Item(byte _datatype, unsigned short _start, unsigned short _size); // constructor
    byte datatype;
    unsigned short start;
    unsigned short size;
};

class cSettings {
  public:
    cSettings(); // constructor
    ~cSettings(); // destructor
    void init();
    void get(Item *item, byte &b);
    void get(Item *item, unsigned short &s);
    void get(Item *item, unsigned long &l);
    void get(Item *item, float &f);
    void get(Item *item, String &s);
    void get(Item *item, char *s);
    byte getByte(Item *item);
    unsigned short getShort(Item *item);
    unsigned long getLong(Item *item);
    float getFloat(Item *item);
    String getString(Item *item);
    String getDecrypt(Item *item);

    void set(Item *item, byte &b);
    void set(Item *item, unsigned short &s);
    void set(Item *item, unsigned long &l);
    void set(Item *item, float &f);
    void set(Item *item, String &s);
    void set(Item *item, char *s);
    void setEncrypt(Item *item, String &s);

    void update();
    unsigned short getSize(byte datatype);
    byte getDatatype(Item *item);
    
    // Waveform parameters
    Item *WaveMode;            // [byte] [0..3]
    Item *WaveMode100Percent;  // [ms] [0..65535]
    Item *WaveEffect;          // [byte] [0..4]
    Item *WaveEffectMagnitude; // [%] [0..100]
    Item *WaveEffectGain;      // [float]
    Item *WaveEffectTime;      // [ms] [0..65535]

    // Triac parameters
    Item *TriacMode;           // [byte] [0..1]
    Item *LevelOff;            // [%] [0..100]
    Item *LevelOn;             // [%] [0..100]
    Item *LevelLounge;         // [%] [0..100]
  
    // Wifi parameters
    Item *ssid;                // String 32
    Item *password;            // String 64
    Item *hostname;            // String 32
    Item *NtpServer;           // String 32
    Item *NtpZone;             // [byte] [-12..12]
    Item *UseDST;              // [bool]
    Item *UdpPort;             // [0..65535]
    Item *UdpEnabled;          // [bool]
    Item *UpdDebugLevel;       // [0..65535]

    // MQTT parameters
    Item *brokerAddress;       // String 32
    Item *mqttPort;            // [0..65535]
    Item *mqttUsername;        // String 32
    Item *mqttPassword;        // String 64
    Item *mainTopic;           // String 64
    Item *mqttQos;             // [byte] [0..1]
    Item *mqttRetain;          // [bool]
    Item *UseMqtt;             // [bool]
    Item *haDisco;             // [bool]
    Item *haTopic;             // String 32

    unsigned short memsize;
  private:
    void initParameters();
    void resetSettings(unsigned short start, unsigned short size);
    boolean IsEmpty(unsigned short start, unsigned short size);
    void defaultDimmerParameters();
    void defaultWifiParameters();
    void defaultMqttParameters();
    void defaultHaParameters(bool doUpdate);
    void aesDecrypt(char *input, char *output, int dataLength);
    void aesEncrypt(const char *input, char *output, int dataLength);
};

extern cSettings settings;

#endif
