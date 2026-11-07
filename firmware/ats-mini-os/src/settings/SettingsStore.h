#pragma once

#include <Arduino.h>
#include <Preferences.h>

// Persistent settings owned by this module. Uses the same dedicated NVS
// partition ("settings") as ats-mini-dev. The namespace remains unique to
// ATS Mini OS so its keys do not collide with the reference firmware.
class SettingsStore {
public:
  bool begin();
  void end();

  uint32_t lastFmFrequencyKHz(uint32_t fallbackKHz);
  bool saveLastFmFrequencyKHz(uint32_t frequencyKHz);

  uint8_t displayBrightness(uint8_t fallback = 180);
  bool saveDisplayBrightness(uint8_t brightness);
  uint8_t displayTheme(uint8_t fallback = 0);
  bool saveDisplayTheme(uint8_t theme);
  bool isReady() const { return ready_; }

private:
  static constexpr char kNamespace[] = "atsmini_os";
  static constexpr char kFrequencyKey[] = "fm_freq";
  static constexpr char kDisplayBrightnessKey[] = "disp_bri";
  static constexpr char kDisplayThemeKey[] = "disp_theme";

  Preferences preferences_;
  bool ready_ = false;
};
