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

#include <Arduino.h>
#include <Wire.h>

class LTR381RGBClass {
  public:
    LTR381RGBClass(TwoWire& wire, uint8_t slaveAddress);
    ~LTR381RGBClass();

    int begin();
    void end();

    int readColors(int& r, int& g, int& b);

    int readAmbientLight(int& als);
    int readLux(int& lux);

    void setGain(int gain);
    void setADCResolution(int resolution);
    void setMeasurementRate(int rate);
    void setUpperThreshold(int utv);
    void setLowerThreshold(int ltv);
    void setTimeout(unsigned long timeout = 200);
    void enableALSInterrupt();
    void disableALSInterrupt();
    void resetSW();

  private:
    int getLuxGain(int gain);
    float getLuxResolution(int resolution);
    int available();
    void enableRGB();
    void enableALS();
   
    int readRegister(uint8_t address);
    int readRegisters(uint8_t address, uint8_t* data, size_t length);
    int writeRegister(uint8_t address, uint8_t value);

  private:
    unsigned long _timeout = 200;
    int _gain = 1;
    float _adcResolution = 1.0f;
    TwoWire* _wire;
    uint8_t _slaveAddress;
    int _csPin;
    int _irqPin;
};

extern LTR381RGBClass RGB;
