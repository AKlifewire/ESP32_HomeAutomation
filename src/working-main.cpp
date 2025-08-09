#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// ================== PIN DEFINITIONS ==================
#define LED_PIN 2
#define RELAY1_PIN 4
#define RELAY2_PIN 5
#define RELAY3_PIN 21
#define RELAY4_PIN 22

// ================== GLOBALS ==================
WiFiClientSecure net;
PubSubClient mqttClient(net);
String thingName = DEVICE_ID;

// ================== RELAY CONTROL ==================
void controlRelay(int relayNum, bool state) {
  int pin;
  switch(relayNum) {
    case 1: pin = RELAY1_PIN; break;
    case 2: pin = RELAY2_PIN; break;
    case 3: pin = RELAY3_PIN; break;
    case 4: pin = RELAY4_PIN; break;
    default: return;
  }
  
  digitalWrite(pin, state ? LOW : HIGH); // Relay modules are active LOW
  Serial.println("🔌 Relay " + String(relayNum) + " " + (state ? "ON" : "OFF"));
  
  // Flash LED to indicate action
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

// ================== BOOT REPORT ==================
void reportBootStatus() {
  JsonDocument bootDoc;
  bootDoc["device_id"] = DEVICE_ID;
  bootDoc["fw_version"] = FIRMWARE_VERSION;
  bootDoc["build_timestamp"] = BUILD_TIMESTAMP;
  bootDoc["owner_id"] = USER_ID;
  bootDoc["ip_address"] = WiFi.localIP().toString();
  bootDoc["device_type"] = DEVICE_TYPE;
  
  String bootMsg;
  serializeJson(bootDoc, bootMsg);
  
  if (mqttClient.publish(bootTopic.c_str(), bootMsg.c_str())) {
    Serial.println("✅ Boot report sent: v" + String(FIRMWARE_VERSION));
  } else {
    Serial.println("❌ Boot report failed");
  }
}

// ================== TELEMETRY ==================
void sendTelemetry() {
  JsonDocument telemetryDoc;
  telemetryDoc["device_id"] = DEVICE_ID;
  telemetryDoc["timestamp"] = millis();
  telemetryDoc["uptime"] = millis() / 1000;
  telemetryDoc["free_heap"] = ESP.getFreeHeap();
  telemetryDoc["wifi_rssi"] = WiFi.RSSI();
  
  // Add relay states
  JsonDocument relayStates;
  relayStates["relay1"] = digitalRead(RELAY1_PIN) == LOW;
  relayStates["relay2"] = digitalRead(RELAY2_PIN) == LOW;
  relayStates["relay3"] = digitalRead(RELAY3_PIN) == LOW;
  relayStates["relay4"] = digitalRead(RELAY4_PIN) == LOW;
  telemetryDoc["relay_states"] = relayStates;
  
  String telemetryMsg;
  serializeJson(telemetryDoc, telemetryMsg);
  
  mqttClient.publish(telemetryTopic.c_str(), telemetryMsg.c_str());
}

// ================== MQTT CALLBACK ==================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n🚨 MQTT CALLBACK TRIGGERED! 🚨");
  Serial.println("📨 Received MQTT message on topic: " + String(topic));
  Serial.println("📏 Message length: " + String(length));
  
  // Convert payload to string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("📄 Message: " + message);
  
  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("❌ JSON parsing failed: " + String(error.c_str()));
    return;
  }
  
  // Extract command data
  if (doc["relay"].is<int>() && doc["state"].is<bool>()) {
    int relayNum = doc["relay"];
    bool state = doc["state"];
    
    Serial.println("🎯 Command: Relay " + String(relayNum) + " -> " + (state ? "ON" : "OFF"));
    controlRelay(relayNum, state);
    
    // Send status update
    JsonDocument statusDoc;
    statusDoc["relay"] = relayNum;
    statusDoc["state"] = state;
    statusDoc["timestamp"] = millis();
    
    String statusMsg;
    serializeJson(statusDoc, statusMsg);
    mqttClient.publish(statusTopic.c_str(), statusMsg.c_str());
    Serial.println("📤 Status sent: " + statusMsg);
  }
}

// ================== WIFI CONNECTION ==================
void connectToWiFi() {
  Serial.println("🌐 Connecting to WiFi: " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.println("📍 IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
  }
}

// ================== MQTT CONNECTION ==================
void connectToMQTT() {
  Serial.println("🔗 Connecting to AWS IoT...");
  
  // Set certificates
  net.setCACert(AWS_CA);
  net.setCertificate(CLAIM_CERT);
  net.setPrivateKey(CLAIM_KEY);
  
  mqttClient.setServer(AWS_ENDPOINT, AWS_PORT);
  mqttClient.setCallback(mqttCallback);
  
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5) {
    Serial.println("🔄 MQTT connection attempt " + String(attempts + 1));
    
    if (mqttClient.connect(thingName.c_str())) {
      Serial.println("✅ MQTT connected!");
      
      // Subscribe to control topic
      if (mqttClient.subscribe(controlTopic.c_str())) {
        Serial.println("📡 Subscribed to: " + controlTopic);
      } else {
        Serial.println("❌ Failed to subscribe to: " + controlTopic);
      }
      
      // Send online status
      JsonDocument onlineDoc;
      onlineDoc["status"] = "online";
      onlineDoc["device"] = thingName;
      onlineDoc["timestamp"] = millis();
      
      String onlineMsg;
      serializeJson(onlineDoc, onlineMsg);
      mqttClient.publish(statusTopic.c_str(), onlineMsg.c_str());
      Serial.println("📤 Online status sent");
      
      // Send boot report
      reportBootStatus();
      
    } else {
      Serial.println("❌ MQTT connection failed, rc=" + String(mqttClient.state()));
      delay(2000);
    }
    attempts++;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 " + String(DEVICE_TYPE) + " v" + String(FIRMWARE_VERSION));
  Serial.println("📋 Device ID: " + String(DEVICE_ID));
  Serial.println("👤 Owner: " + String(USER_ID));
  Serial.println("🔨 Built: " + String(BUILD_TIMESTAMP));
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  // Initialize relays to OFF (HIGH for active-low relays)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
  
  Serial.println("🔌 Relay pins: GPIO4, GPIO5, GPIO21, GPIO22");
  Serial.println("🔌 Relay pins initialized (ACTIVE LOW)");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Connect to MQTT
  if (WiFi.status() == WL_CONNECTED) {
    connectToMQTT();
  }
  
  Serial.println("🎉 Setup complete! Ready for commands.");
  Serial.println("📡 Listening on topic: " + controlTopic);
}

// ================== MAIN LOOP ==================
void loop() {
  static unsigned long lastTelemetry = 0;
  
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    Serial.println("⚠️ MQTT disconnected, reconnecting...");
    connectToMQTT();
  }
  
  mqttClient.loop();
  
  // Keep WiFi alive
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi disconnected, reconnecting...");
    connectToWiFi();
  }
  
  // Send telemetry every 60 seconds
  if (millis() - lastTelemetry > 60000) {
    if (mqttClient.connected()) {
      sendTelemetry();
      lastTelemetry = millis();
    }
  }
  
  delay(100);
}