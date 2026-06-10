#include <Arduino.h>
#include "messages.h"


void setup() {
  // Print a message to the LCD.
  Serial.begin(115200);
  Serial.println("Start Sketch");
}

void loop() {
  weather_data_t msg;
  uint8_t len = sizeof(msg);
  delay(10);
}
