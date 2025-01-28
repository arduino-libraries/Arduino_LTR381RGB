/*
  Copyright (c) 2025 Arduino SA
  This Source Code Form is subject to the terms of the Mozilla
  Public License (MPL), v 2.0. You can obtain a copy of the MPL
  at http://mozilla.org/MPL/2.0/
*/

#include "LTR381RGB.h"

#define LTR381RGB_ADDRESS                    0x53

#define LTR381RGB_MAIN_CTRL                  0X00
#define LTR381RGB_ALS_CS_MEAS_RATE           0X04
#define LTR381RGB_ALS_CS_GAIN                0X05

#define LTR381RGB_PART_ID                    0X06
#define LTR381RGB_MAIN_STATUS                0X07

#define LTR381RGB_CS_DATA_IR                 0X0A
#define LTR381RGB_CS_DATA_GREEN              0X0D
#define LTR381RGB_CS_DATA_RED                0X10
#define LTR381RGB_CS_DATA_BLUE               0X13
#define LTR381RGB_ALS_DATA                   LTR381RGB_CS_DATA_GREEN

#define LTR381RGB_INT_CFG                    0X19
#define LTR381RGB_INT_PST                    0X1A
#define LTR381RGB_ALS_THRES_UTV_1            0X21
#define LTR381RGB_ALS_THRES_UTV_2            0X22
#define LTR381RGB_ALS_THRES_UTV_3            0X23

#define LTR381RGB_ALS_THRES_LTV_1            0X24
#define LTR381RGB_ALS_THRES_LTV_2            0X25
#define LTR381RGB_ALS_THRES_LTV_3            0X26


LTR381RGBClass::LTR381RGBClass(TwoWire& wire, uint8_t slaveAddress) :
  _wire(&wire),
  _slaveAddress(slaveAddress)
{
}

LTR381RGBClass::~LTR381RGBClass()
{
}

int LTR381RGBClass::begin(int gain, int rate, int resolution, int pin, int ut, int lt) {
  _wire->begin();

  uint8_t res = readRegister(LTR381RGB_PART_ID);
  if ((res & 0xF0) != 0xC0) {
#ifdef DEBUG
    dumpReg();
#endif
    return 0;
  }
  // Clear the power-on event bit after the first initialization
  res = readRegister(LTR381RGB_MAIN_STATUS);

  if(pin > -1) {
    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), _callback, FALLING);
    enableALSInterrupt();
    _irq = true;
  }

  setGain(gain);
  setMeasurementRate(rate);
  setADCResolution(resolution);
  setUpperThreshold(ut);
  setLowerThreshold(lt);
  return 1;
}

void LTR381RGBClass::end() {
  _wire->end();
}


int LTR381RGBClass::readAllSensors(int& r, int& g, int& b, int& rawlux, int& lux, int& ir) {
  enableRGB();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  uint8_t cs_buf[9] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_GREEN, cs_buf, 9);
  if(res != 1) {
    return 0;
  }

  enableALS();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t als_buf[6] = {0};
  res = readRegisters(LTR381RGB_CS_DATA_IR, als_buf, 6);
  if(res != 1) {
    return 0;
  }
  getColors(cs_buf, r, g, b);
  getALS(als_buf, ir, rawlux, lux);
  return 1;
}

int LTR381RGBClass::readColors(int& r, int& g, int& b) {
  enableRGB();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t buf[9] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_GREEN, buf, 9);
  if(res != 1) {
    return 0;
  }
  getColors(buf, r, g, b);

  return 1;
}

int LTR381RGBClass::readRawColors(int& r, int& g, int& b) {
  enableRGB();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t buf[9] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_GREEN, buf, 9);
  if(res != 1) {
    return 0;
  }
  int resolution = getADCResolution(_adcResolution);
  g = resolution & (buf[2] << 16 | buf[1] << 8 | buf[0]);
  r = resolution & (buf[5] << 16 | buf[4] << 8 | buf[3]);
  b = resolution & (buf[8] << 16 | buf[7] << 8 | buf[6]);

  return 1;
}

int LTR381RGBClass::readAmbientLight(int& lux) {
  enableALS();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t buf[3] = {0};
  int res = readRegisters(LTR381RGB_ALS_DATA, buf, 3);
  if(res != 1) {
    return 0;
  }
  lux = buf[2] << 16 | buf[1] << 8 | buf[0];

  return 1;
}

