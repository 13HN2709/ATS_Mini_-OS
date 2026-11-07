#pragma once
#include <Arduino.h>
class RadioCore;
class MenuSSB {
public:
  static constexpr uint8_t kPage = 12;
  static constexpr uint8_t kItemCount = 8;
  static const char* itemLabel(uint8_t index);
  static bool apply(RadioCore& radio, uint8_t index);
};
