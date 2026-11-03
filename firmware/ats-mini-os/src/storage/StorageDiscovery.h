#pragma once

#include <Arduino.h>

// Discovers supported data filesystems from the runtime partition table.
// Existing contents are inspected read-only. No format/erase operation is used.
// The current ATS Mini OS partition table provides a LittleFS data partition.
namespace StorageDiscovery {
  // Mounts a supported filesystem without formatting, searches for known
  // existing data files, and creates /RDS only after an anchor file is found.
  bool begin();
  bool ready();
  const char* partitionLabel();
  const char* anchorPath();
}
