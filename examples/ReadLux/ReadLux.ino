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
