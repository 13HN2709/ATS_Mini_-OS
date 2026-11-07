#include "TunerDriver.h"
#include <Wire.h>
#include "ssb_patch.h"

namespace {
constexpr uint8_t kI2cSdaPin = 18;
constexpr uint8_t kI2cSclPin = 17;
constexpr uint16_t kMinFrequency10KHz = 6400;
constexpr uint16_t kMaxFrequency10KHz = 10800;
constexpr uint16_t kDefaultSpacing10KHz = 10;
constexpr uint8_t kDefaultVolume = 25;
constexpr uint8_t kFmFunction = 0;
// ATS Mini V4 audio controls: mute is active HIGH; amplifier enable is active HIGH.
constexpr uint8_t kAudioMutePin = 3;
constexpr uint8_t kAmplifierEnablePin = 10;

void copyRdsText(char* dst, size_t capacity, const char* src) {
  if (!capacity || !src) return;
  size_t i = 0;
  for (; i + 1 < capacity && src[i] != '\0'; ++i) {
    const unsigned char ch = static_cast<unsigned char>(src[i]);
    dst[i] = (ch >= 0x20 && ch <= 0x7e) ? static_cast<char>(ch) : ' ';
  }
  dst[i] = '\0';
}

// Match ATS Mini Dev's RT preparation: skip leading whitespace, stop at CR,
// preserve line breaks, wrap at word boundaries after 32 characters and trim
// trailing whitespace. The destination is bounded to the OS snapshot capacity.
void formatRadioText(char* dst, size_t capacity, const char* src) {
  if (!capacity) return;
  dst[0] = '\0';
  if (!src) return;
  size_t in = 0, out = 0;
  // Match Dev: skip leading whitespace, stop at CR, and keep each displayed
  // line to 32 characters, breaking at the next word boundary.
  while (in < 64 && src[in] && static_cast<unsigned char>(src[in]) <= ' ') ++in;
  uint8_t lineLength = 0;
  while (in < 64 && src[in] && src[in] != '\r' && out + 1 < capacity) {
    const char ch = src[in++];
    if (ch == '\n' || (ch == ' ' && lineLength >= 32)) break;
    const unsigned char byte = static_cast<unsigned char>(ch);
    dst[out++] = (byte >= 0x20 && byte <= 0x7e) ? ch : ' ';
    ++lineLength;
  }
  while (out && static_cast<unsigned char>(dst[out - 1]) <= ' ') --out;
  dst[out] = '\0';
}
}

bool TunerDriver::begin(uint32_t initialFrequencyKHz) {
  ready_ = false;
  haveTextFlag_ = false;
  lastTextFlag_ = 0;
  // Keep audio muted and the amplifier disabled while the tuner starts, to avoid
  // startup pops and to ensure a failed tuner initialization cannot leave audio live.
  pinMode(kAudioMutePin, OUTPUT);
  digitalWrite(kAudioMutePin, HIGH);
  pinMode(kAmplifierEnablePin, OUTPUT);
  digitalWrite(kAmplifierEnablePin, LOW);

  pinMode(kRadioLdoPin, OUTPUT);
  digitalWrite(kRadioLdoPin, HIGH);
  delay(10);

  if (!Wire.setPins(kI2cSdaPin, kI2cSclPin)) {
    Serial.println("[TUNER] ERROR: I2C pin configuration failed");
    return false;
  }
  const int16_t address = radio_.getDeviceI2CAddress(kRadioResetPin);
  if (address == 0) {
    Serial.println("[TUNER] ERROR: SI4735 not detected on I2C (0x11/0x63)");
    return false;
  }
  Serial.printf("[TUNER] SI47XX detected at I2C address 0x%02X\n", static_cast<unsigned>(address));
  radio_.setup(kRadioResetPin, 0, kFmFunction);
  // Match the legacy ATS Mini hardware path: let the SI4735 library manage the
  // external active-HIGH mute during tuner operations, not just at boot.
  radio_.setAudioMuteMcuPin(kAudioMutePin);
  radio_.setFM(kMinFrequency10KHz, kMaxFrequency10KHz,
               static_cast<uint16_t>(constrain(initialFrequencyKHz / 10UL,
                                               kMinFrequency10KHz, kMaxFrequency10KHz)),
               kDefaultSpacing10KHz);

  // Mirror the known-good ATS Mini Dev FM setup sequence. These settings
  // configure the receiver after setFM() has selected the startup frequency.
  radio_.setTuneFrequencyAntennaCapacitor(1);
  radio_.setSeekFmLimits(kMinFrequency10KHz, kMaxFrequency10KHz);
  radio_.setSeekFmRssiThreshold(0);
  radio_.setSeekFmSNRThreshold(1);

  volume_ = kDefaultVolume;
  radio_.setVolume(volume_);
  radio_.RdsInit();
  radio_.setRdsConfig(1, 2, 2, 2, 2);

  // Give the receiver time to settle before making the audio path live.
  // Keep the amplifier disabled and the external mute asserted during setup.
  delay(100);
  digitalWrite(kAmplifierEnablePin, HIGH);
  delay(5);
  digitalWrite(kAudioMutePin, LOW);
  minimumFrequencyKHz_ = 64000;
  maximumFrequencyKHz_ = 108000;
  stepKHz_ = 100;
  modulation_ = RadioModulation::FM;
  bandwidthChoice_ = 0;
  radio_.setFmBandwidth(0);
  radio_.setAutomaticGainControl(agcAutomatic_ ? 0 : 1, agcGainIndex_);
  radio_.setFmSoftMuteMaxAttenuation(softMuteAttenuationDb_);
  ready_ = true;
  return true;
}

