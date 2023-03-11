/* 
 * IOTDimmer - LED
 * Switching the LED
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 29-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef IOTLEDs_h
#define IOTLEDs_h

#ifndef SLED_PIN
#define SLED_PIN    -1
#endif
#ifndef ELED_PIN
#define ELED_PIN  -1
#endif

//#define USE_SYSTEM_LED

#define T_TIME1     (900)
#define T_TIME2     (100)
#define T_TIME3     (500)

#define HIGH_IMPEDANCE_OUTPUTS

class Cled {
public:
  Cled();
  void init();
  void handle(void);
  void On();
  void Step();
  void Off();
  void WifiApC();
  void WifiNC();
  void WifiC();
  void Command();
private:
  enum ledmode {none   = 0, 
                on     = 1, 
                flsh   = 4,   // 0100
                shrt   = 5,   // 0101
                lng    = 6,   // 0110
                stop   = 7,   // 0111
                flsh1  = 8,   // 1000
                shrt1  = 9,   // 1001
                lng1   = 10,  // 1010
                flsh3  = 12,  // 1100
                shrt3  = 13,  // 1101
                lng3   = 14}; // 1110
  struct ledmodes {
    ledmode c;
    ledmode q;
    ledmode d;   
  };
  struct ledflash {
    byte repeatCount;
    unsigned long t1;
    unsigned long t2;
    boolean isOn;
  };
  void setMode(ledmode cmd);
  static void initLED(uint8_t pin);
  static void setLED(uint8_t pin, uint8_t value);
  static void timerCallback(TimerHandle_t xTimer);
  static void flashOn(TimerHandle_t xTimer);
  static void flashOff(TimerHandle_t xTimer);
  TimerHandle_t timer;
  StaticTimer_t timerBuffer;
  static portMUX_TYPE mux;
  static ledmodes modes;
  static bool timerModeChange;
  static ledflash flash;
};

extern Cled LED;

#endif
