#pragma once
#include <Arduino.h>
#include <SI4735.h>
#include "../radio/BandManager.h"

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

  uint16_t currentBlockB() const {
    return static_cast<uint16_t>(
      (static_cast<uint16_t>(currentRdsStatus.resp.BLOCKBH) << 8) |
      currentRdsStatus.resp.BLOCKBL);
  }

  uint16_t currentBlockD() const {
    return static_cast<uint16_t>(
      (static_cast<uint16_t>(currentRdsStatus.resp.BLOCKDH) << 8) |
      currentRdsStatus.resp.BLOCKDL);
  }

  // Decode RDS group 4A Clock-Time and apply its transmitted local-time offset.
  bool currentRdsClock(uint8_t& hour, uint8_t& minute) {
    if (getRdsGroupType() != 4 || getRdsVersionCode() != 0) return false;
    const uint16_t blockB = currentBlockB();
    const uint16_t blockC = currentBlockC();
    const uint16_t blockD = currentBlockD();
    const uint32_t mjd = (static_cast<uint32_t>(blockB & 0x0003) << 15) | (blockC >> 1);
    (void)mjd; // Date is not displayed; the transmitted time and offset are used.
    int utcMinutes = static_cast<int>(((blockC & 0x0001) << 4) | (blockD >> 12)) * 60
                   + static_cast<int>((blockD >> 6) & 0x003F);
    if (utcMinutes >= 24 * 60 || ((blockD >> 6) & 0x003F) > 59) return false;
    const int offsetHalfHours = blockD & 0x001F;
    const int offsetMinutes = offsetHalfHours * 30;
    utcMinutes += (blockD & 0x0020) ? -offsetMinutes : offsetMinutes;
    utcMinutes = (utcMinutes % 1440 + 1440) % 1440;
    hour = static_cast<uint8_t>(utcMinutes / 60);
    minute = static_cast<uint8_t>(utcMinutes % 60);
    return true;
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
  bool clockValid = false;
  uint8_t clockHour = 0;
  uint8_t clockMinute = 0;
  char stationName[9] = {};
  char radioText[65] = {};
};

class TunerDriver {
public:
  bool begin(uint32_t initialFrequencyKHz);
  bool tune(uint32_t frequencyKHz);
  bool setBand(const RadioBandDefinition& band);
  bool tune(uint32_t frequencyKHz, const RadioBandDefinition& band);
  RadioModulation modulation() const { return modulation_; }
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
  bool ssbPatchLoaded_ = false;
  uint8_t volume_ = 25;
  bool haveTextFlag_ = false;
  uint8_t lastTextFlag_ = 0;
  RadioModulation modulation_ = RadioModulation::FM;
  uint32_t minimumFrequencyKHz_ = 87500;
  uint32_t maximumFrequencyKHz_ = 108000;
  uint16_t stepKHz_ = 100;
};
