#include "SettingsStore.h"

bool SettingsStore::begin() {
  // Match ats-mini-dev: use the dedicated NVS partition named "settings".
  // Do not fall back to the default NVS partition, which may belong to the
  // boot environment rather than this application's settings storage.
  ready_ = preferences_.begin(kNamespace, false, "settings");
  return ready_;
}

void SettingsStore::end() {
  if (ready_) {
    preferences_.end();
    ready_ = false;
  }
}

uint32_t SettingsStore::lastFmFrequencyKHz(uint32_t fallbackKHz) {
  if (!ready_) return fallbackKHz;

  const uint32_t saved = preferences_.getUInt(kFrequencyKey, fallbackKHz);
  // Validate persisted values; corrupted or stale values must never be sent
  // to the tuner. The current first milestone is FM-only.
  if (saved < 87500 || saved > 108000 || (saved % 10) != 0) {
    return fallbackKHz;
  }
  return saved;
}

bool SettingsStore::saveLastFmFrequencyKHz(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < 87500 || frequencyKHz > 108000 ||
      (frequencyKHz % 10) != 0) {
    return false;
  }
  return preferences_.putUInt(kFrequencyKey, frequencyKHz) == sizeof(uint32_t);
}

uint8_t SettingsStore::displayBrightness(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kDisplayBrightnessKey, fallback);
  // Keep the backlight above the hardware's unreliable near-zero range.
  return saved < 10 ? fallback : saved;
}

bool SettingsStore::saveDisplayBrightness(uint8_t brightness) {
  if (!ready_ || brightness < 10) return false;
  return preferences_.putUChar(kDisplayBrightnessKey, brightness) == sizeof(uint8_t);
}
