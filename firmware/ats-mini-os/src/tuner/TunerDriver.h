#pragma once
#include <Arduino.h>
#include <SI4735.h>

// Thin adapter: PU2CLR keeps the individual RT buffer-clear helpers protected.
class TunerSI4735 : public SI4735 {
public:
  void clearRadioTextBuffers() {
    clearRdsBuffer2A();
    clearRdsBuffer2B();
  }
};

struct TunerRdsData {
  bool received = false;
  bool piValid = false;
  bool ptyValid = false;
  bool stationNameUpdated = false;
  bool radioTextUpdated = false;
  uint16_t piCode = 0;
  uint8_t pty = 0;
  char stationName[9] = {};
  char radioText[65] = {};
};

class TunerDriver {
public:
  bool begin(uint32_t initialFrequencyKHz);
  bool tune(uint32_t frequencyKHz);
  uint32_t frequencyKHz() const;
  void refreshSignalQuality();
  bool pollRds(TunerRdsData& out);
  int16_t rssi() const;
  int16_t snr() const;
  bool stereo() const;
  bool ready() const { return ready_; }
private:
  static constexpr uint8_t kRadioLdoPin = 15;
  static constexpr uint8_t kRadioResetPin = 16;
  static constexpr uint32_t kMinFrequencyKHz = 87500;
  static constexpr uint32_t kMaxFrequencyKHz = 108000;
  TunerSI4735 radio_;
  bool ready_ = false;
  bool haveTextFlag_ = false;
  uint8_t lastTextFlag_ = 0;
};
