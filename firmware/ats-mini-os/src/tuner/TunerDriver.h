#pragma once

#include <Arduino.h>
#include <SI4735.h>

// Owns the SI4735 hardware instance and all direct tuner-library calls.
// Only RadioCore should call this driver in the application layer.
class TunerDriver {
public:
  bool begin(uint32_t initialFrequencyKHz);
  bool tune(uint32_t frequencyKHz);
  uint32_t frequencyKHz() const;
  void refreshSignalQuality();
  int16_t rssi() const;
  int16_t snr() const;
  bool stereo() const;
  bool ready() const { return ready_; }

private:
  static constexpr uint8_t kRadioLdoPin = 15;
  static constexpr uint8_t kRadioResetPin = 16;
  static constexpr uint32_t kMinFrequencyKHz = 87500;
  static constexpr uint32_t kMaxFrequencyKHz = 108000;

  SI4735 radio_;
  bool ready_ = false;
};
