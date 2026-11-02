#include "TunerDriver.h"
#include <Wire.h>

namespace {
constexpr uint8_t kI2cSdaPin = 18;
constexpr uint8_t kI2cSclPin = 17;
constexpr uint16_t kMinFrequency10KHz = 8750;
constexpr uint16_t kMaxFrequency10KHz = 10800;
constexpr uint16_t kDefaultSpacing10KHz = 10;
constexpr uint8_t kDefaultVolume = 35;
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

  radio_.setVolume(kDefaultVolume);
  radio_.RdsInit();
  radio_.setRdsConfig(1, 2, 2, 2, 2);

  // Give the receiver time to settle before making the audio path live.
  // Keep the amplifier disabled and the external mute asserted during setup.
  delay(100);
  digitalWrite(kAmplifierEnablePin, HIGH);
  delay(5);
  digitalWrite(kAudioMutePin, LOW);
  ready_ = true;
  return true;
}

bool TunerDriver::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < kMinFrequencyKHz || frequencyKHz > kMaxFrequencyKHz) return false;
  radio_.setFrequency(static_cast<uint16_t>(frequencyKHz / 10UL));
  haveTextFlag_ = false;
  return true;
}

uint32_t TunerDriver::frequencyKHz() const {
  if (!ready_) return 0;
  return static_cast<uint32_t>(const_cast<TunerSI4735&>(radio_).getCurrentFrequency()) * 10UL;
}

void TunerDriver::refreshSignalQuality() {
  if (ready_) radio_.getCurrentReceivedSignalQuality();
}

bool TunerDriver::pollRds(TunerRdsData& out) {
  out = TunerRdsData{};
  if (!ready_) return false;
  // One status query per call; application-side processing is deliberately bounded.
  radio_.getRdsStatus(0, 0, 0);
  if (!radio_.getRdsReceived()) return false;
  out.received = true;
  const uint8_t group = radio_.getRdsGroupType();

  if (radio_.getRdsNewBlockA()) {
    const uint16_t pi = radio_.getRdsPI();
    if (pi != 0) { out.piValid = true; out.piCode = pi; }
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
      // PU2CLR 2.1.8 does not automatically clear the RT buffers when the
      // A/B flag changes. Clear both before accepting segments from the new text.
      radio_.clearRadioTextBuffers();
    }
    const char* rt = radio_.getRdsVersionCode() == 0
                         ? radio_.getRdsText2A()
                         : radio_.getRdsText2B();
    if (textChanged) {
      // Publish the freshly cleared/rebuilt buffer, never the previous message.
      copyRdsText(out.radioText, sizeof(out.radioText), rt);
      out.radioTextUpdated = true;
    } else if (rt) {
      copyRdsText(out.radioText, sizeof(out.radioText), rt);
      out.radioTextUpdated = true;
    }
  }
  return true;
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
