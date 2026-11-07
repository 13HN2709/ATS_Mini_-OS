#include "MenuSSB.h"
#include "../radio/RadioCore.h"
const char* MenuSSB::itemLabel(uint8_t index) {
  static const char* const labels[kItemCount] = {"Zurueck", "BFO -500 Hz", "BFO -100 Hz", "BFO -10 Hz", "BFO +10 Hz", "BFO +100 Hz", "BFO +500 Hz", "BFO Null"};
  return index < kItemCount ? labels[index] : labels[0];
}
bool MenuSSB::apply(RadioCore& radio, uint8_t index) {
  static const int16_t changes[kItemCount] = {0, -500, -100, -10, 10, 100, 500, 0};
  if (index == 0 || index >= kItemCount) return false;
  const int16_t next = index == 7 ? 0 : static_cast<int16_t>(radio.bfoOffsetHz() + changes[index]);
  return next >= -16000 && next <= 16000 && radio.setBfoOffsetHz(next);
}
