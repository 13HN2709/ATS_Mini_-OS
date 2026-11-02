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

// Polling GPIO driver for ATS Mini V4 encoder (A=GPIO2, B=GPIO1, push=GPIO21).
class EncoderInput {
public:
  void begin();
  void update();
  bool poll(EncoderEvent& event);

private:
  static constexpr uint8_t kEventQueueCapacity = 8;
  void queueEvent(EncoderEventType type, int16_t delta);
  int16_t acceleratedDelta(int8_t direction, uint32_t now);
  uint8_t previousAB_ = 0;
  int8_t transitionAccumulator_ = 0;
  bool haveLastRotation_ = false;
  uint32_t lastRotationMs_ = 0;
  uint32_t smoothedRotationMs_ = 350;
  int8_t lastRotationDirection_ = 0;
  bool rawButtonPressed_ = false;
  bool stableButtonPressed_ = false;
  bool longPressSent_ = false;
  uint32_t buttonChangedMs_ = 0;
  uint32_t pressStartedMs_ = 0;
  EncoderEvent eventQueue_[kEventQueueCapacity]{};
  uint8_t queueHead_ = 0;
  uint8_t queueTail_ = 0;
  uint8_t queueCount_ = 0;
};
