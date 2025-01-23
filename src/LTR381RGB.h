/*
  Copyright (c) 2025 Arduino SA
  This Source Code Form is subject to the terms of the Mozilla
  Public License (MPL), v 2.0. You can obtain a copy of the MPL
  at http://mozilla.org/MPL/2.0/
*/

#include <Arduino.h>
#include <Wire.h>

class LTR381RGBClass {
  public:
    LTR381RGBClass(TwoWire& wire, uint8_t slaveAddress);
    ~LTR381RGBClass();

    int begin();
    void end();

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
    void setInteruptPersistence(int persistence);
    void enableALSInterrupt();
    void disableALSInterrupt();
    void resetSW();
    void setCalibrations(int rmax, int gmax, int bmax, int rmin, int gmin, int bmin);

  private:
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
    bool _calibrated = false;
    unsigned long _timeout = 200;
    int _gain = 1;
    int _rate = 2;
    int _adcResolution = 2;
    TwoWire* _wire;
    uint8_t _slaveAddress;
    int _csPin;
    int _irqPin;
    int _maxR;
    int _maxG;
    int _maxB;
    int _minR;
    int _minG;
    int _minB;
    int _upperThreshold = 1000;
    int _lowerThreshold = 100;
};

extern LTR381RGBClass RGB;
