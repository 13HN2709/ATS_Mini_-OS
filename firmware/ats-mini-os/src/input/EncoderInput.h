#pragma once
#include <Arduino.h>

enum class EncoderEventType : uint8_t {
  None = 0,
  RotateClockwise,
  RotateCounterClockwise,
  ShortPress,
  LongPress
};

struct EncoderEvent {
  EncoderEventType type = EncoderEventType::None;
  int16_t delta = 0;
  uint32_t timestampMs = 0;
};

// Hardware-independent event contract. GPIO/debounce implementation will be
// added only after the ATS Mini V4 pin map and encoder behavior are verified.
class EncoderInput {
public:
  void begin();
  void update();
  bool poll(EncoderEvent& event);
};
