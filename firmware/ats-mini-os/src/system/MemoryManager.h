#pragma once

#include <Arduino.h>

namespace MemoryManager {
struct Snapshot {
  uint32_t psramTotal;
  uint32_t psramFree;
  uint32_t psramLargest;
  uint32_t psramMinimumFree;
  uint32_t internalTotal;
  uint32_t internalFree;
  uint32_t internalLargest;
  uint32_t internalMinimumFree;
  uint32_t allocationFailures;
  uint32_t sampleUptimeMs;
  bool psramAvailable;
  bool logReady;
};

// Starts telemetry after StorageDiscovery has mounted the existing LittleFS.
// No format, erase, or second filesystem mount is performed.
bool begin();
void update();
Snapshot snapshot();
void recordAllocationFailure(const char* module, size_t bytes);
const char* resetReasonName();
bool logAvailable();
}  // namespace MemoryManager
