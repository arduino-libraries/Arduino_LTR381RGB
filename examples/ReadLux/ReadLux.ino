/*
  Arduino LTR381RGB - Read Lux

  This example reads lux values from the LTR381RGB
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
  int lux;

  if (RGB.readLux(lux)) {
    Serial.print("Lumen: ");
    Serial.println(lux);
  }

  delay(1000);
}
