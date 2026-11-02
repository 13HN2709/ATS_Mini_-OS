#pragma once

// Non-destructive LittleFS access using the same partition label and mount
// options as ats-mini-dev. This module never formats or deletes files.
namespace LittleFsStore {
bool begin();
bool isReady();
}  // namespace LittleFsStore
