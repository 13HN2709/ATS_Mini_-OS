#include "RadioCore.h"

namespace {
constexpr uint32_t kFrequencySaveDelayMs = 3000;
}

bool RadioCore::begin(uint32_t initialFrequencyKHz) {
  // Settings storage is optional: radio operation still proceeds with the
  // supplied default if NVS cannot be opened.
  settings_.begin();
  const uint32_t startupFrequency = settings_.lastFmFrequencyKHz(initialFrequencyKHz);

  ready_ = tuner_.begin(startupFrequency);
  if (!ready_) return false;

  tuner_.refreshSignalQuality();
  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.rssi = tuner_.rssi();
  snapshot_.snr = tuner_.snr();
  snapshot_.stereo = tuner_.stereo();
  snapshot_.generation = ++generation_;
  lastPollMs_ = millis();
  lastTuneChangeMs_ = lastPollMs_;
  Serial.printf("[SETTINGS] NVS=%s savedFM=%lu kHz\n",
                settings_.isReady() ? "ready" : "unavailable",
                static_cast<unsigned long>(snapshot_.frequencyKHz));
  return true;
}

void RadioCore::update() {
  if (!ready_) return;
  const uint32_t now = millis();

  if (now - lastPollMs_ >= 500) {
    lastPollMs_ = now;
    tuner_.refreshSignalQuality();
    snapshot_.frequencyKHz = tuner_.frequencyKHz();
    snapshot_.rssi = tuner_.rssi();
    snapshot_.snr = tuner_.snr();
    snapshot_.stereo = tuner_.stereo();
  }

  // Defer NVS writes until tuning has been idle for a short period to reduce
  // flash wear when a user spins the encoder quickly.
  if (frequencySavePending_ && now - lastTuneChangeMs_ >= kFrequencySaveDelayMs) {
    const bool saved = settings_.saveLastFmFrequencyKHz(snapshot_.frequencyKHz);
    Serial.printf("[SETTINGS] FM frequency save: %s (%lu kHz)\n",
                  saved ? "OK" : "FAILED",
                  static_cast<unsigned long>(snapshot_.frequencyKHz));
    frequencySavePending_ = false;
  }
}

bool RadioCore::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < kMinFrequencyKHz ||
      frequencyKHz > kMaxFrequencyKHz ||
      !tuner_.tune(frequencyKHz)) {
    return false;
  }

  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.generation = ++generation_;
  lastTuneChangeMs_ = millis();
  frequencySavePending_ = true;
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