bool TunerDriver::setAgcAutomatic(bool automatic) {
  if (!ready_) return false;
  agcAutomatic_ = automatic;
  radio_.setAutomaticGainControl(automatic ? 0 : 1, agcGainIndex_);
  Serial.printf("[TUNER] AGC %s (index=%u)\n", automatic ? "AUTO" : "MANUAL", static_cast<unsigned>(agcGainIndex_));
  return true;
}

bool TunerDriver::setAgcManualGain(uint8_t index) {
  if (!ready_ || index > 24 || (index % 8) != 0) return false;
  agcAutomatic_ = false;
  agcGainIndex_ = index;
  radio_.setAutomaticGainControl(1, agcGainIndex_);
  Serial.printf("[TUNER] AGC manual index=%u\n", static_cast<unsigned>(agcGainIndex_));
  return true;
}

bool TunerDriver::setSoftMuteEnabled(bool enabled) {
  if (!ready_) return false;
  softMuteEnabled_ = enabled;
  softMuteAttenuationDb_ = enabled ? 8 : 0;
  const uint8_t attenuation = softMuteAttenuationDb_;
  if (modulation_ == RadioModulation::FM) radio_.setFmSoftMuteMaxAttenuation(attenuation);
  else if (modulation_ == RadioModulation::AM) radio_.setAmSoftMuteMaxAttenuation(attenuation);
  else radio_.setSsbSoftMuteMaxAttenuation(attenuation);
  Serial.printf("[TUNER] soft mute %s\n", enabled ? "ON" : "OFF");
  return true;
}

bool TunerDriver::setSoftMuteAttenuation(uint8_t attenuationDb) {
  if (!ready_ || attenuationDb > 12 || (attenuationDb % 4) != 0) return false;
  softMuteAttenuationDb_ = attenuationDb;
  softMuteEnabled_ = attenuationDb != 0;
  if (modulation_ == RadioModulation::FM) radio_.setFmSoftMuteMaxAttenuation(attenuationDb);
  else if (modulation_ == RadioModulation::AM) radio_.setAmSoftMuteMaxAttenuation(attenuationDb);
  else radio_.setSsbSoftMuteMaxAttenuation(attenuationDb);
  Serial.printf("[TUNER] soft mute attenuation=%u dB\n", static_cast<unsigned>(attenuationDb));
  return true;
}

bool TunerDriver::setBfoOffsetHz(int16_t offsetHz) {
  if (!ready_ || offsetHz < -16000 || offsetHz > 16000) return false;
  bfoOffsetHz_ = offsetHz;
  if (modulation_ == RadioModulation::LSB || modulation_ == RadioModulation::USB) radio_.setSSBBfo(bfoOffsetHz_);
  Serial.printf("[TUNER] BFO offset=%d Hz%s\n", static_cast<int>(bfoOffsetHz_),
                (modulation_ == RadioModulation::LSB || modulation_ == RadioModulation::USB) ? "" : " (stored for SSB)");
  return true;
}

