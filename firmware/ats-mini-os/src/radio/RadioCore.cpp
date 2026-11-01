#include "RadioCore.h"

bool RadioCore::begin(uint32_t initialFrequencyKHz) {
  ready_ = tuner_.begin(initialFrequencyKHz);
  if (!ready_) return false;

  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.rssi = tuner_.rssi();
  snapshot_.snr = tuner_.snr();
  snapshot_.stereo = tuner_.stereo();
  snapshot_.generation = ++generation_;
  lastPollMs_ = millis();
  return true;
}

void RadioCore::update() {
  if (!ready_) return;
  const uint32_t now = millis();
  if (now - lastPollMs_ < 500) return;
  lastPollMs_ = now;

  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.rssi = tuner_.rssi();
  snapshot_.snr = tuner_.snr();
  snapshot_.stereo = tuner_.stereo();
}

bool RadioCore::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < kMinFrequencyKHz ||
      frequencyKHz > kMaxFrequencyKHz ||
      !tuner_.tune(frequencyKHz)) {
    return false;
  }

  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.generation = ++generation_;
  return true;
}

bool RadioCore::nextChannel(int8_t direction) {
  if (!ready_ || direction == 0) return false;
  int32_t target = static_cast<int32_t>(snapshot_.frequencyKHz) +
                   (direction > 0 ? static_cast<int32_t>(kChannelStepKHz)
                                  : -static_cast<int32_t>(kChannelStepKHz));
  if (target > static_cast<int32_t>(kMaxFrequencyKHz)) target = kMinFrequencyKHz;
  if (target < static_cast<int32_t>(kMinFrequencyKHz)) target = kMaxFrequencyKHz;
  return tune(static_cast<uint32_t>(target));
}
