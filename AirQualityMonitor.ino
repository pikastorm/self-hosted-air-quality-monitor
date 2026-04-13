#include "secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "PMS.h"

// Hardware Pins
#define PMS_RX 18
#define PMS_TX 19
#define MQ135_PIN 34

#define THINGNAME "AirSensor"
// Local MQTT Settings
#define MQTT_TOPIC "sensor/air_quality"
const char* mqtt_server = "192.168.6.163"; //  Mosquitto IP

// Initialize Sensors & Network
PMS pms(Serial2);
PMS::DATA data;

WiFiClient espClient;          // Standard non-secure client
PubSubClient client(espClient);

void connectMQTT() {
  // 1. Ensure Wi-Fi is still alive
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWi-Fi Restored.");
  }

  // 2. Connect to local Mosquitto
  while (!client.connected()) {
    Serial.print("Attempting local MQTT connection... ");
    
    // We use THINGNAME as the unique ClientID
    if (client.connect(THINGNAME)) {
      Serial.println("CONNECTED to local broker.");
    } else {
      Serial.print("FAILED, rc=");
      Serial.print(client.state());
      Serial.println(" - Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void publishMessage(int pm25, int pm10, int pm1, int gasRaw) {
  StaticJsonDocument<256> doc;
  doc["device_id"] = THINGNAME;
  doc["pm25"] = pm25;
  doc["pm10"] = pm10;
  doc["pm1"] = pm1;
  doc["gas_raw"] = gasRaw;
  doc["timestamp"] = millis();

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.print("Publishing to Local Broker: ");
  Serial.println(jsonBuffer);

  if (client.publish(MQTT_TOPIC, jsonBuffer)) {
    Serial.println("-> Publish Successful!");
  } else {
    Serial.println("-> Publish FAILED!");
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);

  Serial.println("\n--- Starting Local Vancouver AQI Station ---");

  // 1. Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");

  // 2. Setup MQTT
  client.setServer(mqtt_server, 1883);
  
  // 3. Initial connection
  connectMQTT();
}

void loop() {
  // Ensure we stay connected to broker
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Read Sensors and Publish
  if (pms.read(data)) {
    int mq135Raw = analogRead(MQ135_PIN);
    
    Serial.println("\n--- New Sensor Reading ---");
    Serial.print("PM 2.5: "); 
    Serial.print(data.PM_AE_UG_2_5);
    Serial.println(" ug/m3");
    
    publishMessage(data.PM_AE_UG_2_5, data.PM_AE_UG_10_0, data.PM_AE_UG_1_0, mq135Raw);
    
    // Local network is fast, 5s delay is plenty
    delay(5000); 
  }
}