bool TunerDriver::setSsbBandwidthChoice(uint8_t choice) {
  if (choice >= 6) return false;
  ssbBandwidthChoice_ = choice;
  if (!ready_) return true;
  if (modulation_ == RadioModulation::LSB || modulation_ == RadioModulation::USB) return setBandwidthChoice(choice);
  return true;
}

bool TunerDriver::setAvcGain(uint8_t gain) {
  if (!ready_ || gain < 12 || gain > 90) return false;
  // Remember the AM/SSB value while listening to FM; apply it on the next
  // AM/SSB band switch because AVC is not a valid FM control.
  avcGain_ = gain;
  if (modulation_ != RadioModulation::FM) radio_.setAvcAmMaxGain(gain);
  Serial.printf("[TUNER] AVC max gain=%u%s\n", static_cast<unsigned>(gain),
                modulation_ == RadioModulation::FM ? " (stored for AM/SSB)" : "");
  return true;
}

bool TunerDriver::setVolume(uint8_t volume) {
  if (!ready_) return false;
  volume_ = static_cast<uint8_t>(constrain(volume, 0, 63));
  radio_.setVolume(volume_);
  return true;
}

uint8_t TunerDriver::bandwidthChoiceCount() const {
  if (modulation_ == RadioModulation::FM) return 5;
  if (modulation_ == RadioModulation::AM) return 7;
  return 6;
}

bool TunerDriver::setBandwidthChoice(uint8_t choice) {
  if (!ready_ || choice >= bandwidthChoiceCount()) return false;
  static const uint8_t amProperties[] = {4, 5, 3, 6, 2, 1, 0};
  static const uint8_t ssbProperties[] = {4, 5, 0, 1, 2, 3};
  bandwidthChoice_ = choice;
  if (modulation_ == RadioModulation::LSB || modulation_ == RadioModulation::USB) ssbBandwidthChoice_ = choice;
  if (modulation_ == RadioModulation::FM) {
    radio_.setFmBandwidth(choice); // Auto, 110, 84, 60, 40 kHz
  } else if (modulation_ == RadioModulation::AM) {
    radio_.setBandwidth(amProperties[choice], 1);
  } else {
    const uint8_t property = ssbProperties[choice];
    radio_.setSSBAudioBandwidth(property);
    radio_.setSSBSidebandCutoffFilter(property == 0 || property == 4 || property == 5 ? 0 : 1);
    radio_.setSSBConfig(property, 1, 0, 1, 0, 1);
  }
  Serial.printf("[TUNER] bandwidth choice=%u mode=%u\n",
                static_cast<unsigned>(choice), static_cast<unsigned>(modulation_));
  return true;
}

uint8_t TunerDriver::stepChoiceCount() const {
  return 4;
}

uint8_t TunerDriver::stepChoice() const {
  static const uint16_t fmSteps[] = {10, 50, 100, 200, 1000};
  static const uint16_t amSteps[] = {1, 5, 9, 10};
  const uint16_t* steps = modulation_ == RadioModulation::FM ? fmSteps : amSteps;
  for (uint8_t i = 0; i < (modulation_ == RadioModulation::FM ? 5 : 4); ++i) if (steps[i] == stepKHz_) return i;
  return modulation_ == RadioModulation::FM ? 1 : 0;
}

bool TunerDriver::setStepKHz(uint16_t stepKHz) {
  if (!ready_ || !stepKHz) return false;
  if (modulation_ == RadioModulation::FM) {
    if (stepKHz != 10 && stepKHz != 50 && stepKHz != 100 && stepKHz != 200 && stepKHz != 1000) return false;
  } else if (stepKHz != 1 && stepKHz != 5 && stepKHz != 9 && stepKHz != 10) return false;
  stepKHz_ = stepKHz;
  return true;
}

