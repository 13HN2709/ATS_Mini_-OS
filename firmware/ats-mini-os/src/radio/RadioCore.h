#pragma once
#include <Arduino.h>
#include "RadioSnapshot.h"
#include "../tuner/TunerDriver.h"

class RadioCore {
public:
  bool begin(uint32_t initialFrequencyKHz = 95400);
  void update();
  bool tune(uint32_t frequencyKHz);
  bool selectBand(uint8_t index);
  uint8_t bandCount() const { return BandManager::count(); }
  uint8_t currentBandIndex() const { return BandManager::currentIndex(); }
  bool nextBand(int8_t direction);
  bool setVolume(uint8_t volume);
  bool setBandwidthChoice(uint8_t choice);
  bool setStepChoice(uint8_t choice);
  uint8_t stepChoice() const { return tuner_.stepChoice(); }
  uint8_t stepChoiceCount() const { return tuner_.stepChoiceCount(); }
  uint8_t bandwidthChoice() const { return tuner_.bandwidthChoice(); }
  uint8_t bandwidthChoiceCount() const { return tuner_.bandwidthChoiceCount(); }
  uint8_t volume() const { return tuner_.volume(); }
  bool nextChannel(int16_t acceleratedDelta);
  bool isReady() const { return ready_; }
  const RadioSnapshot& snapshot() const { return snapshot_; }
private:
  static constexpr uint32_t kRdsPollIntervalMs = 250;
  TunerDriver tuner_;
  RadioSnapshot snapshot_{};
  bool ready_ = false;
  uint32_t lastPollMs_ = 0;
  uint32_t lastRdsPollMs_ = 0;
  uint32_t generation_ = 0;
};
