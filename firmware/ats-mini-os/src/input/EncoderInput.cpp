#include "EncoderInput.h"

namespace {
constexpr uint8_t kEncoderPinA = 2;
constexpr uint8_t kEncoderPinB = 1;
constexpr uint8_t kEncoderButtonPin = 21;
constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kLongPressMs = 700;
constexpr int8_t kTransitionTable[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};
}

void EncoderInput::begin() {
  pinMode(kEncoderPinA, INPUT_PULLUP);
  pinMode(kEncoderPinB, INPUT_PULLUP);
  pinMode(kEncoderButtonPin, INPUT_PULLUP);
  const uint8_t a = digitalRead(kEncoderPinA) ? 1 : 0;
  const uint8_t b = digitalRead(kEncoderPinB) ? 1 : 0;
  previousAB_ = static_cast<uint8_t>((a << 1) | b);
  transitionAccumulator_ = 0;
  const bool pressed = digitalRead(kEncoderButtonPin) == LOW;
  rawButtonPressed_ = pressed;
  stableButtonPressed_ = pressed;
  buttonChangedMs_ = millis();
  pressStartedMs_ = pressed ? buttonChangedMs_ : 0;
  longPressSent_ = false;
  queueHead_ = queueTail_ = queueCount_ = 0;
  Serial.printf("[INPUT] encoder ready A=%u B=%u SW=%u\n",
                kEncoderPinA, kEncoderPinB, kEncoderButtonPin);
}

void EncoderInput::update() {
  const uint8_t a = digitalRead(kEncoderPinA) ? 1 : 0;
  const uint8_t b = digitalRead(kEncoderPinB) ? 1 : 0;
  const uint8_t currentAB = static_cast<uint8_t>((a << 1) | b);
  const uint8_t transition = static_cast<uint8_t>((previousAB_ << 2) | currentAB);
  transitionAccumulator_ += kTransitionTable[transition];
  previousAB_ = currentAB;

  // The ATS Mini Dev decoder emits half-step events (at both 00 and 11).
  // Waiting for four transitions here required a full detent and made the OS
  // feel noticeably slower. Emit after two valid transitions, matching that
  // half-step cadence while keeping the existing direction table and accel.
  if (transitionAccumulator_ >= 2) {
    queueEvent(EncoderEventType::RotateClockwise, acceleratedDelta(1, millis()));
    transitionAccumulator_ = 0;
  } else if (transitionAccumulator_ <= -2) {
    queueEvent(EncoderEventType::RotateCounterClockwise, acceleratedDelta(-1, millis()));
    transitionAccumulator_ = 0;
  }

  const uint32_t now = millis();
  const bool rawPressed = digitalRead(kEncoderButtonPin) == LOW;
  if (rawPressed != rawButtonPressed_) {
    rawButtonPressed_ = rawPressed;
    buttonChangedMs_ = now;
  }
  if (rawButtonPressed_ != stableButtonPressed_ &&
      now - buttonChangedMs_ >= kButtonDebounceMs) {
    stableButtonPressed_ = rawButtonPressed_;
    if (stableButtonPressed_) {
      pressStartedMs_ = now;
      longPressSent_ = false;
    } else if (!longPressSent_) {
      queueEvent(EncoderEventType::ShortPress, 0);
    }
  }
  if (stableButtonPressed_ && !longPressSent_ &&
      now - pressStartedMs_ >= kLongPressMs) {
    longPressSent_ = true;
    queueEvent(EncoderEventType::LongPress, 0);
  }
}

int16_t EncoderInput::acceleratedDelta(int8_t direction, uint32_t now) {
  // Match ATS Mini Dev's smoothed encoder-speed thresholds and 1/2/4/8/16x
  // acceleration. Timing is measured when a complete detent is decoded.
  constexpr uint32_t speedThresholds[] = {350, 60, 45, 35, 25};
  constexpr uint16_t accelFactors[] = {1, 2, 4, 8, 16};

  if (!haveLastRotation_) {
    haveLastRotation_ = true;
    lastRotationMs_ = now;
    smoothedRotationMs_ = speedThresholds[0];
    lastRotationDirection_ = direction;
    return direction;
  }

  const uint32_t elapsed = now - lastRotationMs_;
  smoothedRotationMs_ = (elapsed * 7U + smoothedRotationMs_ * 3U) / 10U;
  uint16_t factor = 1;

  if (smoothedRotationMs_ <= speedThresholds[0] &&
      lastRotationDirection_ == direction) {
    for (int8_t i = 4; i >= 0; --i) {
      if (smoothedRotationMs_ <= speedThresholds[i]) {
        factor = accelFactors[i];
        break;
      }
    }
  }

  lastRotationMs_ = now;
  lastRotationDirection_ = direction;
  return static_cast<int16_t>(direction * factor);
}

bool EncoderInput::poll(EncoderEvent& event) {
  if (queueCount_ == 0) {
    event = {};
    return false;
  }
  event = eventQueue_[queueHead_];
  queueHead_ = static_cast<uint8_t>((queueHead_ + 1) % kEventQueueCapacity);
  --queueCount_;
  return true;
}

void EncoderInput::queueEvent(EncoderEventType type, int16_t delta) {
  // Keep older events intact; if the small queue fills, drop the newest event
  // rather than silently overwriting an event already waiting for poll().
  if (queueCount_ >= kEventQueueCapacity) {
    Serial.println("[INPUT] encoder event queue full; dropping newest event");
    return;
  }
  eventQueue_[queueTail_].type = type;
  eventQueue_[queueTail_].delta = delta;
  eventQueue_[queueTail_].timestampMs = millis();
  queueTail_ = static_cast<uint8_t>((queueTail_ + 1) % kEventQueueCapacity);
  ++queueCount_;
}