bool TunerDriver::setBand(const RadioBandDefinition& band) {
  if (!ready_) return false;
  const uint32_t target = constrain(band.startKHz, band.minimumKHz, band.maximumKHz);
  minimumFrequencyKHz_ = band.minimumKHz;
  maximumFrequencyKHz_ = band.maximumKHz;
  stepKHz_ = band.stepKHz ? band.stepKHz : 1;
  modulation_ = band.modulation;
  haveTextFlag_ = false;

  if (band.modulation == RadioModulation::FM) {
    bandwidthChoice_ = 0;
    ssbPatchLoaded_ = false;
    radio_.setFM(static_cast<uint16_t>(band.minimumKHz / 10UL),
                 static_cast<uint16_t>(band.maximumKHz / 10UL),
                 static_cast<uint16_t>(target / 10UL),
                 static_cast<uint16_t>(stepKHz_ / 10UL));
    radio_.setTuneFrequencyAntennaCapacitor(1);
    radio_.setSeekFmLimits(static_cast<uint16_t>(band.minimumKHz / 10UL),
                           static_cast<uint16_t>(band.maximumKHz / 10UL));
    radio_.setSeekFmRssiThreshold(0);
    radio_.setSeekFmSNRThreshold(1);
    radio_.setFmBandwidth(0);
    radio_.setAutomaticGainControl(agcAutomatic_ ? 0 : 1, agcGainIndex_);
    radio_.setFmSoftMuteMaxAttenuation(softMuteAttenuationDb_);
    radio_.RdsInit();
    radio_.setRdsConfig(1, 2, 2, 2, 2);
  } else if (band.modulation == RadioModulation::AM) {
    bandwidthChoice_ = 4;
    ssbPatchLoaded_ = false;
    radio_.setAM(static_cast<uint16_t>(band.minimumKHz),
                 static_cast<uint16_t>(band.maximumKHz),
                 static_cast<uint16_t>(target), stepKHz_);
    radio_.setSeekAmLimits(static_cast<uint16_t>(band.minimumKHz),
                           static_cast<uint16_t>(band.maximumKHz));
    radio_.setSeekAmRssiThreshold(10);
    radio_.setSeekAmSNRThreshold(3);
    radio_.setBandwidth(2, 1);
    radio_.setAutomaticGainControl(agcAutomatic_ ? 0 : 1, agcGainIndex_);
    radio_.setAmSoftMuteMaxAttenuation(softMuteAttenuationDb_);
    radio_.setAvcAmMaxGain(avcGain_);
  } else {
    // The SI4735 requires the SSB DSP patch before sideband mode can be used.
    // Load it only when entering SSB; AM/FM transitions invalidate this state.
    bandwidthChoice_ = ssbBandwidthChoice_;
    if (!ssbPatchLoaded_) {
      radio_.loadPatch(ssb_patch_content, sizeof(ssb_patch_content), 2);
      ssbPatchLoaded_ = true;
    }
    const uint8_t mode = band.modulation == RadioModulation::USB ? 2 : 1;
    radio_.setSSB(static_cast<uint16_t>(band.minimumKHz),
                  static_cast<uint16_t>(band.maximumKHz),
                  static_cast<uint16_t>(target), 0, mode);
    radio_.setSSBConfig(2, 1, 0, 1, 0, 1);
    radio_.setSSBBfo(bfoOffsetHz_);
    setBandwidthChoice(ssbBandwidthChoice_);
    radio_.setAutomaticGainControl(agcAutomatic_ ? 0 : 1, agcGainIndex_);
    radio_.setSsbSoftMuteMaxAttenuation(softMuteAttenuationDb_);
    radio_.setAvcAmMaxGain(avcGain_);
  }
  radio_.setVolume(volume_);
  return true;
}

bool TunerDriver::setModulation(const RadioBandDefinition& band, RadioModulation modulation) {
  if (!ready_ || (modulation == RadioModulation::FM && band.type != RadioBandType::FM) ||
      (modulation != RadioModulation::FM && band.type == RadioBandType::FM)) return false;
  RadioBandDefinition configured = band;
  configured.modulation = modulation;
  configured.startKHz = frequencyKHz();
  if (modulation == RadioModulation::LSB || modulation == RadioModulation::USB) {
    if (configured.startKHz < 1800 || configured.startKHz > 30000) return false;
    if (configured.minimumKHz < 1800) configured.minimumKHz = 1800;
    if (configured.maximumKHz > 30000) configured.maximumKHz = 30000;
  }
  return setBand(configured);
}
bool TunerDriver::tune(uint32_t frequencyKHz, const RadioBandDefinition& band) {
  if (!ready_ || frequencyKHz < band.minimumKHz || frequencyKHz > band.maximumKHz) return false;
  if (band.modulation == RadioModulation::FM)
    radio_.setFrequency(static_cast<uint16_t>(frequencyKHz / 10UL));
  else
    radio_.setFrequency(static_cast<uint16_t>(frequencyKHz));
  haveTextFlag_ = false;
  return true;
}

