#include <Arduino.h>
#include <PubSubClient.h> // MQTT
#include <WiFiClientSecure.h>
#include "esp_system.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <RadioLib.h>
#include "messages.h"

#define LORA_RST_PIN 05
#define LORA_DIO1_PIN 19
#define LORA_BUSY_PIN 21
#define LORA_DIO3_PIN 26
#define LORA_CS_PIN 25
#define LORA_SCK_PIN 18
#define LORA_MISO_PIN 17
#define LORA_MOSI_PIN 23

#define SCL_PIN 6
#define SDA_PIN 7

#define RGB_PIN 23

#define MQTT_PORT 1883

// CA Let's encrypt pour MQTT TLS. cf: https://letsencrypt.org/certificates/
const char* root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "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\n" \
  "-----END CERTIFICATE-----";



void callback(char* topic, byte* payload, unsigned int length) { }

WiFiClientSecure wifiClient;
PubSubClient client(ENV_MQTT_SERVER, MQTT_PORT, callback, wifiClient);

SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "UPHF1";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), ENV_MQTT_USERNAME, ENV_MQTT_PASSWORD)) {
      // digitalWrite(13, HIGH);
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  // Print a message to the LCD.
  Serial.begin(115200);
  Serial.println("Start Sketch");

  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);

  // initialize SX1262 at 868.1 MHz
  Serial.print(F("[SX1262] Initializing ... "));
  ConfigLoRa_t config;
  config.frequency = 868.1;
  config.codingRate = 5;
  config.spreadingFactor = 8;
  config.power = 22;
  int state = radio.begin(config);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }


  Serial.print(F("[WIFI] Initializing ... "));
  WiFi.setBandMode(WIFI_BAND_MODE_2G_ONLY);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(ENV_WIFI_SSID, ENV_WIFI_PASSWORD);
  // Attendre que le wifi se connecte.
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
    // digitalWrite(12, HIGH);
    // delay(150);
    // digitalWrite(12, LOW);
    // delay(50);
  }
  // digitalWrite(12, HIGH);
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  configTime(1 * 60 * 60, 0, "pool.ntp.org", "0.fr.pool.ntp.org", "2.fr.pool.ntp.org"); // Configuration de l'horloge pour TLS
  Serial.print("Waiting for NTP time sync...");
  time_t now = 0;
  while (now < 1000000000) {
    delay(500);
    Serial.print(".");
    time(&now);
  }
  Serial.println("\nTime synced");
  // wifiClient.setCACert(root_ca); // CA
  wifiClient.setInsecure(); // Pour desactiver la verification de CA
  reconnect();
  client.setBufferSize(512);
}

void loop() {
  weather_data_t msg;
  uint8_t len = sizeof(msg);
  delay(10);

  DynamicJsonDocument doc(1024);
  Serial.print(F("[SX1262] Waiting for incoming transmission ... "));
  int state = radio.receive((uint8_t*) &msg, sizeof(weather_data_t), 0);

  Serial.printf("temp???: ");
  Serial.println(msg.GXHT30_temperature);
  doc["wind_direction_vane"] = msg.wind_direction;
  doc["wind_speed_anenometer"] = msg.wind_speed;
  doc["rain_bucket"] = msg.rain_fall;
  doc["temperature_ms5607"] = msg.MS5607_temperature;
  doc["pressure_ms5607"] = msg.MS5607_pressure;
  doc["temperature_htu21d"] = msg.HTU21D_temperature;
  doc["humidity_htu21d"] = msg.HTU21D_humidity;
  doc["humidity_htu21d_comp"] = msg.HTU21D_compensed_humidity;
  doc["temperature_gxht30"] = msg.GXHT30_temperature;
  doc["humidity_gxht30"] = msg.GXHT30_humidity;

  char buffer[512];
  size_t n = serializeJson(doc, buffer);
  Serial.println(buffer);
  if (!client.connected()) {
    reconnect();
  }
  client.publish("weather/uphf1", buffer, n);
}
