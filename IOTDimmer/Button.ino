/* 
 * IOTDimmer - Button
 * Button Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 29-5-2022
 * Copyright: Ivo Helwegen
 */

/* 
 * Original: Up/ Down button Control
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 */ 
 
#include "Button.h"

StaticTimer_t CButton::timerBuffer;
TimerHandle_t CButton::timer; // = NULL;
volatile CButton::isrData CButton::btnData = {CButton::idle, 0};
portMUX_TYPE CButton::isrMux = portMUX_INITIALIZER_UNLOCKED;

CButton::CButton() { // constructor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  ButtonStep = false;  
  StepUp = true;
}

void CButton::init() {
  timer = xTimerCreateStatic("button", pdMS_TO_TICKS(DEBOUNCE_TIME), pdFALSE, (void *)BUTTON_TIMER, timerCallback, &timerBuffer);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr_button, FALLING);
}

void CButton::handle() {
  if (!iotWifi.wakingUp()) {
    handleButton();
  }
}

boolean CButton::initButtonPressed() {
  return checkButton() == onoff;
}

boolean CButton::stepping() {
  return ButtonStep;
}

// Privates ........

void CButton::handleButton() {
  switch (checkButton()) {
    case onoff:
      if (ButtonStep) { // handle step mode
        logger.printf(LOG_BUTTON, "Button mode step");
        waveform.setPower(nextStep());
      } else { // handle on/off mode
        if (waveform.getStatus()) { // = on, switch off
          logger.printf(LOG_BUTTON, "Button off");
          waveform.setPower(settings.getByte(settings.LevelOff));
        } else { //= off, switch on
          logger.printf(LOG_BUTTON, "Button on");
          waveform.setPower(settings.getByte(settings.LevelOn));
        }
        LED.Command();
      }
      break;
    case step:
      ButtonStep = !ButtonStep;
      if (ButtonStep) {
        logger.printf(LOG_BUTTON, "Button step");
        LED.Step();
      } else {
        logger.printf(LOG_BUTTON, "Button mode onoff");
        LED.Command();
      }
      break;
    case reset:
      logger.printf(LOG_BUTTON, "System reset");
      ESP.restart();
      break;
  }  
}

CButton::buttonaction CButton::checkButton() {
  buttonaction action = none;
  isrData btnCopy = {idle, 0};

#ifdef BUTTON_HW_DEBUG
  String logString = "";
#endif
  portENTER_CRITICAL(&isrMux);
  if (btnData.state != idle) {
#ifdef BUTTON_HW_DEBUG
    logString = "Btn, state: " + String(btnData.state) + ", count: " + String(btnData.count);
#endif
    memcpy((void*)&btnCopy, (void*)&btnData, sizeof(isrData));
  }
  portEXIT_CRITICAL(&isrMux);
#ifdef BUTTON_HW_DEBUG
  if (!logString.isEmpty()) {
    logger.printf(LOG_BUTTON, logString);  
  }
#endif
  if (btnCopy.state == fired) {
    if (btnCopy.count > RESET_COUNTS) {
      action = reset;
    } else if (btnCopy.count > LEVEL_CHANGE_COUNTS) {
      action = step;
    } else {
      action = onoff;
    }
  }

  idleButton(action);

  return action;
}

void CButton::idleButton(buttonaction action) {
  if (action != none) { // if idle do nothing
#ifdef BUTTON_HW_DEBUG
    logger.printf(LOG_BUTTON, "Action: " + String(action));
#endif
    portENTER_CRITICAL(&isrMux); // if action, always idle
    btnData.state = idle;
    portEXIT_CRITICAL(&isrMux);
  }
}

byte CButton::nextStep() {
  byte CurrentStep = ((waveform.getPower() + (STEP_SIZE/2)) / STEP_SIZE) * STEP_SIZE;
  if (StepUp) {
    if (CurrentStep >= PWR_ON) {
      CurrentStep -= STEP_SIZE;
      StepUp = false;
    } else {
      CurrentStep += STEP_SIZE;
    }
  } else {
    if (CurrentStep <= PWR_OFF) {
      CurrentStep += STEP_SIZE;
      StepUp = true;
    } else {
      CurrentStep -= STEP_SIZE;
    }
  }
  return CurrentStep;
}

void IRAM_ATTR CButton::isr_button() {
  portENTER_CRITICAL_ISR(&isrMux);
  if (btnData.state == idle) {
    if (xTimerStartFromISR(timer, NULL) == pdPASS) {
      btnData.state = debounce;
      btnData.count = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&isrMux);
}

void CButton::timerCallback(TimerHandle_t xTimer) {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    portENTER_CRITICAL(&isrMux);
    if (!btnData.count) {
      btnData.state = idle;  
    } else if (btnData.state != idle) {
      btnData.state = fired; 
      btnData.count++;
    }
    portEXIT_CRITICAL(&isrMux);      
  } else if (btnData.state != idle) {
    portENTER_CRITICAL(&isrMux);
    btnData.state = pressed;
    btnData.count++;
    portEXIT_CRITICAL(&isrMux);
    xTimerStart(xTimer, portMAX_DELAY);
  }   
}

CButton button;
