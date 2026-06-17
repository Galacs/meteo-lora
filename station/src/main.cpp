#include <Arduino.h>
#include "messages.h"
#include <SPI.h>
#include <SparkFun_Weather_Meter_Kit_Arduino_Library.h>
#include <MS5611.h>
#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <SPI.h>
#include <HTU21D.h>
#include <RadioLib.h>
#include <sys/unistd.h>
#include <errno.h>

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
Adafruit_SHT31 sht31 = Adafruit_SHT31();

bool enableHeater = false;
uint8_t loopCnt = 0;

SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);
// Singleton instance of the radio driver

void setup() {
  SPI.setMISO(LORA_MISO_PIN);
  SPI.setMOSI(LORA_MOSI);
  SPI.setSCLK(LORA_SCK_PIN);
  int state = radio.begin(868.1);
  if (state == RADIOLIB_ERR_NONE) {
    printf("lora success!\n");
  } else {
    printf("failed, code: %d \n", state);

    while (true) { delay(10); }
  }

  weatherMeterKit.begin();

  Wire.setSCL(SCL_PIN);
  Wire.setSDA(SDA_PIN);
  Wire.begin();
  if (MS5607.begin() == true)
  {
    printf("MS5607 found: ");
    printf("%d\n", MS5607.getAddress());
  }
  else
  {
    printf("MS5607 not found. halt.\n");
    while (1);
  }

   while (myHTU21D.begin() != true)
  {
    printf("HTU21D, SHT21 sensor is faild or not connected\n"); //(F()) saves string to flash & keeps dynamic memory free
    delay(5000);
  }

    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      printf("Couldn't find SHT31\n");
    while (1) delay(1);
  }

  printf("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    printf("ENABLED\n");
  else
    printf("DISABLED\n");

  printf("HTU21D, SHT21 sensor is active \n");
  pinMode(STATUS_PIN, OUTPUT);

}


extern "C" int _write(int file, char *data, int len) {
  if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
    errno = EBADF;
    return -1;
  }

  for (int i = 0; i < len; i++) {
    ITM_SendChar(data[i]);
  }
  return len;
  }
bool a = false;
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
    msg.GXHT30_temperature = sht31.readTemperature();
    msg.GXHT30_humidity = sht31.readHumidity();

    char buf[10];
    dtostrf(msg.MS5607_temperature, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.wind_speed, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.HTU21D_humidity, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.HTU21D_temperature, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.GXHT30_temperature, 3, 2, buf);
    printf("a: %s\n", buf);

    radio.transmit((uint8_t *)&msg, sizeof(msg));
  
    printf("sent \n");
    delay(500);
    digitalWrite(STATUS_PIN, a);
    a != a;
    delay(500);
  }
}
