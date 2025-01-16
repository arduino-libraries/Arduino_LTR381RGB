/*
  This file is part of the Arduino_LTR381RGB library.
  Copyright (c) 2021 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

enum {
  ADC_RES_16BIT = 0x00,
  ADC_RES_17BIT = 0x01,
  ADC_RES_18BIT = 0x02,
  ADC_RES_19BIT = 0x03,
  ADC_RES_20BIT = 0x04,
};

enum {
  ADC_MEAS_RATE_25MS = 0x00,
  ADC_MEAS_RATE_50MS = 0x01,
  ADC_MEAS_RATE_100MS = 0x02,
  ADC_MEAS_RATE_200MS = 0x03,
  ADC_MEAS_RATE_400MS = 0x04,
};

enum {
  ALS_CS_GAIN_1 = 0x00,
  ALS_CS_GAIN_3 = 0x01,
  ALS_CS_GAIN_6 = 0x02,
  ALS_CS_GAIN_9 = 0x03,
  ALS_CS_GAIN_18 = 0x04,
};

LTR381RGBClass::LTR381RGBClass(TwoWire& wire, uint8_t slaveAddress) :
  _wire(&wire),
  _slaveAddress(slaveAddress)
{
}

LTR381RGBClass::~LTR381RGBClass()
{
}

int LTR381RGBClass::begin() {
  _wire->begin();

  uint8_t res = readRegister(LTR381RGB_PART_ID);
  if ((res & 0xF0) != 0xC0) {
    return 0;
  }

  res = readRegister(LTR381RGB_MAIN_STATUS);
  // Check if Power-On event happens
  if ((res & 0x20) != 0) {
    return 0;
  }

  enableALSInterrupt();
  setGain(ALS_CS_GAIN_1);

  setUpperThreshold(10000);
  setLowerThreshold(100);
  return 1;
}

void LTR381RGBClass::end() {
  _wire->end();
}

int LTR381RGBClass::readColors(int& r, int& g, int& b) {
  enableRGB();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout);
  uint8_t buf[9] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_GREEN, buf, 9);
  if(res != 1) {
    return 0;
  }
  g = buf[2] << 16 | buf[1] << 8 | buf[0];
  r = buf[5] << 16 | buf[4] << 8 | buf[3];
  b = buf[8] << 16 | buf[7] << 8 | buf[6];

  return 1;
}

int LTR381RGBClass::readAmbientLight(int& lux) {
  enableALS();
  unsigned long start = millis();
  while(!available() && (millis() - start) < _timeout);
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
  while(!available() && (millis() - start) < _timeout);
  uint8_t buf[6] = {0};
  int res = readRegisters(LTR381RGB_CS_DATA_IR, buf, 6);
  if(res != 1) {
    return 0;
  }
  int ir = buf[2] << 16 | buf[1] << 8 | buf[0];
  int csg = buf[5] << 16 | buf[4] << 8 | buf[3];
  lux = ((0.8f*csg)/(_gain*_adcResolution))*(1 - (0.033f*(ir/csg)));
  return 1;
}


void LTR381RGBClass::setGain(int gain) {
  writeRegister(LTR381RGB_ALS_CS_GAIN, (0x07 & gain));
  _gain = getLuxGain(gain);
}

void LTR381RGBClass::setADCResolution(int resolution) {
  uint8_t res = readRegister(LTR381RGB_ALS_CS_MEAS_RATE);
  writeRegister(LTR381RGB_ALS_CS_MEAS_RATE, ((res & 0xF8) | resolution));
  _adcResolution = getLuxResolution(resolution);
}

void LTR381RGBClass::setMeasurementRate(int rate) {
  uint8_t res = readRegister(LTR381RGB_ALS_CS_MEAS_RATE);
  writeRegister(LTR381RGB_ALS_CS_MEAS_RATE, ((res & 0x8F) | (rate << 4)));
}

void LTR381RGBClass::setUpperThreshold(int utv) {
  uint8_t utv2 = utv >> 16;
  uint8_t utv1 = utv >> 8;
  uint8_t utv0 = utv & 0xFF;

  writeRegister(LTR381RGB_ALS_THRES_UTV_1, utv0);
  writeRegister(LTR381RGB_ALS_THRES_UTV_2, utv1);
  writeRegister(LTR381RGB_ALS_THRES_UTV_3, 0x0F & utv2);
}

void LTR381RGBClass::setLowerThreshold(int ltv) {
  uint8_t ltv2 = ltv >> 16;
  uint8_t ltv1 = ltv >> 8;
  uint8_t ltv0 = ltv & 0xFF;

  writeRegister(LTR381RGB_ALS_THRES_LTV_1, ltv0);
  writeRegister(LTR381RGB_ALS_THRES_LTV_2, ltv1);
  writeRegister(LTR381RGB_ALS_THRES_LTV_3, 0x0F & ltv2);
}

void LTR381RGBClass::setTimeout(unsigned long timeout) {
  _timeout = timeout;
}

void LTR381RGBClass::enableALSInterrupt() {
  uint8_t res = readRegister(LTR381RGB_INT_CFG);
  writeRegister(LTR381RGB_INT_CFG, res | 0x03);
}

void LTR381RGBClass::disableALSInterrupt() {
  uint8_t res = readRegister(LTR381RGB_INT_CFG);
  writeRegister(LTR381RGB_INT_CFG, res & 0xFB);
}

void LTR381RGBClass::resetSW() {
  uint8_t res = readRegister(LTR381RGB_MAIN_CTRL);
  writeRegister(LTR381RGB_MAIN_CTRL, res | 0x80);
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
  }
  return _gain;
}

float LTR381RGBClass::getLuxResolution(int resolution) {
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
  }
  return _adcResolution;
}

int LTR381RGBClass::available() {
  auto res = readRegister(LTR381RGB_MAIN_STATUS);
  if ((res & 0x08) == 0x08) {
    return 1;
  }
  return 0;
}

void LTR381RGBClass::enableRGB() {
  writeRegister(LTR381RGB_MAIN_CTRL, 0x06);
}

void LTR381RGBClass::enableALS() {
  writeRegister(LTR381RGB_MAIN_CTRL, 0x02);
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
