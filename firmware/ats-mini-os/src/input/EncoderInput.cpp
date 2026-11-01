#include "EncoderInput.h"

void EncoderInput::begin() {
  // No GPIO configured in the scaffold phase.
}

void EncoderInput::update() {
  // The concrete ATS Mini V4 input driver is not implemented yet.
}

bool EncoderInput::poll(EncoderEvent& event) {
  event = {};
  return false;
}
