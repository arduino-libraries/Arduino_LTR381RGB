/* 
  Arduino LTR381RGB - Read Calibrated colors
 
  This example show how to set calibration values and reads
  the RGB channels values from the LTR381RGB sensor and continuously 
  prints them to the Serial Monitor or Serial Plotter.

  The calibration parameters have to be calculated using the 
  MeasureCalibrationParameters example.
  
  This example code is in the public domain.
 */

#include "Arduino_LTR381RGB.h"

// Modify these values with the calibration parameters
#define MAX_RED  255
#define MAX_GREEN  255
#define MAX_BLUE  255
#define MIN_RED  0
#define MIN_GREEN  0
#define MIN_BLUE  0

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!RGB.begin()) {
    Serial.println("Failed to initialize rgb sensor!");
    while (1);
  }
  RGB.setCalibrations(MAX_RED, MAX_GREEN, MAX_BLUE, MIN_RED, MIN_GREEN, MIN_BLUE);
}

void loop() {
  int r, g, b;

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
