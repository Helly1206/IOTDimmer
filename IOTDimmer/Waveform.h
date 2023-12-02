/* 
 * IOTDimmer - Waveform
 * Dimmer waveform generation
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 27-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef Waveform_h
#define Waveform_h

#ifndef SEED_PIN
#define SEED_PIN            -1
#endif

#define MODE_TIMER          0
#define EFFECT_TIMER        1

class CWaveform {
public:
  enum waveformmode {instant = 0, linear = 1, sine = 2, qsine = 3};
  enum waveformeffect {enone = 0, eramp = 1, esine = 2, erandom = 3, einput = 4};
  CWaveform(); // constructor
  void init(void);
  void handle(void);
  void setPower(byte ipower);
  byte getPower();
  void setInput(int iinput);
  int getInput();
  void setMode(byte imode);
  byte getMode();
  boolean getStatus();
  waveformmode getModeEnum();
  
  void setEffect(byte ieffect);
  byte getEffect();
  waveformeffect getEffectEnum();
private:
  void updateMode(byte ipower);
  byte calcMode();
  unsigned long getElapsedPercent();
  byte calcModeLinear();
  byte calcModeSine();
  byte calcModeQsine();
  void calcEffPower();
  unsigned long getEffectElapsedPercent();
  byte calcEffRamp();
  byte calcEffSine();
  byte calcEffRandom();
  byte calcEffInput();
  bool effDo(short &pwr);
  void effRange(short &pwr);
  byte power;
  byte effPower;
  byte prevPower;
  byte startPower;
  bool modeConvDone;
  int effectInput;
  waveformmode mode;
  waveformeffect effect;
  byte triacMode;
  static void timerCallback(TimerHandle_t xTimer);
  TimerHandle_t modeTimer;
  StaticTimer_t modeTimerBuffer;
  static portMUX_TYPE mux;
  TimerHandle_t effTimer;
  StaticTimer_t effTimerBuffer;
  static boolean doEffect;
};

extern CWaveform waveform;

#endif
