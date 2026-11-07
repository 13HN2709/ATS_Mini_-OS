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
  bool agcAutomatic(bool fallback = true);
  bool saveAgcAutomatic(bool automatic);
  uint8_t agcGainIndex(uint8_t fallback = 20);
  bool saveAgcGainIndex(uint8_t index);
  uint8_t softMuteAttenuationDb(uint8_t fallback = 8);
  bool saveSoftMuteAttenuationDb(uint8_t attenuationDb);
  uint8_t avcGain(uint8_t fallback = 90);
  bool saveAvcGain(uint8_t gain);
  bool isReady() const { return ready_; }

private:
  static constexpr char kNamespace[] = "atsmini_os";
  static constexpr char kFrequencyKey[] = "fm_freq";
  static constexpr char kDisplayBrightnessKey[] = "disp_bri";
  static constexpr char kDisplayThemeKey[] = "disp_theme";
  static constexpr char kAgcAutomaticKey[] = "rx_agc_auto";
  static constexpr char kAgcGainIndexKey[] = "rx_agc_idx";
  static constexpr char kSoftMuteKey[] = "rx_softmute";
  static constexpr char kAvcGainKey[] = "rx_avc";

  Preferences preferences_;
  bool ready_ = false;
};
