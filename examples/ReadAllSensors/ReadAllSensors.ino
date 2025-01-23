/*
  Arduino LTR381RGB - Read All Sensors

  This example reads the RGB, raw lux and lux values from the LTR381RGB
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.

  The circuit:
  - Arduino Uno WiFi R4
  - Modulino light sensor

  This example code is in the public domain.
*/

#include "Arduino_LTR381RGB.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!RGB.begin()) {
    Serial.println("Failed to initialize rgb sensor!");
    while (1);
  }
}

void loop() {
  int r, g, b, rawlux, lux, ir;

  if (RGB.readIR(ir)) {
    Serial.print("IR: ");
    Serial.print(ir);
    Serial.println();
  }

  if (RGB.readAmbientLight(lux)) {
    Serial.print("Raw Lux: ");
    Serial.print(rawlux);
    Serial.println();
  }

  if (RGB.readLux(lux)) {
    Serial.print("Lumen: ");
    Serial.print(lux);
    Serial.println();
  }

  if (RGB.readColors(r, g, b)) {
    Serial.print("Red: ");
    Serial.print(r);
    Serial.print("\tGreen: ");
    Serial.print(g);
    Serial.print("\tBlue: ");
    Serial.println(b);
  }
  delay(1000);
}
