/* 
 * IOTDimmer - Triac
 * Dimmer triac Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 15-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef Triac_h
#define Triac_h

#define MOVAV_WIDTH    16     // moving average of 16 samples
#define ZERO_MIN       5000   // minimum 5000 us -> 100 Hz
#define ZERO_MAX       20000  // maximum 20000 us -> 25 Hz
#define IGNITION_MAX   100000 // maximum ignition time if no zero crossing
#define SAFETY_TIME_US 100
#define STABILIZER_NR  6      // about 1 second (= 6.25 @ 50Hz)

#define PWR_ON         100
#define PWR_OFF        0

#if defined(ARDUINO_ARCH_AVR)
#define ZEROCROSS_PIN  2
#define TRIGGER_PIN    3
typedef int portMUX_TYPE; // get ESP mutex and IRAM work on arduino
#define portENTER_CRITICAL_ISR(mux)
#define portEXIT_CRITICAL_ISR(mux)
#define portENTER_CRITICAL(mux)
#define portEXIT_CRITICAL(mux)
#define portMUX_INITIALIZER_UNLOCKED 0
#define IRAM_ATTR
#else
#ifndef ZEROCROSS_PIN
#define ZEROCROSS_PIN  -1
#endif
#ifndef TRIGGER_PIN
#define TRIGGER_PIN    -1
#endif
#endif

typedef void (*lowpower_cb)(bool);

class CTriac {
public:
  enum triacmode {timed = 0, power = 1};
  CTriac(); // constructor
  void init(void);
  void handle(void);
  void reset(void);
  float getFreq();
  void setPower(byte power);
  byte getPower();
  byte getMode();
  void setMode(byte mode);
  void setCallback(void *cb);
private:
  enum triacstate {idle = 0, zero = 1, pulse = 2, off = 3, on = 4, zerouncalibrated = 5, zerocalibrating = 6};
  struct triacdata {
    triacstate state;
    unsigned long igniteTime;
    unsigned long pulseWidth;
    unsigned long zeroStamp;
    unsigned long movAvMemory[MOVAV_WIDTH];
    byte movAvCounter;
    byte stabilizerCounter;
    bool movAvValid;
  };
  void setZero(byte power);
  void zeroOn();
  void zeroOff();
  void calibrateZero();
  void testZeroCalibrated();
  unsigned long calcPowerMode(byte power);
  unsigned long calcTimedMode(byte power);
  byte calcFromPowerMode(unsigned long ignTime);
  byte calcFromTimedMode(unsigned long ignTime);
  unsigned long getZeroTime();
  void setIgniteTime(unsigned long ignTime, byte &power);
  unsigned long getIgniteTime();
  void setState(byte power);
  void ClearMovAvFilter();
  triacmode dimMode;
  bool zeroState;
  lowpower_cb lpCallback;

  // static
  static portMUX_TYPE movAvMux;
  static portMUX_TYPE stateMux;
  volatile static triacdata triacData;
  volatile static unsigned long zeroSample;
  static void IRAM_ATTR isr_ext();
  static void IRAM_ATTR isr_timer();
};

extern CTriac triac;

#endif
