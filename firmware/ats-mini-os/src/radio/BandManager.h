#pragma once
#include <Arduino.h>

// Band data is kept independent from the tuner and display layers.
// Frequencies are expressed in kHz, matching RadioSnapshot.
enum class RadioBandType : uint8_t { FM, LW, MW, SW, ALL };
enum class RadioModulation : uint8_t { FM, AM, LSB, USB };

struct RadioBandDefinition {
  const char* name;
  RadioBandType type;
  RadioModulation modulation;
  uint32_t minimumKHz;
  uint32_t maximumKHz;
  uint32_t startKHz;
  uint16_t stepKHz;
  uint8_t bandwidthIndex;
};

class BandManager {
public:
  static constexpr uint8_t count();
  static const RadioBandDefinition& at(uint8_t index);
  static uint8_t currentIndex();
  static const RadioBandDefinition& current();
  static bool select(uint8_t index);
  static bool next(int8_t direction);
  static bool contains(uint8_t index, uint32_t frequencyKHz);
};
