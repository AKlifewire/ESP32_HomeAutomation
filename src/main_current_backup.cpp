#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================== CONFIG ==================
const char* WIFI_SSID = "iPhone 11 Ak";
const char* WIFI_PASSWORD = "lifewire";
const char* USER_ID = "Akorede";
#define LED_PIN 2

// Relay pins for 4-channel relay module
#define RELAY1_PIN 4
#define RELAY2_PIN 5
#define RELAY3_PIN 21
#define RELAY4_PIN 22

// AWS IoT
const char AWS_ENDPOINT[] = "a3rbu1pildf8jg-ats.iot.us-east-1.amazonaws.com";
const int AWS_PORT = 8883;

// MQTT Topics
String controlTopic = "hub-346570/digital/control";
String statusTopic = "hub-346570/digital/status";

const char CLAIM_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUQmTsauXe/3FQyNc9EAz5mHoAUFMwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDgwNzIxNTk0
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM17Shh6wrfG/5gZDe5O
tjgj3EgV0CKo5BGU8eD+LpocWW1JLPnkjdlPNzu1S187OAPSeGpA+7oBX/8lkxmD
ZMgKDmgbvI7yOOR1MGZdgnMwqx70YDXlp2xNKVVBficDlj1AIFUO9i62b5OUkmnu
HXpZhyE8DqAngB8CEd/GIBi/2dWu0jKNUK9hQUI+UAwgsk7QhwGzMRsAKat8gF53
VFkOodQasEK9kvKVElZHPholYEB7vQmSG5nBLMQbNoF+atvNN0r9puq3RV1pqefN
ouSby5wCk0/8uU8mbbadqj4bI8IJ5cmEYrGTMgwp3EdbiQ7cHkvZeCFKz6uJVeUv
49MCAwEAAaNgMF4wHwYDVR0jBBgwFoAU7T/7AQVoeapRQjHIEU7JeWuDil8wHQYD
VR0OBBYEFJWK88HJ4WaBxELjFrsgbPS2eeboMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQB3REQ8ATZhdKCeBaNLUyCFXcP3
5/MtpcKPFqW6LnaZylJxc7XfALbxiAM7pwoO376Kpv+I0A/rkM2LT8TFkMh47GXX
I35eWma8rl8MIrpwHiWz874VawJsMMkBLXB5INhMFNL4jVxBbCjTa6L7Cw/PyQRI
GeIQvshiv5ElRWq5uRYDc9OFJNZEClCLf0aUEzrmUWQbKqybLdJ7ugLdwt5yTXVk
3FTVz/CF3vasJ8hAdSJbO7ktsLM1AJurpRpJJNnSjEdp3jFeZyxwAiTiWb1slbw+
JUWccY6qao+vvNrOsi5bkW5LnT6gvhjCgKMqVbDotwHELJLO3uVkMFBQ9cg6
-----END CERTIFICATE-----
)EOF";

const char CLAIM_KEY[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpgIBAAKCAQEAzXtKGHrCt8b/mBkN7k62OCPcSBXQIqjkEZTx4P4umhxZbUks
+eSN2U83O7VLXzs4A9J4akD7ugFf/yWTGYNkyAoOaBu8jvI45HUwZl2CczCrHvRg
NeWnbE0pVUF+JwOWPUAgVQ72LrZvk5SSae4delmHITwOoCeAHwIR38YgGL/Z1a7S
Mo1Qr2FBQj5QDCCyTtCHAbMxGwApq3yAXndUWQ6h1BqwQr2S8pUSVkc+GiVgQHu9
CZIbmcEsxBs2gX5q2803Sv2m6rdFXWmp582i5JvLnAKTT/y5TyZttp2qPhsjwgnl
yYRisZMyDCncR1uJDtweS9l4IUrPq4lV5S/j0wIDAQABAoIBAQDBR1/p6MbxcMiI
e8Cj0PwLkIeqcoIlp/FnLE/cT5rsMRBZAMTChZ57ssyEaEJuYFiPF9FWOKvPSW3P
pLtZ+0K3+uWUPcq+Ns8W7bhKAVksGrJLWpiI+ezirS3c2M7dybzu/jurZnhgdhA7
pDuYUnb91+qIc2JaeO1FvX+smkMkotk7drE3VCP0tT3THX1xyHYQBRs+eAHqdRJd
hBcn8qodYU7Yns/ZLCXeSIwojQE//jtXWDfVuoJ0Mhh6+uXIeoZzO7t05CkGhh4+
qlM4+mOS8CAiSJI9j2CHnYWFQKyLJMYiWo4M8XQMZyRxgNHsZ+lNmm0pavSj8b4/
zzsl2lABAoGBAPDDORB2lnD6QgitglWnLJN9mAh9ZMjp6VB3onk2Gfmwzh/cdI54
4pzhLZOWJI4YBYOWyz6JjxaAy+HMKX/Yaoxqd8FsYPwhSEnye24zJRCpSTaFHOlQ
tBCUc7IfOVJ7bo7dGhIuncz81aJtOOu92dt5GvqtRvOmgNsl1QX+DQgBAoGBANp8
c9XZsV35bkNuSUiQ0wPK/aKk1Y3xagz9Y8tuiiNFYU9Ouww5F671Jqa7vsvxuzeb
ZKRumg6PDsNstQq8mDPiNeq+YjRHh2Od5DrFImEQtBtPi/2AWcF/JpurbsAnkLtR
2Hcg06b0B0yIn3SdyADCbWcgzuHsJYUJrKzfGkvTAoGBAJjWJSx5S2kuxKqDksBS
3m5GhTOmfks4ih7Fw0MVAApMnZ0GMoua26hhHbCZ2FzHjQCKwjzR0B6l5kdRdmqs
9H5su9byuOJ1MlGW95nuJ7Ja9JoCiGboD1aTFprVTWs55eYVH465Pv0451kz4rVs
EJdNWr4VL3xOj3AOpRsnuhgBAoGBAKIWqikAs6EwJylHbH61UpcBxWEgQH5pl2/D
nIGTpE++bQ7VLddHk9ZscRxJNKdA0s/SLLwAgbJDV4SL5VPtzkzPiYh5bYmzdzHk
NSmUjojMR5HrVcPfsLboic/7QtKzh6AwfBME+4BkkfWpdgKh+3r0ww07PcBF3R/x
qXUmnu+ZAoGBAM56EGIHhn2qAy1ifmrdcejjLR55nfKJQwSdNQo2lk4FGpMZGsX3
m1E3sG9WcmKKsFUccmqn/8EtTitxXgRbdMsJdrJxomUGwOuvoUI5sIjfjYdf82la
FCLq+m/qE9phpFdgutUuepOvXJ2XzqOmZE7eMEryr3MoIzK1H8xVHoGz
-----END RSA PRIVATE KEY-----
)EOF";

const char AWS_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Globals
WiFiClientSecure net;
PubSubClient mqttClient(net);
String thingName = "hub-346570";

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
  Serial.println("\n🚀 ESP32 Smart Home Hub Starting...");
  
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
  Serial.println("🔌 Relay pins initialized");
  
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
  
  delay(100);
}