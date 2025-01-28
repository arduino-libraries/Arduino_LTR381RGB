/* 
  Arduino LTR381RGB - Measure Calibration Parameters

  This example shows have to be used to measure the calibration parameters.
  The calibration parameters are the maximum and minimum values for each 
  color channel.

  To calculate the calibration parameters follow the steps below:
  - Turn Off all the lights and let the sketch measure the minimum R, G, B 
    light intensity.
  - Turn On the light RGB channel one at a time and with the light channels
    set to the maximum brightness let the sketch measure the maximum R, G
    and B values.
  - Take note of the maximum and minimum values and use them in the 
    ReadCalibratedColors example.

  This example code is in the public domain.
*/

#include "Arduino_LTR381RGB.h"

int maxR = 0x000000;
int maxG = 0x000000;
int maxB = 0x000000;

int minR = 0x0FFFFFF;
int minG = 0x0FFFFFF;
int minB = 0x0FFFFFF;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!RGB.begin()) {
    Serial.println("Failed to initialize rgb sensor!");
    while (1);
  }
}

void loop() {
  int r, g, b;

  if (RGB.readColors(r, g, b)) {
    if (r > maxR) {
        maxR = r;
    }
    if (r < minR) {
        minR = r;
    }
    if (g > maxG) {
        maxG = g;
    }
    if (g < minG) {
        minG = g;
    }
    if (b > maxB) {
        maxB = b;
    }
    if (b < minB) {
        minB = b;
    }
    Serial.print("Max Red: ");
    Serial.print(maxR);
    Serial.print("\tMin Red: ");
    Serial.print(minR);
    Serial.print("\tMax Green: ");
    Serial.print(maxG);
    Serial.print("\tMin Green: ");
    Serial.print(minG);
    Serial.print("\tMax Blue: ");
    Serial.print(maxB);
    Serial.print("\tMin Blue: ");
    Serial.println(minB);
  }

}