int LTR381RGBClass::readLux(int& lux) {
  enableALS();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t buf[6] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_IR, buf, 6);
  if(res != 1) {
    return 0;
  }
  int gain = getLuxGain(_gain);
  float intTime = getLuxIntTime(_adcResolution);
  int ir = buf[2] << 16 | buf[1] << 8 | buf[0];
  int csg = buf[5] << 16 | buf[4] << 8 | buf[3];
  lux = ((0.8f*csg)/(gain*intTime))*(1 - (0.033f*(ir/csg)));

  return 1;
}

int LTR381RGBClass::readIR(int& ir) {
  enableALS();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout) {
    delay(50);
  }
  disableMeas();
  uint8_t buf[3] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_IR, buf, 3);
  if(res != 1) {
    return 0;
  }
  ir = buf[2] << 16 | buf[1] << 8 | buf[0];

  return 1;
}

void LTR381RGBClass::setGain(int gain) {
  if (gain > 0x04) {
    gain = 0x04;
  }
  _gain = gain;
  writeRegister(LTR381RGB_ALS_CS_GAIN, (0x07 & gain));
}

void LTR381RGBClass::setADCResolution(int resolution) {
  if (resolution > 0x04) {
    resolution = 0x04;
  }
  uint8_t res = readRegister(LTR381RGB_ALS_CS_MEAS_RATE);
  writeRegister(LTR381RGB_ALS_CS_MEAS_RATE, ((res & 0x8F) | (resolution << 4)));
  setTimeout(getADCResTime(resolution) + getADCRate(_rate));
  _adcResolution = resolution;
}

void LTR381RGBClass::setMeasurementRate(int rate) {
  if (rate > 0x08) {
    rate = 0x08;
  }
  uint8_t res = readRegister(LTR381RGB_ALS_CS_MEAS_RATE);
  writeRegister(LTR381RGB_ALS_CS_MEAS_RATE, ((res & 0xF8) | rate));
  setTimeout(getADCResTime(_adcResolution) + getADCRate(rate));
  _rate = rate;
}

void LTR381RGBClass::setUpperThreshold(int utv) {
  uint8_t utv2 = utv >> 16;
  uint8_t utv1 = utv >> 8;
  uint8_t utv0 = utv & 0xFF;

  writeRegister(LTR381RGB_ALS_THRES_UTV_1, utv0);
  writeRegister(LTR381RGB_ALS_THRES_UTV_2, utv1);
  writeRegister(LTR381RGB_ALS_THRES_UTV_3, 0x0F & utv2);
  _upperThreshold = utv;
}

void LTR381RGBClass::setLowerThreshold(int ltv) {
  uint8_t ltv2 = ltv >> 16;
  uint8_t ltv1 = ltv >> 8;
  uint8_t ltv0 = ltv & 0xFF;

  writeRegister(LTR381RGB_ALS_THRES_LTV_1, ltv0);
  writeRegister(LTR381RGB_ALS_THRES_LTV_2, ltv1);
  writeRegister(LTR381RGB_ALS_THRES_LTV_3, 0x0F & ltv2);
  _lowerThreshold = ltv;
}

void LTR381RGBClass::setTimeout(unsigned long timeout) {
  if (timeout > 200) {
    _timeout = timeout;
  } else {
    _timeout = 200;
  }
}

int LTR381RGBClass::getADCResTime(int resolution) {
  switch(resolution) {
    case ADC_RES_16BIT:
      return 25;
    case ADC_RES_17BIT:
      return 50;
    case ADC_RES_18BIT:
      return 100;
    case ADC_RES_19BIT:
      return 200;
    case ADC_RES_20BIT:
      return 400;
    default:
      return 100;
  }
}

int LTR381RGBClass::getADCRate(int rate) {
  switch(rate) {
    case ADC_MEAS_RATE_25MS:
      return 25;
    case ADC_MEAS_RATE_50MS:
      return 50;
    case ADC_MEAS_RATE_100MS:
      return 100;
    case ADC_MEAS_RATE_200MS:
      return 200;
    case ADC_MEAS_RATE_400MS:
      return 400;
    case ADC_MEAS_RATE_500MS:
      return 500;
    case ADC_MEAS_RATE_1000MS:
      return 1000;
    case ADC_MEAS_RATE_2000MS:
      return 2000;
    case ADC_MEAS_RATE_2000MS + 1:
      return 2000;
    default:
      return 100;
  }
}

