#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Firmware v1.6.4");
}

void loop() {
  Serial.println("Running...");
  delay(5000);
}