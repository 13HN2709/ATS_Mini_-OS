#pragma once

#include <Arduino.h>
#include <FS.h>

// Persistent RadioDNS cache on the dedicated LittleFS partition.
// All cached RadioDNS content lives under /RDS; the cache never formats a
// failed filesystem automatically, because that could destroy existing data.
class RadioDnsCache {
public:
  bool begin();
  bool ready() const { return ready_; }
  bool saveAsset(const char* cacheKey, const char* extension,
                 const uint8_t* data, size_t length);
  bool assetExists(const char* cacheKey, const char* extension) const;
  String assetPath(const char* cacheKey, const char* extension) const;
  bool saveMetadata(const char* cacheKey, const char* text);
  String readMetadata(const char* cacheKey) const;
private:
  bool ready_ = false;
  String safePath(const char* cacheKey, const char* extension) const;
};
