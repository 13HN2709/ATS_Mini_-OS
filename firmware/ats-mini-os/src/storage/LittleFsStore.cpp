#include "LittleFsStore.h"

#include <Arduino.h>
#include <LittleFS.h>

namespace LittleFsStore {
namespace {
bool ready = false;
}

bool begin() {
  if (ready) return true;

  // Match the working ats-mini-dev mount configuration, but explicitly
  // disable format-on-failure to protect existing EiBi/memory files.
  ready = LittleFS.begin(false, "/littlefs", 10, "littlefs");
  if (ready) {
    Serial.println("[STORAGE] LittleFS mounted read/write; auto-format is disabled.");
  } else {
    Serial.println("[STORAGE] LittleFS mount failed; no format, erase, or delete attempted.");
  }
  return ready;
}

bool isReady() {
  return ready;
}
}  // namespace LittleFsStore
