#pragma once

#include <Arduino.h>
#include "RadioSnapshot.h"
#include "../tuner/TunerDriver.h"
#include "../settings/SettingsStore.h"

// Owns the radio's public operating state. UI/services must use this API;
// they must not access the tuner library directly.
class RadioCore {
public:
  bool begin(uint32_t initialFrequencyKHz = 103900);
  void update();
  bool tune(uint32_t frequencyKHz);
  bool nextChannel(int8_t direction);
  bool isReady() const { return ready_; }
  const RadioSnapshot& snapshot() const { return snapshot_; }

private:
  static constexpr uint32_t kMinFrequencyKHz = 87500;
  static constexpr uint32_t kMaxFrequencyKHz = 108000;
  static constexpr uint32_t kChannelStepKHz = 100;

  TunerDriver tuner_;
  SettingsStore settings_;
  RadioSnapshot snapshot_{};
  bool ready_ = false;
  uint32_t lastPollMs_ = 0;
  uint32_t generation_ = 0;
  uint32_t lastTuneChangeMs_ = 0;
  bool frequencySavePending_ = false;
};
