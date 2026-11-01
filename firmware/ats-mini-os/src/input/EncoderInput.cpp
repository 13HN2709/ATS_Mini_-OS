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
  pendingEvent_ = {};
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

  if (transitionAccumulator_ >= 4) {
    queueEvent(EncoderEventType::RotateClockwise, 1);
    transitionAccumulator_ = 0;
  } else if (transitionAccumulator_ <= -4) {
    queueEvent(EncoderEventType::RotateCounterClockwise, -1);
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

bool EncoderInput::poll(EncoderEvent& event) {
  if (pendingEvent_.type == EncoderEventType::None) {
    event = {};
    return false;
  }
  event = pendingEvent_;
  pendingEvent_ = {};
  return true;
}

void EncoderInput::queueEvent(EncoderEventType type, int16_t delta) {
  pendingEvent_.type = type;
  pendingEvent_.delta = delta;
  pendingEvent_.timestampMs = millis();
}
