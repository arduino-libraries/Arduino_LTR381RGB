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
  int als;

  if (RGB.readAmbientLight(als)) {
    Serial.print("ALS: ");
    Serial.println(als);
  }

  delay(1000);
}
