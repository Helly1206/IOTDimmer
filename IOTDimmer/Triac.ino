/* 
 * IOTDimmer - Triac
 * Dimmer triac Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 15-5-2022
 * Copyright: Ivo Helwegen
 */

#include <math.h>
#include "Triac.h"
#include "HWtimer.h"

portMUX_TYPE CTriac::stateMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE CTriac::movAvMux = portMUX_INITIALIZER_UNLOCKED;
volatile CTriac::triacdata CTriac::triacData = {CTriac::idle, 0, 0, 0, {0}, 0,0, false};
volatile unsigned long CTriac::zeroSample = 0;

CTriac::CTriac() { // constructor
  pinMode(ZEROCROSS_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  dimMode = timed;
  zeroState = false;
  lpCallback = NULL;
  triacData.state = zerouncalibrated;
  triacData.igniteTime = 0;
  triacData.pulseWidth = 0;
  triacData.zeroStamp = 0;
  triacData.movAvMemory[MOVAV_WIDTH] = {0};
  triacData.movAvCounter = 0;
  triacData.stabilizerCounter = 0;
  triacData.movAvValid = false;
}

void CTriac::init(void) {
  hwtimer.init(hwtimer.getPrescaler(ZERO_MAX), TMR_SINGLE);
  hwtimer.attachInterrupt(isr_timer);
  setMode(settings.getByte(settings.TriacMode));
  setPower(PWR_OFF);
  calibrateZero();
}

void CTriac::handle(void) {
  testZeroCalibrated();
}

void CTriac::reset(void) {
  logger.printf(LOG_TRIAC, "Reset");
  setPower(PWR_OFF);
  ClearMovAvFilter();
  triacData.state = zerouncalibrated;
  calibrateZero();
}

float CTriac::getFreq(void) {
  float freq = 0;
  unsigned long zeroTime;
  zeroTime = getZeroTime();
  if (zeroTime >= ZERO_MIN) {
    freq = 1.0/((2.0*zeroTime)/1000000.0);
  }
  return freq;
}

void CTriac::setPower(byte power) {
  unsigned long ignTime = 0;
  //logger.printf(LOG_TRIAC, "Setpower: " + String(power)); // don't log too much data

  if (triacData.state < zerouncalibrated) { 
    if (dimMode == timed) {
      ignTime = calcTimedMode(power);
    } else { // power
      ignTime = calcPowerMode(power);
    }
    setIgniteTime(ignTime, power);
    setState(power);
    setZero(power);
  } else {
    setState(PWR_OFF);
  }
}

byte CTriac::getPower() {
  byte power = PWR_OFF;
  unsigned long ignTime = getIgniteTime();

  if (triacData.state == on) {
    power = PWR_ON;
  } else if (triacData.state < off) {
    if (dimMode == timed) {
      power = calcFromTimedMode(ignTime);
    } else { // power
      power = calcFromPowerMode(ignTime);
    }
  }
  
  return power;
}

void CTriac::setMode(byte mode) {
  logger.printf(LOG_TRIAC, "Setmode: " + String(mode));
  dimMode = (triacmode)mode;
}

byte CTriac::getMode() {
  return (byte)dimMode;
}

void CTriac::setCallback(void *cb) {
  if (cb) {
    lpCallback = (lowpower_cb)cb; 
  } else {
    lpCallback = NULL;
  }
}

// Privates !!!!!!!!!!!!!

void CTriac::setZero(byte power) {
  if (power == PWR_OFF) {
    zeroOff();
    if (lpCallback != NULL) {
      lpCallback(false);
    }
  } else {
    zeroOn();
    if (lpCallback != NULL) {
      lpCallback(true);
    }
  }
}

void CTriac::zeroOn() {
  if (!zeroState) {
    attachInterrupt(digitalPinToInterrupt(ZEROCROSS_PIN), isr_ext, RISING);
    zeroState = true;
  }
}

void CTriac::zeroOff() {
  if (zeroState) {
    detachInterrupt(digitalPinToInterrupt(ZEROCROSS_PIN));
    zeroState = false;
  }
}

void CTriac::calibrateZero() {
  zeroOn();
  portENTER_CRITICAL(&stateMux);
  triacData.state = zerocalibrating;
  portEXIT_CRITICAL(&stateMux);
}

void CTriac::testZeroCalibrated() {
  if (triacData.state == zerocalibrating) {
    if (triacData.stabilizerCounter >= STABILIZER_NR) {
      zeroOff();
      portENTER_CRITICAL(&stateMux);
      triacData.state = off;
      portEXIT_CRITICAL(&stateMux);
    }
  }
}

unsigned long CTriac::calcPowerMode(byte power) {
  float freq = getFreq();
  unsigned long ignTime = IGNITION_MAX;
  if (freq > 0) {
    ignTime = (unsigned long)round((acos(power/50.0 - 1.0) * 1000000.0) / (2*M_PI*freq));
  }
  return ignTime;
}

unsigned long CTriac::calcTimedMode(byte power) {
  unsigned long zeroTime = getZeroTime();
  unsigned long ignTime = IGNITION_MAX;
  if (zeroTime >= ZERO_MIN) {
    ignTime = ((zeroTime*(PWR_ON-power)) / 100);
  }
  return ignTime;
}

byte CTriac::calcFromPowerMode(unsigned long ignTime) {
  float freq = getFreq();
  return (byte)round((1.0 + cos(2.0*M_PI*freq*(ignTime/1000000.0))) * 50.0);
}

byte CTriac::calcFromTimedMode(unsigned long ignTime) {
  unsigned long zeroTime = getZeroTime();
  return (byte)(PWR_ON - (ignTime*100 /zeroTime));
}

void CTriac::setIgniteTime(unsigned long ignTime, byte &power) {
  portENTER_CRITICAL(&stateMux);
  unsigned long zeroTime = getZeroTime();
  triacData.igniteTime = ignTime;
  triacData.pulseWidth = zeroTime/100;
  // check safety
  if (triacData.igniteTime > ZERO_MAX) {
    if (&power != NULL) {
      power = PWR_OFF;
    }
  } else if ((triacData.igniteTime + triacData.pulseWidth) > (zeroTime - SAFETY_TIME_US)) {
    triacData.igniteTime -= SAFETY_TIME_US;
  }
  portEXIT_CRITICAL(&stateMux);
}

unsigned long CTriac::getIgniteTime() {
  return triacData.igniteTime;
}

void CTriac::setState(byte power) {
  portENTER_CRITICAL(&stateMux);
  if (triacData.state < zerouncalibrated) {
    if (power == PWR_OFF) {
      triacData.state = off;
      digitalWrite(TRIGGER_PIN, LOW);
    } else if (power == PWR_ON) {
      triacData.state = on;
      digitalWrite(TRIGGER_PIN, HIGH);
    } else if (triacData.state > pulse) {
      triacData.state = idle;
    }
  } else {
    digitalWrite(TRIGGER_PIN, LOW);
  } 
  portEXIT_CRITICAL(&stateMux);
}

unsigned long CTriac::getZeroTime() {
  int i;
  unsigned long sumMemory = 0;

  if (triacData.movAvValid) {
    portENTER_CRITICAL(&movAvMux);
    for (i=0; i<MOVAV_WIDTH; i++) {
      sumMemory += triacData.movAvMemory[i];
    }
    portEXIT_CRITICAL(&movAvMux);
  } else {
    sumMemory = MOVAV_WIDTH; //prevent div 0 if not valid
  }
  return (sumMemory/MOVAV_WIDTH);
}

void CTriac::ClearMovAvFilter() {
  portENTER_CRITICAL(&movAvMux);
  triacData.movAvMemory[MOVAV_WIDTH] = { 0 };
  triacData.movAvCounter = 0;
  triacData.stabilizerCounter = 0;
  triacData.movAvValid = false;
  portEXIT_CRITICAL(&movAvMux);
}

void IRAM_ATTR CTriac::isr_ext() {
  zeroSample = micros() - triacData.zeroStamp;
  if (zeroSample >= ZERO_MIN) {
    triacData.zeroStamp += zeroSample;
    if (zeroSample <= ZERO_MAX) {
      portENTER_CRITICAL_ISR(&movAvMux);
      triacData.movAvMemory[triacData.movAvCounter] = zeroSample;
      if (triacData.movAvCounter < MOVAV_WIDTH-1) {
        triacData.movAvCounter++;
      } else {
        triacData.movAvValid = true;
        triacData.movAvCounter = 0;
        triacData.stabilizerCounter++;
      }
      portEXIT_CRITICAL_ISR(&movAvMux);
      if (triacData.state == idle) {
        portENTER_CRITICAL_ISR(&stateMux);
        triacData.state = zero;
        hwtimer.trigger(triacData.igniteTime);
        portEXIT_CRITICAL_ISR(&stateMux);
      }
    }
  }
}

void IRAM_ATTR CTriac::isr_timer() {
  portENTER_CRITICAL_ISR(&stateMux);
  if (triacData.state == zero) {
    digitalWrite(TRIGGER_PIN, HIGH);
    triacData.state = pulse;
    hwtimer.trigger(triacData.pulseWidth);
  } else if (triacData.state == pulse) {
    digitalWrite(TRIGGER_PIN, LOW);
    triacData.state = idle;
  }
  portEXIT_CRITICAL_ISR(&stateMux);
}

CTriac triac;
