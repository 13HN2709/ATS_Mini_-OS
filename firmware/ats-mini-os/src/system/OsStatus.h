#pragma once
#include <Arduino.h>

enum class OsSubsystem : uint8_t {
  System = 0,
  Display,
  Tuner,
  Audio,
  Input,
  Network,
  Count
};

enum class OsHealth : uint8_t {
  NotImplemented = 0,
  Initializing,
  Ready,
  Degraded,
  Failed
};

struct OsStatusSnapshot {
  OsSubsystem subsystem;
  OsHealth health;
  uint32_t lastChangeMs;
  int32_t detailCode;
};

const char* osSubsystemName(OsSubsystem subsystem);
const char* osHealthName(OsHealth health);