bool TunerDriver::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < minimumFrequencyKHz_ || frequencyKHz > maximumFrequencyKHz_) return false;
  if (modulation_ == RadioModulation::FM)
    radio_.setFrequency(static_cast<uint16_t>(frequencyKHz / 10UL));
  else
    radio_.setFrequency(static_cast<uint16_t>(frequencyKHz));
  haveTextFlag_ = false;
  return true;
}

uint32_t TunerDriver::frequencyKHz() const {
  if (!ready_) return 0;
  const uint32_t raw = static_cast<uint32_t>(const_cast<TunerSI4735&>(radio_).getCurrentFrequency());
  return modulation_ == RadioModulation::FM ? raw * 10UL : raw;
}

void TunerDriver::refreshSignalQuality() {
  if (ready_) radio_.getCurrentReceivedSignalQuality();
}

bool TunerDriver::pollRds(TunerRdsData& out) {
  out = TunerRdsData{};
  if (!ready_) return false;

  // ATS Mini Dev drains up to one SI4735 RDS FIFO capacity per 250 ms cycle.
  // This reconstructs PS/RT much faster than reading just one group each time.
  static constexpr uint8_t kMaxGroupsPerPoll = 25;
  bool receivedAny = false;
  for (uint8_t i = 0; i < kMaxGroupsPerPoll; ++i) {
    radio_.getRdsStatus(0, 0, 0);
    const uint8_t fifoCount = radio_.getNumRdsFifoUsed();
    if (!fifoCount) break;
    receivedAny = true;
    const uint8_t group = radio_.getRdsGroupType();

    if (group == 4 && radio_.currentRdsClock(out.clockHour, out.clockMinute))
      out.clockValid = true;

    if (radio_.getRdsNewBlockA()) {
      const uint16_t pi = radio_.currentPiCode();
      if (pi != 0) { out.piValid = true; out.piCode = pi; }
    }

    if (group == 1 && radio_.getRdsVersionCode() == 0 &&
        radio_.getRdsNewBlockB() && radio_.currentBlockCError() < 3) {
      const uint16_t blockC = radio_.currentBlockC();
      const uint8_t ecc = static_cast<uint8_t>(blockC & 0xFF);
      if (ecc != 0) { out.eccValid = true; out.eccCode = ecc; }
    }
    if (radio_.getRdsNewBlockB()) {
      out.ptyValid = true;
      out.pty = radio_.getRdsProgramType();
    }

    if (group == 0) {
      const char* ps = radio_.getRdsText0A();
      if (ps) {
        copyRdsText(out.stationName, sizeof(out.stationName), ps);
        out.stationNameUpdated = true;
      }
    } else if (group == 2) {
      const uint8_t flag = radio_.getRdsFlagAB();
      const bool textChanged = !haveTextFlag_ || flag != lastTextFlag_;
      if (textChanged) {
        haveTextFlag_ = true;
        lastTextFlag_ = flag;
        // PU2CLR 2.1.8 does not clear the RT buffers on A/B changes.
        radio_.clearRadioTextBuffers();
      }
      const char* rt = radio_.getRdsVersionCode() == 0
                           ? radio_.getRdsText2A()
                           : radio_.getRdsText2B();
      // Publish even an empty freshly-cleared buffer. This prevents old RT
      // from remaining visible while the new A/B text is being assembled.
      formatRadioText(out.radioText, sizeof(out.radioText), rt);
      out.radioTextUpdated = true;
    }

    // The count includes the group just read (AN332), as in ATS Mini Dev.
    if (fifoCount == 1) break;
  }
  out.received = receivedAny;
  return receivedAny;
}

int16_t TunerDriver::rssi() const {
  return ready_ ? const_cast<TunerSI4735&>(radio_).getCurrentRSSI() : -1;
}
int16_t TunerDriver::snr() const {
  return ready_ ? const_cast<TunerSI4735&>(radio_).getCurrentSNR() : -1;
}
bool TunerDriver::stereo() const {
  return ready_ && const_cast<TunerSI4735&>(radio_).getCurrentPilot();
}
