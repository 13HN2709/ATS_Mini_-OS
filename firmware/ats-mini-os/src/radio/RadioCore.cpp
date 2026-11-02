#include "RadioCore.h"

namespace {
void copyField(char* dst, size_t capacity, const char* src) {
  if (!capacity || !src) return;
  size_t i = 0;
  for (; i + 1 < capacity && src[i] != '\0'; ++i) dst[i] = src[i];
  dst[i] = '\0';
}
}

bool RadioCore::begin(uint32_t initialFrequencyKHz) {
  // First milestone deliberately avoids persistent storage: boot and test the
  // tuner/display/RDS path before adding settings, memories or schedules.
  ready_ = tuner_.begin(initialFrequencyKHz);
  if (!ready_) return false;

  tuner_.refreshSignalQuality();
  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.rssi = tuner_.rssi();
  snapshot_.snr = tuner_.snr();
  snapshot_.stereo = tuner_.stereo();
  snapshot_.generation = ++generation_;
  lastPollMs_ = millis();
  lastRdsPollMs_ = lastPollMs_;
  Serial.printf("[RADIO] Test mode; no settings/memories/schedules storage. Start=%lu kHz\n",
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

  if (now - lastRdsPollMs_ >= kRdsPollIntervalMs) {
    lastRdsPollMs_ = now;
    TunerRdsData rds;
    if (tuner_.pollRds(rds)) {
      if (rds.piValid) snapshot_.piCode = rds.piCode;
      if (rds.ptyValid) snapshot_.pty = rds.pty;
      if (rds.stationNameUpdated) copyField(snapshot_.stationName, sizeof(snapshot_.stationName), rds.stationName);
      if (rds.radioTextUpdated) copyField(snapshot_.radioText, sizeof(snapshot_.radioText), rds.radioText);
      snapshot_.rdsAvailable = true;
    }
  }
}

bool RadioCore::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < kMinFrequencyKHz || frequencyKHz > kMaxFrequencyKHz ||
      !tuner_.tune(frequencyKHz)) return false;
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
