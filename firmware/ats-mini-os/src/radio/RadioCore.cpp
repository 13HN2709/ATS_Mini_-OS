#include "RadioCore.h"
#include <stdio.h>

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
      if (rds.eccValid) {
        snapshot_.eccCode = rds.eccCode;
        snapshot_.eccValid = true;
      }
      if (snapshot_.piCode != 0 && snapshot_.eccValid) {
        // GCC = PI country-code nibble + the received 8-bit ECC.
        snprintf(snapshot_.gcc, sizeof(snapshot_.gcc), "%1X%02X",
          static_cast<unsigned>((snapshot_.piCode >> 12) & 0x0F),
          static_cast<unsigned>(snapshot_.eccCode));
      }
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
  snapshot_.piCode = 0;
  snapshot_.eccCode = 0;
  snapshot_.eccValid = false;
  snapshot_.gcc[0] = '\0';
  snapshot_.pty = 0;
  snapshot_.rdsAvailable = false;
  snapshot_.stationName[0] = '\0';
  snapshot_.radioText[0] = '\0';
  snapshot_.generation = ++generation_;
  return true;
}

bool RadioCore::nextChannel(int16_t acceleratedDelta) {
  if (!ready_ || acceleratedDelta == 0) return false;
  // The input driver supplies signed 1/2/4/8/16-step deltas, matching ATS Mini
  // Dev's acceleration factors. Wrap cleanly across the FM band limits.
  const int32_t span = static_cast<int32_t>(kMaxFrequencyKHz - kMinFrequencyKHz + kChannelStepKHz);
  int32_t offset = static_cast<int32_t>(snapshot_.frequencyKHz) -
                   static_cast<int32_t>(kMinFrequencyKHz);
  offset += static_cast<int32_t>(acceleratedDelta) * static_cast<int32_t>(kChannelStepKHz);
  offset %= span;
  if (offset < 0) offset += span;
  const uint32_t target = kMinFrequencyKHz + static_cast<uint32_t>(offset);
  return tune(target);
}

RadioDnsFmIdentity RadioCore::radioDnsIdentity() const {
  // Temporary test fallback: Germany (DE) maps to ECC E0 for the RDS country
  // code D, producing GCC DE0. A received ECC always takes precedence.
  if (snapshot_.eccValid && snapshot_.gcc[0] != '\0') {
    return RadioDnsIdentity::buildFm(snapshot_.frequencyKHz, snapshot_.piCode,
                                     snapshot_.gcc);
  }

  // Do not apply the German test fallback to a PI whose country-code nibble
  // does not identify Germany. Cross-border country resolution can be added
  // later when the configurable country setting is implemented.
  if (snapshot_.piCode != 0 &&
      ((snapshot_.piCode >> 12) & 0x0F) == 0x0D) {
    RadioDnsFmIdentity identity =
        RadioDnsIdentity::buildFm(snapshot_.frequencyKHz, snapshot_.piCode, "DE0");
    if (identity.status == RadioDnsIdentityStatus::Ready) {
      identity.countryFallbackUsed = true;
    }
    return identity;
  }

  return RadioDnsIdentity::buildFm(snapshot_.frequencyKHz, snapshot_.piCode, "");
}
