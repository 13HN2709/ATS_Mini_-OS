#include "EncoderInput.h"

namespace {
constexpr uint8_t kEncoderPinA = 2;
constexpr uint8_t kEncoderPinB = 1;
constexpr uint8_t kEncoderButtonPin = 21;
constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kLongPressMs = 700;

// Full-step quadrature decoder. Called from the GPIO interrupt on either pin.
constexpr int8_t kTransitionTable[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};
EncoderInput* volatile gEncoderInstance = nullptr;
void IRAM_ATTR encoderEdgeA() {
  if (gEncoderInstance) gEncoderInstance->handleEncoderInterrupt();
}
void IRAM_ATTR encoderEdgeB() {
  if (gEncoderInstance) gEncoderInstance->handleEncoderInterrupt();
}
}

void EncoderInput::begin() {
  pinMode(kEncoderPinA, INPUT_PULLUP);
  pinMode(kEncoderPinB, INPUT_PULLUP);
  pinMode(kEncoderButtonPin, INPUT_PULLUP);
  const uint8_t a = digitalRead(kEncoderPinA) ? 1 : 0;
  const uint8_t b = digitalRead(kEncoderPinB) ? 1 : 0;
  previousAB_ = static_cast<uint8_t>((a << 1) | b);
  transitionAccumulator_ = 0;
  pendingSteps_ = 0;
  lastRotationMs_ = millis();
  smoothedRotationMs_ = 350;
  lastRotationDirection_ = 0;
  lastAccelFactor_ = 1;
  const bool pressed = digitalRead(kEncoderButtonPin) == LOW;
  rawButtonPressed_ = pressed;
  stableButtonPressed_ = pressed;
  buttonChangedMs_ = millis();
  pressStartedMs_ = pressed ? buttonChangedMs_ : 0;
  longPressSent_ = false;
  queueHead_ = queueTail_ = queueCount_ = 0;

  // Attach both quadrature channels so short transitions are not lost while
  // SI4735 I2C operations or display rendering temporarily occupy loop().
  gEncoderInstance = this;
  attachInterrupt(digitalPinToInterrupt(kEncoderPinA), encoderEdgeA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(kEncoderPinB), encoderEdgeB, CHANGE);
  Serial.printf("[INPUT] encoder ready (interrupt) A=%u B=%u SW=%u\n",
                kEncoderPinA, kEncoderPinB, kEncoderButtonPin);
}

void IRAM_ATTR EncoderInput::handleEncoderInterrupt() {
  const uint8_t a = digitalRead(kEncoderPinA) ? 1 : 0;
  const uint8_t b = digitalRead(kEncoderPinB) ? 1 : 0;
  const uint8_t currentAB = static_cast<uint8_t>((a << 1) | b);
  const uint8_t transition = static_cast<uint8_t>((previousAB_ << 2) | currentAB);
  transitionAccumulator_ += kTransitionTable[transition];
  previousAB_ = currentAB;

  if (transitionAccumulator_ >= 4) {
    if (pendingSteps_ < 32) ++pendingSteps_;
    transitionAccumulator_ = 0;
  } else if (transitionAccumulator_ <= -4) {
    if (pendingSteps_ > -32) --pendingSteps_;
    transitionAccumulator_ = 0;
  }
}

void EncoderInput::update() {
  // Copy and clear ISR-produced steps atomically, then do timing/queue work
  // outside the interrupt context.
  int8_t steps = 0;
  noInterrupts();
  steps = pendingSteps_;
  pendingSteps_ = 0;
  interrupts();

  const uint32_t now = millis();
  while (steps > 0) {
    queueEvent(EncoderEventType::RotateClockwise, acceleratedDelta(1, now));
    --steps;
  }
  while (steps < 0) {
    queueEvent(EncoderEventType::RotateCounterClockwise, acceleratedDelta(-1, now));
    ++steps;
  }

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
  // Match ATS Mini Dev's persistent acceleration ramp rather than recalculating
  // the factor downward on every detent.
  constexpr uint32_t speedThresholds[] = {350, 60, 45, 35, 25};
  constexpr uint8_t accelFactors[] = {1, 2, 4, 8, 16};

  const uint32_t elapsed = now - lastRotationMs_;
  smoothedRotationMs_ = (elapsed * 7U + smoothedRotationMs_ * 3U) / 10U;

  if (smoothedRotationMs_ > speedThresholds[0] ||
      lastRotationDirection_ != direction) {
    smoothedRotationMs_ = speedThresholds[0];
    lastAccelFactor_ = accelFactors[0];
  } else {
    for (int8_t i = 4; i >= 0; --i) {
      if (smoothedRotationMs_ <= speedThresholds[i] &&
          lastAccelFactor_ < accelFactors[i]) {
        lastAccelFactor_ = accelFactors[i];
        break;
      }
    }
  }

  lastRotationMs_ = now;
  lastRotationDirection_ = direction;
  return static_cast<int16_t>(direction * lastAccelFactor_);
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
