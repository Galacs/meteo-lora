#pragma once

#include <Arduino.h>
#include <RH_RF95.h>

struct __attribute__((packed)) weather_data_t {
  static constexpr int ID = 10;
  float si7021_temperature;
  float si7021_humidity;
  float dht22_temperature;
  float dht22_humidity;
};
static_assert(sizeof(weather_data_t) <= (255 - 4), "weather_data_t exceeds lora frame size (??)");