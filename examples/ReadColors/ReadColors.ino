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
