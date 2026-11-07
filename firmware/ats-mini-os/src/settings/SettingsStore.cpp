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

uint8_t SettingsStore::displayTheme(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kDisplayThemeKey, fallback);
  return saved < 3 ? saved : fallback;
}

bool SettingsStore::saveDisplayTheme(uint8_t theme) {
  if (!ready_ || theme >= 3) return false;
  return preferences_.putUChar(kDisplayThemeKey, theme) == sizeof(uint8_t);
}

uint8_t SettingsStore::displayLayout(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kDisplayLayoutKey, fallback);
  return saved <= 1 ? saved : fallback;
}

bool SettingsStore::saveDisplayLayout(uint8_t layout) {
  return ready_ && layout <= 1 &&
         preferences_.putUChar(kDisplayLayoutKey, layout) == sizeof(uint8_t);
}

bool SettingsStore::agcAutomatic(bool fallback) {
  return ready_ ? preferences_.getBool(kAgcAutomaticKey, fallback) : fallback;
}
bool SettingsStore::saveAgcAutomatic(bool automatic) {
  return ready_ && preferences_.putBool(kAgcAutomaticKey, automatic) == sizeof(bool);
}
uint8_t SettingsStore::agcGainIndex(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kAgcGainIndexKey, fallback);
  return (saved <= 24 && (saved % 8) == 0) ? saved : fallback;
}
bool SettingsStore::saveAgcGainIndex(uint8_t index) {
  return ready_ && index <= 24 && (index % 8) == 0 &&
         preferences_.putUChar(kAgcGainIndexKey, index) == sizeof(uint8_t);
}
uint8_t SettingsStore::softMuteAttenuationDb(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kSoftMuteKey, fallback);
  return (saved <= 12 && (saved % 4) == 0) ? saved : fallback;
}
bool SettingsStore::saveSoftMuteAttenuationDb(uint8_t attenuationDb) {
  return ready_ && attenuationDb <= 12 && (attenuationDb % 4) == 0 &&
         preferences_.putUChar(kSoftMuteKey, attenuationDb) == sizeof(uint8_t);
}
uint8_t SettingsStore::avcGain(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kAvcGainKey, fallback);
  return (saved >= 12 && saved <= 90) ? saved : fallback;
}
bool SettingsStore::saveAvcGain(uint8_t gain) {
  return ready_ && gain >= 12 && gain <= 90 &&
         preferences_.putUChar(kAvcGainKey, gain) == sizeof(uint8_t);
}
uint8_t SettingsStore::ssbBandwidthChoice(uint8_t fallback) {
  if (!ready_) return fallback;
  const uint8_t saved = preferences_.getUChar(kSsbBandwidthKey, fallback);
  return saved < 6 ? saved : fallback;
}
bool SettingsStore::saveSsbBandwidthChoice(uint8_t choice) {
  return ready_ && choice < 6 && preferences_.putUChar(kSsbBandwidthKey, choice) == sizeof(uint8_t);
}
int16_t SettingsStore::bfoOffsetHz(int16_t fallback) {
  if (!ready_) return fallback;
  const int16_t saved = preferences_.getShort(kBfoOffsetKey, fallback);
  return saved >= -16000 && saved <= 16000 ? saved : fallback;
}
bool SettingsStore::saveBfoOffsetHz(int16_t offsetHz) {
  return ready_ && offsetHz >= -16000 && offsetHz <= 16000 && preferences_.putShort(kBfoOffsetKey, offsetHz) == sizeof(int16_t);
}
