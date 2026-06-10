#include <Arduino.h>
#include <RH_RF95.h>

#include "messages.h"

#include <Adafruit_Si7021.h>
#include <SPI.h>
#include <SparkFunMPL3115A2.h>
#include <SparkFun_Weather_Meter_Kit_Arduino_Library.h>

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_Si7021 sensor = Adafruit_Si7021();
MPL3115A2 myPressure;
int windDirectionPin = A0;
int windSpeedPin = 3;
int rainfallPin = 2;
SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);
// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 868.1;
void setup() {
  weatherMeterKit.begin();
  Serial.begin(115200);
  Serial.println("Start Sketch");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(18);

  // Setup Spreading Factor
  rf95.setSpreadingFactor(8);

  // Setup BandWidth, option
  rf95.setSignalBandwidth(125000);

  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)
  rf95.setCodingRate4(5);

  Serial.print("Listening on frequency: ");
  Serial.println(frequency);

  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true);
  }

  myPressure.begin(); // Get sensor online
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags
}

void loop() {
  if (true) {
    weather_data_t msg;
    msg.si7021_humidity = sensor.readHumidity();
    msg.si7021_temperature = sensor.readTemperature();
    msg.mpl_pressure = myPressure.readPressure();
    msg.mpl_temperature = myPressure.readTemp();

    msg.wind_direction = weatherMeterKit.getWindDirection();
    msg.wind_speed = weatherMeterKit.getWindSpeed();
    msg.rain_fall = weatherMeterKit.getTotalRainfall();

    rf95.send((uint8_t *)&msg, sizeof(msg));
    rf95.waitPacketSent();
    Serial.println(("sent"));
    delay(1000);

    if (++loopCnt == 30) {
      enableHeater = !enableHeater;
      sensor.heater(enableHeater);
      Serial.print("Heater Enabled State: ");
      if (sensor.isHeaterEnabled())
        Serial.println("ENABLED");
      else
        Serial.println("DISABLED");

      loopCnt = 0;
    }
    Serial.println("Waiting...");
    delay(100);
  }
}
