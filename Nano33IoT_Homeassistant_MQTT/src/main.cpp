#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <avr/dtostrf.h>

const char* ssid     = "Your_SSID";
const char* password = "Your_PASSWORD";
IPAddress broker(192,168,1,21);

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const char* deviceId          = "nano33_device";
const char* relayDiscovery    = "homeassistant/switch/nano33_relay/config";
const char* relayStateTopic   = "nano33/relay/state";
const char* relayCommandTopic = "nano33/relay/command";
const char* sensorDiscovery   = "homeassistant/sensor/nano33_analog/config";
const char* sensorStateTopic  = "nano33/analog/state";
const char* availabilityTopic = "nano33/availability";

const int relayPin  = 7;
const int analogPin = A0;

// Declare buffers here
char bufRelay[256];
char bufSensor[256];

void sendRelayDiscovery() {
  StaticJsonDocument<256> doc;
  doc["name"]               = "Nano33 Relay";
  doc["unique_id"]          = "nano33_relay";
  doc["state_topic"]        = relayStateTopic;
  doc["command_topic"]      = relayCommandTopic;
  doc["availability_topic"] = availabilityTopic;
  doc["payload_on"]         = "ON";
  doc["payload_off"]        = "OFF";
  doc["qos"]                = 0;
  doc["retain"]             = true;

  size_t n = serializeJson(doc, bufRelay, sizeof(bufRelay)); // Serialize to buffer :contentReference[oaicite:1]{index=1}
  mqtt.publish(relayDiscovery, (const uint8_t*)bufRelay, n, true);
}

void sendSensorDiscovery() {
  StaticJsonDocument<256> doc;
  doc["name"]               = "Nano33 Analog Voltage";
  doc["unique_id"]          = "nano33_analog";
  doc["state_topic"]        = sensorStateTopic;
  doc["unit_of_measurement"]= "V";
  doc["device_class"]       = "voltage";
  doc["availability_topic"] = availabilityTopic;
  doc["qos"]                = 0;
  doc["retain"]             = true;

  size_t n = serializeJson(doc, bufSensor, sizeof(bufSensor));
  mqtt.publish(sensorDiscovery, (const uint8_t*)bufSensor, n, true);
}

void sendDiscovery() {
  sendRelayDiscovery();
  sendSensorDiscovery();
}

void connectMQTT() {
  mqtt.setBufferSize(512);

  while (!mqtt.connected()) {
    Serial.print("MQTT connecting...");
    if (mqtt.connect(deviceId,
                     availabilityTopic, 0, true, "offline")) {
      Serial.println("connected");
      mqtt.publish(availabilityTopic, (const uint8_t*)"online", strlen("online"), true);
      sendDiscovery();
      mqtt.subscribe(relayCommandTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retry in 5s");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned len) {
  String msg;
  for (uint i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();

  if (String(topic) == relayCommandTopic) {
    if (msg == "ON") {
      digitalWrite(relayPin, HIGH);
      mqtt.publish(relayStateTopic, "ON", true);
    } else {
      digitalWrite(relayPin, LOW);
      mqtt.publish(relayStateTopic, "OFF", true);
    }
  }
}

unsigned long lastAnalog = 0;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi‑Fi connected: " + WiFi.localIP().toString());

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastAnalog > 5000) {
    lastAnalog = millis();
    float volts = analogRead(analogPin) * (5.0f / 1023.0f);
    char buf[16];
    dtostrf(volts, 0, 2, buf);
    mqtt.publish(sensorStateTopic, buf, true);
  }
}
