#include "RadioCore.h"
#include <stdio.h>
#include <string.h>

namespace {
void copyField(char* dst, size_t capacity, const char* src) {
  if (!capacity || !src) return;
  size_t i = 0;
  for (; i + 1 < capacity && src[i] != '\0'; ++i) dst[i] = src[i];
  dst[i] = '\0';
}
void publishBand(RadioSnapshot& snapshot, const RadioBandDefinition& band) {
  strncpy(snapshot.bandName, band.name, sizeof(snapshot.bandName) - 1);
  snapshot.bandName[sizeof(snapshot.bandName) - 1] = '\0';
  snapshot.modulation = band.modulation;
  snapshot.bandMinimumKHz = band.minimumKHz;
  snapshot.bandMaximumKHz = band.maximumKHz;
  snapshot.tuningStepKHz = band.stepKHz;
}
}

bool RadioCore::begin(uint32_t initialFrequencyKHz) {
  BandManager::select(0);
  ready_ = tuner_.begin(initialFrequencyKHz);
  if (!ready_) return false;
  const auto& band = BandManager::current();
  publishBand(snapshot_, band);
  tuner_.refreshSignalQuality();
  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.rssi = tuner_.rssi();
  snapshot_.snr = tuner_.snr();
  snapshot_.stereo = tuner_.stereo();
  snapshot_.generation = ++generation_;
  lastPollMs_ = millis();
  lastRdsPollMs_ = lastPollMs_;
  Serial.printf("[RADIO] Ready; band=%s start=%lu kHz\n",
                snapshot_.bandName, static_cast<unsigned long>(snapshot_.frequencyKHz));
  return true;
}

void RadioCore::update() {
  if (!ready_) return;
  const uint32_t now = millis();
  if (now - lastPollMs_ >= 500) {
    lastPollMs_ = now;
    tuner_.refreshSignalQuality();
    snapshot_.frequencyKHz = tuner_.frequencyKHz();
    snapshot_.rssi = tuner_.rssi();
    snapshot_.snr = tuner_.snr();
    snapshot_.stereo = tuner_.stereo();
  }
  if (snapshot_.modulation != RadioModulation::FM) return;
  if (now - lastRdsPollMs_ >= kRdsPollIntervalMs) {
    lastRdsPollMs_ = now;
    TunerRdsData rds;
    if (tuner_.pollRds(rds)) {
      if (rds.piValid) snapshot_.piCode = rds.piCode;
      if (rds.ptyValid) snapshot_.pty = rds.pty;
      if (rds.clockValid) {
        snapshot_.rdsClockHour = rds.clockHour;
        snapshot_.rdsClockMinute = rds.clockMinute;
        snapshot_.rdsClockValid = true;
      }
      if (rds.stationNameUpdated) copyField(snapshot_.stationName, sizeof(snapshot_.stationName), rds.stationName);
      if (rds.radioTextUpdated && rds.radioText[0] != '\0')
        copyField(snapshot_.radioText, sizeof(snapshot_.radioText), rds.radioText);
      snapshot_.rdsAvailable = true;
    }
  }
}

bool RadioCore::tune(uint32_t frequencyKHz) {
  const auto& band = BandManager::current();
  if (!ready_ || !BandManager::contains(BandManager::currentIndex(), frequencyKHz) ||
      !tuner_.tune(frequencyKHz, band)) return false;
  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.piCode = 0;
  snapshot_.pty = 0;
  snapshot_.rdsAvailable = false;
  snapshot_.rdsClockValid = false;
  snapshot_.stationName[0] = '\0';
  snapshot_.radioText[0] = '\0';
  snapshot_.generation = ++generation_;
  return true;
}

bool RadioCore::selectBand(uint8_t index) {
  if (!ready_ || index >= BandManager::count()) return false;
  const auto& band = BandManager::at(index);
  if (!tuner_.setBand(band) || !BandManager::select(index)) return false;
  publishBand(snapshot_, band);
  snapshot_.frequencyKHz = tuner_.frequencyKHz();
  snapshot_.piCode = 0;
  snapshot_.pty = 0;
  snapshot_.rdsAvailable = false;
  snapshot_.rdsClockValid = false;
  snapshot_.stationName[0] = '\0';
  snapshot_.radioText[0] = '\0';
  snapshot_.generation = ++generation_;
  lastRdsPollMs_ = millis();
  Serial.printf("[RADIO] band=%s mode=%u freq=%lu kHz\n",
                band.name, static_cast<unsigned>(band.modulation),
                static_cast<unsigned long>(snapshot_.frequencyKHz));
  return true;
}

bool RadioCore::nextBand(int8_t direction) {
  if (!direction) return false;
  int16_t index = static_cast<int16_t>(BandManager::currentIndex()) + (direction > 0 ? 1 : -1);
  if (index < 0) index = BandManager::count() - 1;
  if (index >= BandManager::count()) index = 0;
  return selectBand(static_cast<uint8_t>(index));
}

bool RadioCore::setVolume(uint8_t volume) {
  return ready_ && tuner_.setVolume(volume);
}

bool RadioCore::setBandwidthChoice(uint8_t choice) {
  if (!ready_ || !tuner_.setBandwidthChoice(choice)) return false;
  snapshot_.generation = ++generation_;
  return true;
}

bool RadioCore::setStepChoice(uint8_t choice) {
  static const uint16_t fmSteps[] = {10, 50, 100, 200, 1000};
  static const uint16_t amSteps[] = {1, 5, 9, 10};
  if (!ready_ || choice >= (snapshot_.modulation == RadioModulation::FM ? 5 : 4)) return false;
  const uint16_t step = snapshot_.modulation == RadioModulation::FM ? fmSteps[choice] : amSteps[choice];
  if (!tuner_.setStepKHz(step) || !BandManager::setCurrentStepKHz(step)) return false;
  snapshot_.generation = ++generation_;
  Serial.printf("[RADIO] step=%u kHz mode=%u\\n", static_cast<unsigned>(step), static_cast<unsigned>(snapshot_.modulation));
  return true;
}

bool RadioCore::nextChannel(int16_t acceleratedDelta) {
  if (!ready_ || acceleratedDelta == 0) return false;
  const auto& band = BandManager::current();
  const uint32_t step = BandManager::currentStepKHz() ? BandManager::currentStepKHz() : (band.stepKHz ? band.stepKHz : 1);
  const int64_t span = static_cast<int64_t>(band.maximumKHz) - band.minimumKHz + step;
  int64_t offset = static_cast<int64_t>(snapshot_.frequencyKHz) - band.minimumKHz;
  offset += static_cast<int64_t>(acceleratedDelta) * step;
  offset %= span;
  if (offset < 0) offset += span;
  return tune(band.minimumKHz + static_cast<uint32_t>(offset));
}

bool RadioCore::setAgcAutomatic(bool automatic) { return ready_ && tuner_.setAgcAutomatic(automatic); }
bool RadioCore::setSoftMuteEnabled(bool enabled) { return ready_ && tuner_.setSoftMuteEnabled(enabled); }
bool RadioCore::setAvcGain(uint8_t gain) { return ready_ && tuner_.setAvcGain(gain); }
