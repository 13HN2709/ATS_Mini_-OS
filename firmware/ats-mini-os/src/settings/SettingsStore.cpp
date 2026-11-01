#include "SettingsStore.h"

bool SettingsStore::begin() {
  ready_ = preferences_.begin(kNamespace, false);
  return ready_;
}

void SettingsStore::end() {
  if (ready_) {
    preferences_.end();
    ready_ = false;
  }
}

uint32_t SettingsStore::lastFmFrequencyKHz(uint32_t fallbackKHz) const {
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
