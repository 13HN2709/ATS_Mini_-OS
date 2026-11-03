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

  // Read the complete current RDS group already fetched by pollRds().
  // SI4735 2.1.8 getRdsPI() returns only BLOCKAL, so combine both PI bytes here.
  uint16_t currentPiCode() const {
    return static_cast<uint16_t>(
      (static_cast<uint16_t>(currentRdsStatus.resp.BLOCKAH) << 8) |
      currentRdsStatus.resp.BLOCKAL);
  }

  uint8_t currentBlockCError() const {
    return currentRdsStatus.resp.BLEC;
  }

  uint16_t currentBlockC() const {
    return static_cast<uint16_t>(
      (static_cast<uint16_t>(currentRdsStatus.resp.BLOCKCH) << 8) |
      currentRdsStatus.resp.BLOCKCL);
  }
};

struct TunerRdsData {
  bool received = false;
  bool piValid = false;
  bool ptyValid = false;
  bool eccValid = false;
  bool stationNameUpdated = false;
  bool radioTextUpdated = false;
  uint16_t piCode = 0;
  uint8_t pty = 0;
  uint8_t eccCode = 0;
  char stationName[9] = {};
  char radioText[65] = {};
};

class TunerDriver {
public:
  bool begin(uint32_t initialFrequencyKHz);
  bool tune(uint32_t frequencyKHz);
  bool setVolume(uint8_t volume);
  uint8_t volume() const { return volume_; }
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
  uint8_t volume_ = 25;
  bool haveTextFlag_ = false;
  uint8_t lastTextFlag_ = 0;
};