void LTR381RGBClass::setInterruptPersistence(int persistence) {
  writeRegister(LTR381RGB_INT_PST, (persistence << 4));
}

void LTR381RGBClass::enableALSInterrupt() {
  uint8_t res = readRegister(LTR381RGB_INT_CFG);
  writeRegister(LTR381RGB_INT_CFG, res | 0x04);
}

void LTR381RGBClass::disableALSInterrupt() {
  uint8_t res = readRegister(LTR381RGB_INT_CFG);
  writeRegister(LTR381RGB_INT_CFG, res & 0xFB);
}

void LTR381RGBClass::resetSW() {
  uint8_t res = readRegister(LTR381RGB_MAIN_CTRL);
  writeRegister(LTR381RGB_MAIN_CTRL, res | 0x80);
}

void LTR381RGBClass::setCalibrations(int rmax, int gmax, int bmax, int rmin, int gmin, int bmin) {
  _maxR = rmax;
  _maxG = gmax;
  _maxB = bmax;
  _minR = rmin;
  _minG = gmin;
  _minB = bmin;
  _calibrated = true;
}

void LTR381RGBClass::setCallback(callback_f callback) {
  _callback = callback;
}

void LTR381RGBClass::getHSV(int r, int g, int b, float& h, float& s, float& v) {
  float red = r/255.0;
  float green = g/255.0;
  float blue = b/255.0;
  float cmax = max(red, max(blue, green));
  float cmin = min(red, min(blue, green));
  float delta = cmax - cmin;
  if(delta == 0) {
    h = 0;
  } else if (cmax == red) {
    h = 60*fmod((((green - blue) / delta)),6);
  } else if(cmax == green) {
    h = 60*(((blue - red) / delta) + 2);
  } else {
    h = 60*(((red - green) / delta) + 4);
  }

  if(cmax == 0) {
    s = 0;
  } else {
    s = delta/cmax;
  }

  v = cmax * 100.0;
  s = s*100;
}

void LTR381RGBClass::getHSL(int r, int g, int b, float& h, float& s, float& l) {
  float red = r/255.0;
  float green = g/255.0;
  float blue = b/255.0;
  float cmax = max(red, max(blue, green));
  float cmin = min(red, min(blue, green));
  float delta = cmax - cmin;
  l = (cmax + cmin)/2;
  if(delta == 0) {
    h = 0;
  } else if (cmax == red) {
    h = 60*fmod((((green - blue) / delta)),6);
  } else if(cmax == green) {
    h = 60* (( (blue - red) / delta) + 2);
  } else {
    h = 60*(((red - green) / delta) + 4);
  }

  if(delta == 0) {
    s = 0.0;
  } else {
    s = (delta/(1 - abs(2*l -1)));
  }
  l = l*100;
  s= s*100;
}

void LTR381RGBClass::getALS(uint8_t * buf, int& ir, int& rawlux, int& lux) {
  int resolution = getADCResolution(_adcResolution);
  int gain = getLuxGain(_gain);
  float intTime = getLuxIntTime(_adcResolution);

  ir = resolution &  buf[2] << 16 | buf[1] << 8 | buf[0];
  rawlux = resolution & buf[5] << 16 | buf[4] << 8 | buf[3];
  lux = ((0.8f*rawlux)/(gain*intTime))*(1 - (0.033f*(ir/rawlux)));
}

void LTR381RGBClass::getColors(uint8_t * buf, int& r, int& g, int& b) {
  if (_calibrated) {
    int lg = adcToValue((buf[2] << 16 | buf[1] << 8 | buf[0]));
    int lr = adcToValue((buf[5] << 16 | buf[4] << 8 | buf[3]));
    int lb = adcToValue((buf[8] << 16 | buf[7] << 8 | buf[6]));
    r = normAndTrim(lr, _minR, _maxR);
    g = normAndTrim(lg, _minG, _maxG);
    b = normAndTrim(lb, _minB, _maxB);

  } else {
    g = adcToValue((buf[2] << 16 | buf[1] << 8 | buf[0]));
    r = adcToValue((buf[5] << 16 | buf[4] << 8 | buf[3]));
    b = adcToValue((buf[8] << 16 | buf[7] << 8 | buf[6]));
  }
}


