#pragma once

#include <Arduino.h>

struct __attribute__((packed)) weather_data_t {
  static constexpr int ID = 10;
  float wind_direction;
  float wind_speed;
  float rain_fall;
  float MS5607_temperature;
  float MS5607_pressure;
  float HTU21D_temperature;
  float HTU21D_humidity;
  float HTU21D_compensed_humidity;
};
static_assert(sizeof(weather_data_t) <= (255 - 4), "weather_data_t exceeds lora frame size (??)");