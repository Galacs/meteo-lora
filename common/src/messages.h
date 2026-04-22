#pragma once

#include <Arduino.h>
#include <RH_RF95.h>

#define LORA_STRUCT(struct_name, lora_id, ...)                                 \
  struct __attribute__((packed)) struct_name {                                 \
    static constexpr int ID = lora_id;                                         \
    __VA_ARGS__                                                                \
  };                                                                           \
  static_assert(sizeof(struct_name) <= RH_RF95_MAX_MESSAGE_LEN,                \
                #struct_name " exceeds lora frame size (??)");                 \
  // static_assert(std::is_trivially_copyable<struct_name>::value,                \
  //               #struct_name " must be trivially copyable")

LORA_STRUCT(weather_data_t, 10, float temperature; float pressure;);
