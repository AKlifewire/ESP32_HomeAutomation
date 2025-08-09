#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

WiFiClientSecure net;
PubSubClient mqttClient(net);

void setup() {
  Serial.begin(115200);
  Serial.println("SmartHomeHub v" + String(FIRMWARE_VERSION) + " starting...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  
  net.setCACert(AWS_CA);
  net.setCertificate(CLAIM_CERT);
  net.setPrivateKey(CLAIM_KEY);
  
  mqttClient.setServer(AWS_ENDPOINT, AWS_PORT);
  
  while (!mqttClient.connected()) {
    if (mqttClient.connect(DEVICE_ID)) {
      Serial.println("MQTT connected!");
      String bootMsg = "{\"status\":\"online\",\"version\":\"" + String(FIRMWARE_VERSION) + "\"}";
      mqttClient.publish(bootTopic.c_str(), bootMsg.c_str());
    } else {
      delay(5000);
    }
  }
}

void loop() {
  mqttClient.loop();
  
  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > 30000) {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["version"] = FIRMWARE_VERSION;
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    
    String payload;
    serializeJson(doc, payload);
    mqttClient.publish(telemetryTopic.c_str(), payload.c_str());
    
    lastTelemetry = millis();
  }
  
  delay(1000);
}