void LTR381RGBClass::dumpReg() {
  for (int i = 0x00; i < 0x27; i++) {
    if(i == 0x01 || i == 0x02  || i ==0x03 || i == 0x08 || i == 0x09
                 || i == 016 || i == 0x17 || i == 0x18 || i == 0x1B || i == 0x1C
                             || i == 0x1D || i == 0x1E || i == 0x1F || i == 0x20) {
      Serial.print("ADDRESS 0x");
      Serial.print(i, HEX);
      Serial.println(" IS RESERVED");
      continue;
    }

    int res = readRegister(i);
    Serial.print("REGISTER ADDRESS 0x");
    Serial.print(i, HEX);
    Serial.print(" VALUE= 0x");
    Serial.print(res, HEX);
    Serial.print(":");
    Serial.println(res, BIN);
  }
}

int LTR381RGBClass::getLuxGain(int gain) {
  switch(gain) {
    case ALS_CS_GAIN_1:
      return 1;
    case ALS_CS_GAIN_3:
      return 3;
    case ALS_CS_GAIN_6:
      return 6;
    case ALS_CS_GAIN_9:
      return 9;
    case ALS_CS_GAIN_18:
      return 18;
    default:
      return 6;
  }
}

int LTR381RGBClass::getADCResolution(int resolution) {
  switch(resolution) {
    case ADC_RES_16BIT:
      return 65534;
    case ADC_RES_17BIT:
      return 131071;
    case ADC_RES_18BIT:
      return 262143;
    case ADC_RES_19BIT:
      return 524287;
    case ADC_RES_20BIT:
      return 1048575;
    default:
      return 262143;
  }
}

float LTR381RGBClass::getLuxIntTime(int resolution) {
  switch(resolution) {
    case ADC_RES_16BIT:
      return 0.25;
    case ADC_RES_17BIT:
      return 0.5;
    case ADC_RES_18BIT:
      return 1.0f;
    case ADC_RES_19BIT:
      return 2.0f;
    case ADC_RES_20BIT:
      return 4.0f;
    default:
      return 1.0f;
  }
}

int LTR381RGBClass::available() {
  auto res = readRegister(LTR381RGB_MAIN_STATUS);
  if ((res & 0x20) == 0x20) {
#ifdef DEBUG
    Serial.println("Power-On event happens");
#endif
    resetSW();
#ifdef DEBUG
    dumpReg();
#endif
    // clear the reading after the reset
    end();
    begin();
    return 0;
  }
  if(_irq) {
    if ((res & 0x18) == 0x18) {
      return 1;
    }
  } else {
    if ((res & 0x08) == 0x08) {
      return 1;
    }
  }
  return 0;
}

void LTR381RGBClass::enableRGB() {
  writeRegister(LTR381RGB_MAIN_CTRL, 0x06);
}

void LTR381RGBClass::enableALS() {
  writeRegister(LTR381RGB_MAIN_CTRL, 0x02);
}

void LTR381RGBClass::disableMeas() {
  writeRegister(LTR381RGB_MAIN_CTRL, 0x00);
}

int LTR381RGBClass::normAndTrim(int color, int min, int max) {
  color = (color - min)*255/(max - min);
  if (color < 0) {
    color = 0;
  }
  if (color > 255) {
    color = 255;
  }
  return color;

}

int LTR381RGBClass::adcToValue(int adc) {
  int resolution = getADCResolution(_adcResolution);
  return ((resolution & adc)*255/(resolution));;
}


int LTR381RGBClass::readRegister(uint8_t address) {
  uint8_t value;
  if (readRegisters(address, &value, sizeof(value)) != 1) {
    return -1;
  }

  return value;
}

int LTR381RGBClass::readRegisters(uint8_t address, uint8_t* data, size_t length) {
  _wire->beginTransmission(_slaveAddress);
  _wire->write(address);

  if (_wire->endTransmission(false) != 0) {
    return -1;
  }

  if (_wire->requestFrom(_slaveAddress, length) != length) {
    return 0;
  }

  for (size_t i = 0; i < length; i++) {
    *data++ = _wire->read();
  }
  return 1;
}

int LTR381RGBClass::writeRegister(uint8_t address, uint8_t value) {
  _wire->beginTransmission(_slaveAddress);
  _wire->write(address);
  _wire->write(value);
  if (_wire->endTransmission() != 0) {
    return 0;
  }
  return 1;
}

LTR381RGBClass RGB(Wire1, LTR381RGB_ADDRESS);
