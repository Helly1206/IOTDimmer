/* 
 * IOTDimmer - Waveform
 * Dimmer waveform generation
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 27-5-2022
 * Copyright: Ivo Helwegen
 */

#include <math.h>
#include "Waveform.h"

portMUX_TYPE CWaveform::mux = portMUX_INITIALIZER_UNLOCKED;
boolean CWaveform::doEffect = false;

CWaveform::CWaveform() { // constructor
  power = PWR_OFF;
  prevPower = power;
  triacMode = (byte)triac.timed;
}

void CWaveform::init(void) {
  power = PWR_OFF;
  effectInput = 0;
  setMode(settings.getByte(settings.WaveMode));
  setEffect(settings.getByte(settings.WaveEffect));
  effPower = PWR_OFF;
  prevPower = PWR_OFF;
  startPower = PWR_OFF;
  modeConvDone = true;
  randomSeed(analogRead(SEED_PIN));
  modeTimer = xTimerCreateStatic("", pdMS_TO_TICKS(settings.getShort(settings.WaveMode100Percent)), pdFALSE, (void *)MODE_TIMER, timerCallback, &modeTimerBuffer);
  effTimer = xTimerCreateStatic("", pdMS_TO_TICKS(settings.getShort(settings.WaveEffectTime)), pdTRUE, (void *)EFFECT_TIMER, timerCallback, &effTimerBuffer);
}

void CWaveform::handle(void) {
  // calc mode
  byte calcPower;
  if (triac.getMode() != triacMode) {
    triacMode = triac.getMode();
    if (effect == enone) {
      setPower(triac.getPower());
    }
  }
  calcEffPower();
  if (effPower != prevPower) {
    calcPower = calcMode();
    if (calcPower != prevPower) {
      triac.setPower(calcPower);
      prevPower = calcPower;
    }
  }
}

void CWaveform::setPower(byte ipower) {
  logger.printf("Power: " + String(ipower));
  power = ipower;
}

byte CWaveform::getPower() {
  return power;
}

void CWaveform::setInput(int iinput) {
  logger.printf(LOG_WAVEFORM, "Input: " + String(iinput));
  effectInput = iinput;
}

int CWaveform::getInput() {
  return effectInput;
}

void CWaveform::setMode(byte imode) {
  logger.printf(LOG_WAVEFORM, "Mode: " + String(imode));
  mode = (waveformmode)imode;
}

byte CWaveform::getMode() {
  return (byte)mode;
}

boolean CWaveform::getStatus() {
  return getPower() > settings.getByte(settings.LevelOff);
}

CWaveform::waveformmode CWaveform::getModeEnum() {
  return mode;
}

void CWaveform::setEffect(byte ieffect) {
  logger.printf(LOG_WAVEFORM, "Effect: " + String(ieffect));
  if ((waveformeffect)ieffect != effect) {
    if (((waveformeffect)ieffect != enone) && ((waveformeffect)ieffect != einput)) {
      xTimerChangePeriod(effTimer, pdMS_TO_TICKS(settings.getShort(settings.WaveEffectTime)), portMAX_DELAY);      
    } else {
      xTimerStop(effTimer, portMAX_DELAY);
    }
  }
  effect = (waveformeffect)ieffect;
}

byte CWaveform::getEffect() {
  return (byte)effect;
}

CWaveform::waveformeffect CWaveform::getEffectEnum() {
  return effect;
}

// Privates !!!!!!!!!!!!!

void CWaveform::updateMode(byte ipower) {
  unsigned long modeTime = 0;
  if (mode != instant) {
    startPower = prevPower;
    if (ipower < startPower) {
      modeTime = ((unsigned long)(startPower - ipower) * settings.getShort(settings.WaveMode100Percent)) / 100;
    } else {
      modeTime = ((unsigned long)(ipower - startPower) * settings.getShort(settings.WaveMode100Percent)) / 100;
    }
    xTimerChangePeriod(modeTimer, pdMS_TO_TICKS(modeTime), portMAX_DELAY);
    modeConvDone = false;
  } else {
    modeConvDone = true;
  }
}

byte CWaveform::calcMode() {
  byte ipower = PWR_OFF;
  if (modeConvDone) {
    ipower = effPower;
  } else {
    if (xTimerIsTimerActive(modeTimer) == pdFALSE) {
      ipower = effPower;
      modeConvDone = true;
    } else {
      switch (mode) {
        case linear:
          ipower = calcModeLinear();
          break;
        case sine:
          ipower = calcModeSine();
          break;
        case qsine:
          ipower = calcModeQsine();
          break;
        default:
          ipower = effPower;
      }
    }
  }
  return ipower;
}

unsigned long CWaveform::getElapsedPercent() {
  return (100*(xTimerGetPeriod(modeTimer) - (xTimerGetExpiryTime(modeTimer) - xTaskGetTickCount())))/xTimerGetPeriod(modeTimer);
}

