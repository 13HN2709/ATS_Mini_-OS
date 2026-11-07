#include "ThemeManager.h"

namespace {
constexpr ThemePalette kThemes[] = {
  {
    "Classic",
    0x0000, // background
    0xFFFF, // text
    0x7BEF, // muted
    0x07E0, // accent green
    0x07FF, // secondary cyan
    0xFFE0, // warning
    0x07E0, // frequency
    0x07E0, // frequency unit
    0xFFFF, // RDS text
    0x0320, // divider
    0x0000, // box background
    0x07E0  // box border
  },
  {
    "TEF Blue",
    0x0000,
    0xFFFF,
    0x7BEF,
    0x07E0,
    0x1D7F,
    0xFFE0,
    0x07E0,
    0x07E0,
    0xFFFF,
    0x1D7F,
    0x000B,
    0x1D7F
  },
  {
    "Amber",
    0x0000,
    0xFFFF,
    0xA514,
    0xFD20,
    0xFBE0,
    0xFFE0,
    0xFD20,
    0xFD20,
    0xFFFF,
    0x8200,
    0x0000,
    0xFD20
  }
};
}

uint8_t ThemeManager::normalize(uint8_t index) {
  return index < count() ? index : 0;
}

const ThemePalette& ThemeManager::at(uint8_t index) {
  return kThemes[normalize(index)];
}
