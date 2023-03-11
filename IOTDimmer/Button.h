/* 
 * IOTDimmer - Button
 * Button Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 29-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef Button_h
#define Button_h

#ifndef BUTTON_PIN
#define BUTTON_PIN       -1
#endif

#define STEP_SIZE        5

#define DEBOUNCE_TIME       50   /* ms */
#define LEVEL_CHANGE_TIME   1000 /* ms */
#define RESET_TIME          3000 /* ms */
#define LEVEL_CHANGE_COUNTS (byte)(LEVEL_CHANGE_TIME/DEBOUNCE_TIME - 1)
#define RESET_COUNTS        (byte)(RESET_TIME/DEBOUNCE_TIME - 1)

//#define BUTTON_HW_DEBUG

#define BUTTON_TIMER        0

class CButton {
public:
  CButton(); // constructor
  void init();
  void handle();
  boolean initButtonPressed();
  boolean stepping();
private:
  enum buttonstate {idle = 0, debounce, pressed, fired};
  enum buttonaction {none = 0, onoff, step, reset};
  struct isrData {
    buttonstate state;  
    byte count;
  };
  void handleButton();
  buttonaction checkButton();
  void idleButton(buttonaction action);
  byte nextStep();
  boolean ButtonStep;
  boolean StepUp;

  // static
  static void timerCallback(TimerHandle_t xTimer);
  static void IRAM_ATTR isr_button();
  static portMUX_TYPE isrMux;

  volatile static isrData btnData;
  static TimerHandle_t timer;
  static StaticTimer_t timerBuffer;
};

extern CButton button;

#endif