byte CWaveform::calcModeLinear() {
  byte ipower = PWR_OFF;
  if (effPower < startPower) {
    ipower = (byte)(startPower - (((startPower - effPower)*getElapsedPercent())/100));
  } else {
    ipower = (byte)(startPower + (((effPower - startPower)*getElapsedPercent())/100));
  }
  return ipower;
}

byte CWaveform::calcModeSine()  {
  byte ipower = PWR_OFF;
  //(1+/sin(2*PI*t/2*T-PI/4))/2 = (1+sin(PI*t/T-PI/4))/2;
  if (effPower < startPower) {
    ipower = (byte)(startPower - round((startPower - effPower)*(1+sin(M_PI*getElapsedPercent()/100-(M_PI/4)))/2));
  } else {
    ipower = (byte)(startPower + round((effPower - startPower)*(1+sin(M_PI*getElapsedPercent()/100-(M_PI/4)))/2));
  }
  return ipower;
}

byte CWaveform::calcModeQsine() {
  byte ipower = PWR_OFF;
  //sin(2*PI*t/4*T) = sin(PI*t/2*T);
  if (effPower < startPower) {
    ipower = (byte)(startPower - round((startPower - effPower)*sin(M_PI*getElapsedPercent()/200)));
  } else {
    ipower = (byte)(startPower + round((effPower - startPower)*sin(M_PI*getElapsedPercent()/200)));
  }
  return ipower;
}

void CWaveform::calcEffPower() {
  byte ieffPower = PWR_OFF;
  switch (effect) {
    case eramp:
      ieffPower =  calcEffRamp();
      break;
    case esine:
      ieffPower =  calcEffSine();
      break;
    case erandom:
      ieffPower =  calcEffRandom();
      break;
    case einput:
      ieffPower =  calcEffInput();
      break;
    default:
      ieffPower = power;
  }
  if (ieffPower != effPower) {
    updateMode(ieffPower);
    effPower = ieffPower;
  }
}

unsigned long CWaveform::getEffectElapsedPercent() {
  return (100*(xTimerGetPeriod(effTimer) - (xTimerGetExpiryTime(effTimer) - xTaskGetTickCount())))/xTimerGetPeriod(effTimer);
}

byte CWaveform::calcEffRamp() {
  short pwr = PWR_OFF;
  byte effectMagnitude = settings.getByte(settings.WaveEffectMagnitude);
  unsigned short effectTime = settings.getShort(settings.WaveEffectTime);
  if (effDo(pwr)) {
    unsigned long elapsedPercent = getEffectElapsedPercent();
    if (elapsedPercent < 50) { //positive ramp
      pwr = power - effectMagnitude + (((2*effectMagnitude)*elapsedPercent)/50);
    } else { //negative ramp
      pwr = power + effectMagnitude - (((2*effectMagnitude)*(elapsedPercent - 50))/50);
    }
    effRange(pwr);
  }
  return (byte)pwr;
}

byte CWaveform::calcEffSine() {
  short pwr = PWR_OFF;
  if (effDo(pwr)) {
    pwr = power + round((settings.getByte(settings.WaveEffectMagnitude))*sin(2*M_PI*getEffectElapsedPercent()/100));
    effRange(pwr);
  }
  return (byte)pwr;
}

byte CWaveform::calcEffRandom() {
  short pwr = PWR_OFF;
  byte effectMagnitude = settings.getByte(settings.WaveEffectMagnitude);
  if (effDo(pwr)) {
    if (doEffect) {
      portENTER_CRITICAL(&mux);
      doEffect = false;
      portEXIT_CRITICAL(&mux);
      pwr = random(power - effectMagnitude, power + effectMagnitude);
      effRange(pwr);
    } else {
      pwr = effPower;
    }
  }
  return (byte)pwr;
}

byte CWaveform::calcEffInput() {
  short pwr = PWR_OFF;
  if (effDo(pwr)) {
    pwr = power + (short)round(effectInput*settings.getFloat(settings.WaveEffectGain));
    effRange(pwr);
  }
  return (byte)pwr;
}

bool CWaveform::effDo(short &pwr) {
  bool val = true;
  if ((power == PWR_OFF) || (power == PWR_ON)) {
    pwr = power;
    val = false;
  }
  return val;
}

void CWaveform::effRange(short &pwr) {
  if (pwr >= PWR_ON) {
    pwr = PWR_ON-1;
  }
  if (pwr <= PWR_OFF) {
    pwr = PWR_OFF+1;
  }
}

void CWaveform::timerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == EFFECT_TIMER) {
    portENTER_CRITICAL(&mux);
    doEffect = true;
    portEXIT_CRITICAL(&mux);
  }
}

CWaveform waveform;
