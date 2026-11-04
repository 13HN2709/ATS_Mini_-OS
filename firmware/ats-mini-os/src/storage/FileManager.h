#pragma once

#include <Arduino.h>
#include <FS.h>

namespace FileManager {
  constexpr uint8_t MAX_ENTRIES = 24;
  struct Entry {
    char name[48];
    char path[96];
    bool directory;
    size_t size;
  };

  bool begin();
  bool ready();
  bool list(const char* directory, Entry* entries, uint8_t capacity, uint8_t& count);
  bool makeDirectory(const char* path);
  bool copyFile(const char* source, const char* destination);
  bool moveFile(const char* source, const char* destination);
  bool renamePath(const char* source, const char* destination);
  bool removeFile(const char* path);
  bool removeEmptyDirectory(const char* path);
}
