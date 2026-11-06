#include "BandManager.h"

namespace {
// Values follow ATS Mini DEV Menu_german.cpp.src (band table).
// MW1 deliberately starts at 100 kHz as requested; ALL remains the full
// 100 kHz–30 MHz coverage band. LW is presented as its own selectable range.
constexpr RadioBandDefinition kBands[] = {
  {"VHF",   RadioBandType::FM,  RadioModulation::FM,  64000, 108000, 103900, 100, 0},
  {"ALL",   RadioBandType::ALL, RadioModulation::AM,    100,  30000,  15000,   1, 4},
  {"LW",    RadioBandType::LW,  RadioModulation::AM,    100,    495,    100,   1, 4},
  {"MW1",   RadioBandType::MW,  RadioModulation::AM,    100,   1800,    100,   9, 4},
  {"MW2",   RadioBandType::MW,  RadioModulation::AM,    495,   1701,    783,   9, 4},
  {"MW3",   RadioBandType::MW,  RadioModulation::AM,   1700,   3500,   2500,   1, 4},
  {"160M",  RadioBandType::MW,  RadioModulation::LSB,  1800,   2000,   1900,   5, 4},
  {"90M",   RadioBandType::SW,  RadioModulation::AM,   3000,   3500,   3300,   1, 4},
  {"75M",   RadioBandType::SW,  RadioModulation::AM,   3500,   4000,   3950,   1, 4},
  {"80M",   RadioBandType::SW,  RadioModulation::LSB,  3500,   4000,   3800,   5, 4},
  {"60M",   RadioBandType::SW,  RadioModulation::AM,   4000,   5100,   4950,   1, 4},
  {"49M",   RadioBandType::SW,  RadioModulation::AM,   5000,   7000,   6000,   1, 4},
  {"41M",   RadioBandType::SW,  RadioModulation::AM,   7000,   9000,   7300,   1, 4},
  {"40M",   RadioBandType::SW,  RadioModulation::LSB,  7000,   7300,   7150,   5, 4},
  {"31M",   RadioBandType::SW,  RadioModulation::AM,   9000,  11000,   9650,   1, 4},
  {"30M",   RadioBandType::SW,  RadioModulation::LSB, 10000,  10200,  10125,   5, 4},
  {"25M",   RadioBandType::SW,  RadioModulation::AM,  11000,  13000,  11850,   1, 4},
  {"22M",   RadioBandType::SW,  RadioModulation::AM,  13500,  13900,  13650,   1, 4},
  {"20M",   RadioBandType::SW,  RadioModulation::USB, 14000,  14400,  14100,   5, 4},
  {"19M",   RadioBandType::SW,  RadioModulation::AM,  15100,  15900,  15450,   1, 4},
  {"16M",   RadioBandType::SW,  RadioModulation::AM,  17400,  18100,  17650,   1, 4},
  {"17M",   RadioBandType::SW,  RadioModulation::USB, 18000,  18200,  18115,   5, 4},
  {"15M",   RadioBandType::SW,  RadioModulation::AM,  18900,  19100,  18950,   1, 4},
  {"15M",   RadioBandType::SW,  RadioModulation::USB, 21000,  21500,  21225,   5, 4},
  {"13M",   RadioBandType::SW,  RadioModulation::AM,  21500,  21900,  21650,   1, 4},
  {"12M",   RadioBandType::SW,  RadioModulation::USB, 24800,  25000,  24940,   5, 4},
  {"CB",    RadioBandType::SW,  RadioModulation::AM,  25000,  28000,  27135,   1, 4},
  {"11M",   RadioBandType::SW,  RadioModulation::AM,  25600,  26100,  25850,   1, 4},
  {"10M",   RadioBandType::SW,  RadioModulation::USB, 28000,  29700,  28500,   5, 4},
};
uint8_t gCurrentBand = 0;
}

constexpr uint8_t BandManager::count() {
  return static_cast<uint8_t>(sizeof(kBands) / sizeof(kBands[0]));
}
const RadioBandDefinition& BandManager::at(uint8_t index) {
  return kBands[index < count() ? index : 0];
}
uint8_t BandManager::currentIndex() { return gCurrentBand; }
const RadioBandDefinition& BandManager::current() { return at(gCurrentBand); }
bool BandManager::select(uint8_t index) {
  if (index >= count()) return false;
  gCurrentBand = index;
  return true;
}
bool BandManager::next(int8_t direction) {
  if (!direction) return false;
  int16_t nextIndex = static_cast<int16_t>(gCurrentBand) + (direction > 0 ? 1 : -1);
  if (nextIndex < 0) nextIndex = count() - 1;
  if (nextIndex >= count()) nextIndex = 0;
  gCurrentBand = static_cast<uint8_t>(nextIndex);
  return true;
}
bool BandManager::contains(uint8_t index, uint32_t frequencyKHz) {
  const auto& band = at(index);
  return frequencyKHz >= band.minimumKHz && frequencyKHz <= band.maximumKHz;
}
