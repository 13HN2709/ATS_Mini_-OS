#pragma once
#include <Arduino.h>
#include "BandManager.h"

// Immutable-style data contract for publishing tuner state to UI and services.
// Only the future radio-core/RDS-owner modules should produce snapshots.
struct RadioSnapshot {
  uint32_t frequencyKHz = 0;
  char bandName[8] = "VHF";
  RadioModulation modulation = RadioModulation::FM;
  uint32_t bandMinimumKHz = 87500;
  uint32_t bandMaximumKHz = 108000;
  uint16_t tuningStepKHz = 100;
  int16_t rssi = -1;
  int16_t snr = -1;
  uint16_t piCode = 0;
  uint8_t pty = 0;
  bool stereo = false;
  bool rdsAvailable = false;
  bool rdsClockValid = false;
  uint8_t rdsClockHour = 0;
  uint8_t rdsClockMinute = 0;
  char stationName[9] = {};
  char radioText[65] = {};
  uint32_t generation = 0;
};
