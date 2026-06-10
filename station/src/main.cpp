#include <Arduino.h>
#include "messages.h"
#include <SPI.h>
#include <SparkFun_Weather_Meter_Kit_Arduino_Library.h>
#include "MS5611.h"
#include <Wire.h>
#include <HTU21D.h>
#include <RadioLib.h>


#define LORA_RST_PIN PB0
#define LORA_DIO1_PIN PB1
#define LORA_BUSY_PIN PB11
#define LORA_DIO3_PIN PB10
#define LORA_CS_PIN PA4
#define LORA_SCK_PIN PA5
#define LORA_MISO_PIN PA6
#define LORA_MOSI PA7


SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);

#define BTN_PIN PB12
#define STATUS_PIN PB13

#define SCL_PIN PB6
#define SDA_PIN PB7
#define CE_PIN PA11
#define INT_PIN PA12
#define QON_PIN PA10

#define RGB_PIN PC13

#define rainfallPin PB4
#define windDirectionPin PA1
#define windSpeedPin PA0

/*
HTU21D(resolution)

resolution:
HTU21D_RES_RH12_TEMP14 - RH: 12Bit, Temperature: 14Bit, by default
HTU21D_RES_RH8_TEMP12  - RH: 8Bit,  Temperature: 12Bit
HTU21D_RES_RH10_TEMP13 - RH: 10Bit, Temperature: 13Bit
HTU21D_RES_RH11_TEMP11 - RH: 11Bit, Temperature: 11Bit
*/
HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);
MS5611 MS5607(0x77);

bool enableHeater = false;
uint8_t loopCnt = 0;

SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);
// Singleton instance of the radio driver

  




void setup() {
  ConfigLoRa_t config;
  config.frequency = 868.1;
  int state = radio.begin(config);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
  weatherMeterKit.begin();
  Serial.begin(115200);
  Serial.println("Start Sketch");
  

  

   Wire.begin();
  if (MS5607.begin() == true)
  {
    Serial.print("MS5607 found: ");
    Serial.println(MS5607.getAddress());
  }
  else
  {
    Serial.println("MS5607 not found. halt.");
    while (1);
  }
  Serial.println();

   while (myHTU21D.begin() != true)
  {
    Serial.println(F("HTU21D, SHT21 sensor is faild or not connected")); //(F()) saves string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("HTU21D, SHT21 sensor is active"));

}

void loop() {
  if (true) {
    MS5607.read();
    weather_data_t msg;
    msg.wind_direction = weatherMeterKit.getWindDirection();
    msg.wind_speed = weatherMeterKit.getWindSpeed();
    msg.rain_fall = weatherMeterKit.getTotalRainfall();
    msg.MS5607_temperature = MS5607.getTemperature();
    msg.MS5607_pressure = MS5607.getPressure();
    msg.HTU21D_temperature = myHTU21D.readTemperature();
    msg.HTU21D_humidity = myHTU21D.readHumidity();
    msg.HTU21D_compensed_humidity = myHTU21D.readCompensatedHumidity();


    radio.transmit((uint8_t *)&msg, sizeof(msg));
  
    Serial.println(("sent"));
    delay(1000);
  }
}
