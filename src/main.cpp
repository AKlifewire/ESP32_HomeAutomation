#include <WiFi.h>
#include "secrets.h"

void setup() {
  Serial.begin(115200);
  Serial.println("SmartHomeHub v" + String(FIRMWARE_VERSION) + " starting...");
  Serial.println("Device ID: " + String(DEVICE_ID));
  Serial.println("Build: " + String(BUILD_TIMESTAMP));
}

void loop() {
  Serial.println("Running firmware v" + String(FIRMWARE_VERSION));
  delay(10000);
}