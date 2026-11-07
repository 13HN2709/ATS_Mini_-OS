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
  bool canUseModulation(RadioModulation modulation) const;
  bool setModulation(RadioModulation modulation);
  uint8_t bandCount() const { return BandManager::count(); }
  uint8_t currentBandIndex() const { return BandManager::currentIndex(); }
  bool nextBand(int8_t direction);
  bool setVolume(uint8_t volume);
  bool setAgcAutomatic(bool automatic);
  bool setAgcManualGain(uint8_t index);
  bool setSoftMuteEnabled(bool enabled);
  bool setSoftMuteAttenuation(uint8_t attenuationDb);
  bool setAvcGain(uint8_t gain);
  bool setBfoOffsetHz(int16_t offsetHz);
  int16_t bfoOffsetHz() const { return tuner_.bfoOffsetHz(); }
  bool setSsbBandwidthChoice(uint8_t choice);
  uint8_t ssbBandwidthChoice() const { return tuner_.ssbBandwidthChoice(); }
  bool agcAutomatic() const { return tuner_.agcAutomatic(); }
  uint8_t agcGainIndex() const { return tuner_.agcGainIndex(); }
  uint8_t softMuteAttenuationDb() const { return tuner_.softMuteAttenuationDb(); }
  uint8_t avcGain() const { return tuner_.avcGain(); }
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
