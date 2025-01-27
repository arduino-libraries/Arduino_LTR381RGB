/*
  Copyright (c) 2025 Arduino SA
  This Source Code Form is subject to the terms of the Mozilla
  Public License (MPL), v 2.0. You can obtain a copy of the MPL
  at http://mozilla.org/MPL/2.0/
*/

#include <Arduino.h>
#include <Wire.h>

enum {
  ADC_RES_20BIT = 0x00,
  ADC_RES_19BIT = 0x01,
  ADC_RES_18BIT = 0x02,
  ADC_RES_17BIT = 0x03,
  ADC_RES_16BIT = 0x04,
};

enum {
  ADC_MEAS_RATE_25MS = 0x00,
  ADC_MEAS_RATE_50MS = 0x01,
  ADC_MEAS_RATE_100MS = 0x02,
  ADC_MEAS_RATE_200MS = 0x03,
  ADC_MEAS_RATE_400MS = 0x04,
  ADC_MEAS_RATE_500MS = 0x05,
  ADC_MEAS_RATE_1000MS = 0x06,
  ADC_MEAS_RATE_2000MS = 0x07,
};

enum {
  ALS_CS_GAIN_1 = 0x00,
  ALS_CS_GAIN_3 = 0x01,
  ALS_CS_GAIN_6 = 0x02,
  ALS_CS_GAIN_9 = 0x03,
  ALS_CS_GAIN_18 = 0x04,
};

using callback_f = void (*)();
class LTR381RGBClass {
  public:
    LTR381RGBClass(TwoWire& wire, uint8_t slaveAddress);
    ~LTR381RGBClass();

    int begin(int gain = ALS_CS_GAIN_18, int rate = ADC_MEAS_RATE_25MS, int resolution = ADC_RES_16BIT, int pin = -1, int ut = 1000, int lt = 100);
    void end();

    int readAllSensors(int& r, int& g, int& b, int& rawlux, int& lux, int& ir);
    int readColors(int& r, int& g, int& b);
    int readRawColors(int& r, int& g, int& b);
    int readAmbientLight(int& lux);
    int readLux(int& lux);
    int readIR(int& ir);
    void setGain(int gain);
    void setADCResolution(int resolution);
    void setMeasurementRate(int rate);
    void setUpperThreshold(int utv);
    void setLowerThreshold(int ltv);
    void setTimeout(unsigned long timeout = 200);
    int getADCResTime(int resolution);
    int getADCRate(int rate);
    void setInterruptPersistence(int persistence);
    void enableALSInterrupt();
    void disableALSInterrupt();
    void resetSW();
    void setCalibrations(int rmax, int gmax, int bmax, int rmin, int gmin, int bmin);
    void setCallback(callback_f callback);
    void getHSV(int r, int g, int b, float& h, float& s, float& v);
    void getHSL(int r, int g, int b,  float& h, float& s, float& l);
  private:
    void getALS(uint8_t * buf, int& ir, int& rawlux, int& lux);
    void getColors(uint8_t * buf, int& r, int& g, int& b);
    void dumpReg();
    int getLuxGain(int gain);
    int getADCResolution(int resolution);
    float getLuxIntTime(int resolution);
    int available();
    void enableRGB();
    void enableALS();
    void disableMeas();
    int normAndTrim(int color, int min, int max);
    int adcToValue(int adc);
    int readRegister(uint8_t address);
    int readRegisters(uint8_t address, uint8_t* data, size_t length);
    int writeRegister(uint8_t address, uint8_t value);

  private:
    callback_f _callback = nullptr;
    bool _calibrated = false;
    bool _irq = false;
    unsigned long _timeout = 200;
    int _gain = 1;
    int _rate = 2;
    int _adcResolution = 2;
    TwoWire* _wire;
    uint8_t _slaveAddress;
    int _csPin;
    int _irqPin;
    int _maxR = 255;
    int _maxG = 255;
    int _maxB = 255;
    int _minR = 0;
    int _minG = 0;
    int _minB = 0;
    int _upperThreshold = 1000;
    int _lowerThreshold = 100;
};

extern LTR381RGBClass RGB;
