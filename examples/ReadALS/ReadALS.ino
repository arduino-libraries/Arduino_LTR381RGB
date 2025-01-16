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
  int rawlux;

  if (RGB.readAmbientLight(rawlux)) {
    Serial.print("Raw Lux: ");
    Serial.println(rawlux);
  }

  delay(1000);
}
