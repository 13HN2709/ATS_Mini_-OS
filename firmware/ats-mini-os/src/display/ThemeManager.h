#pragma once
#include <Arduino.h>

struct ThemePalette {
  const char* name;
  uint16_t background;
  uint16_t text;
  uint16_t muted;
  uint16_t accent;
  uint16_t secondary;
  uint16_t warning;
  uint16_t frequency;
  uint16_t frequencyUnit;
  uint16_t radioText;
  uint16_t divider;
  uint16_t boxBackground;
  uint16_t boxBorder;
};

class ThemeManager {
public:
  static constexpr uint8_t count() { return 3; }
  static const ThemePalette& at(uint8_t index);
  static uint8_t normalize(uint8_t index);
};
