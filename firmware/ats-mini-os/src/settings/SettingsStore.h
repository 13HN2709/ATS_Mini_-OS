#pragma once

#include <Arduino.h>
#include <Preferences.h>

// Persistent settings owned by this module. Uses ESP32 NVS rather than a
// shared LittleFS partition so this OS does not depend on filesystem layout.
// Namespace is deliberately unique to ATS Mini OS.
class SettingsStore {
public:
  bool begin();
  void end();

  uint32_t lastFmFrequencyKHz(uint32_t fallbackKHz) const;
  bool saveLastFmFrequencyKHz(uint32_t frequencyKHz);
  bool isReady() const { return ready_; }

private:
  static constexpr char kNamespace[] = "atsmini_os";
  static constexpr char kFrequencyKey[] = "fm_freq";

  Preferences preferences_;
  bool ready_ = false;
};
