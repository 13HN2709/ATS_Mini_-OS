#include "OsStatus.h"

const char* osSubsystemName(OsSubsystem subsystem) {
  switch (subsystem) {
    case OsSubsystem::System: return "System";
    case OsSubsystem::Display: return "Display";
    case OsSubsystem::Tuner: return "Tuner";
    case OsSubsystem::Audio: return "Audio";
    case OsSubsystem::Input: return "Input";
    case OsSubsystem::Network: return "Network";
    case OsSubsystem::Count: break;
  }
  return "Unknown";
}

const char* osHealthName(OsHealth health) {
  switch (health) {
    case OsHealth::NotImplemented: return "NotImplemented";
    case OsHealth::Initializing: return "Initializing";
    case OsHealth::Ready: return "Ready";
    case OsHealth::Degraded: return "Degraded";
    case OsHealth::Failed: return "Failed";
  }
  return "Unknown";
}
