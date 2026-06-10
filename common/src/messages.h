#pragma once

#include <Arduino.h>

struct __attribute__((packed)) weather_data_t {
  static constexpr int ID = 10;
  float si7021_temperature;
  float si7021_humidity;
  float mpl_pressure;
  float mpl_temperature;
  float wind_direction;
  float wind_speed;
  float rain_fall;
};
static_assert(sizeof(weather_data_t) <= (255 - 4), "weather_data_t exceeds lora frame size (??)");