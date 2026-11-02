#pragma once
#include <Arduino.h>
#include "RadioSnapshot.h"
#include "../tuner/TunerDriver.h"

class RadioCore {
public:
  bool begin(uint32_t initialFrequencyKHz = 95400);
  void update();
  bool tune(uint32_t frequencyKHz);
  bool nextChannel(int16_t acceleratedDelta);
  bool isReady() const { return ready_; }
  const RadioSnapshot& snapshot() const { return snapshot_; }
private:
  static constexpr uint32_t kMinFrequencyKHz = 87500;
  static constexpr uint32_t kMaxFrequencyKHz = 108000;
  static constexpr uint32_t kChannelStepKHz = 100;
  static constexpr uint32_t kRdsPollIntervalMs = 250;
  TunerDriver tuner_;
  RadioSnapshot snapshot_{};
  bool ready_ = false;
  uint32_t lastPollMs_ = 0;
  uint32_t lastRdsPollMs_ = 0;
  uint32_t generation_ = 0;
};
