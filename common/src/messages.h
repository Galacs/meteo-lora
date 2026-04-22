#pragma once

#include <Arduino.h>
#include <RH_RF95.h>

struct __attribute__((packed)) weather_data_t {
  static constexpr int ID = 10;
  float si7021_temperature;
  float si7021_humidity;
  float mpl_pressure;
  float mpl_temperature;
  int wind_direction;
  int wind_speed;
  int rain_fall;
};
static_assert(sizeof(weather_data_t) <= (255 - 4), "weather_data_t exceeds lora frame size (??)");