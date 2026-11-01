#include "TunerDriver.h"

#include <Wire.h>

namespace {
constexpr uint8_t kI2cSdaPin = 18;
constexpr uint8_t kI2cSclPin = 17;
constexpr uint16_t kMinFrequency10KHz = 8750;
constexpr uint16_t kMaxFrequency10KHz = 10800;
constexpr uint16_t kDefaultSpacing10KHz = 10;
constexpr uint8_t kDefaultVolume = 35;
// PU2CLR SI4735 setup() defaultFunction: 0 = FM, 1 = AM.
constexpr uint8_t kFmFunction = 0;
}

bool TunerDriver::begin(uint32_t initialFrequencyKHz) {
  ready_ = false;

  // ATS Mini V4 reference pin map: SDA=18, SCL=17, radio LDO=15, reset=16.
  pinMode(kRadioLdoPin, OUTPUT);
  digitalWrite(kRadioLdoPin, HIGH);
  delay(10);

  // PU2CLR SI4735 2.1.8 calls Wire.begin() internally. Configure the custom
  // ATS Mini pins first so its internal begin() uses SDA=18 and SCL=17.
  if (!Wire.setPins(kI2cSdaPin, kI2cSclPin)) {
    Serial.println("[TUNER] ERROR: I2C pin configuration failed");
    return false;
  }

  // This library method pulses RESET, probes both supported SI47XX addresses
  // (0x11 and 0x63), and returns 0 if no device responds.
  const int16_t i2cAddress = radio_.getDeviceI2CAddress(kRadioResetPin);
  if (i2cAddress == 0) {
    Serial.println("[TUNER] ERROR: SI4735 not detected on I2C (0x11/0x63)");
    return false;
  }
  Serial.printf("[TUNER] SI47XX detected at I2C address 0x%02X\n",
                static_cast<unsigned>(i2cAddress));

  radio_.setup(kRadioResetPin, 0, kFmFunction);
  radio_.setFM(kMinFrequency10KHz, kMaxFrequency10KHz,
               static_cast<uint16_t>(constrain(initialFrequencyKHz / 10UL,
                                               kMinFrequency10KHz,
                                               kMaxFrequency10KHz)),
               kDefaultSpacing10KHz);
  radio_.setVolume(kDefaultVolume);

  ready_ = true;
  return true;
}

bool TunerDriver::tune(uint32_t frequencyKHz) {
  if (!ready_ || frequencyKHz < kMinFrequencyKHz ||
      frequencyKHz > kMaxFrequencyKHz) {
    return false;
  }

  // SI4735 library frequency values use 10 kHz units for FM.
  radio_.setFrequency(static_cast<uint16_t>(frequencyKHz / 10UL));
  return true;
}

uint32_t TunerDriver::frequencyKHz() const {
  if (!ready_) return 0;
  return static_cast<uint32_t>(const_cast<SI4735&>(radio_).getCurrentFrequency()) * 10UL;
}

void TunerDriver::refreshSignalQuality() {
  if (!ready_) return;
  // PU2CLR requires an RSQ_STATUS query before reading RSSI/SNR/pilot values.
  radio_.getCurrentReceivedSignalQuality();
}

int16_t TunerDriver::rssi() const {
  if (!ready_) return -1;
  return const_cast<SI4735&>(radio_).getCurrentRSSI();
}

int16_t TunerDriver::snr() const {
  if (!ready_) return -1;
  return const_cast<SI4735&>(radio_).getCurrentSNR();
}

bool TunerDriver::stereo() const {
  if (!ready_) return false;
  return const_cast<SI4735&>(radio_).getCurrentPilot();
}
