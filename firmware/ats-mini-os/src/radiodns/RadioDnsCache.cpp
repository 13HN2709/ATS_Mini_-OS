#include "RadioDnsCache.h"

#include <LittleFS.h>
#include "../storage/StorageDiscovery.h"
#include <ctype.h>
#include <string.h>

namespace {
bool isSafeKey(const char* key) {
  if (!key || !*key) return false;
  for (const char* p = key; *p; ++p) {
    const unsigned char ch = static_cast<unsigned char>(*p);
    if (!(isalnum(ch) || ch == '-' || ch == '_')) return false;
  }
  return true;
}
bool isSafeExtension(const char* ext) {
  if (!ext || !*ext) return false;
  for (const char* p = ext; *p; ++p) {
    if (!isalnum(static_cast<unsigned char>(*p))) return false;
  }
  return true;
}
}

bool RadioDnsCache::begin() {
  // StorageDiscovery mounts without formatting, searches for existing anchor
  // files, and creates /RDS only on the filesystem where an anchor was found.
  if (!StorageDiscovery::begin()) {
    Serial.println("[RADIODNS] storage not selected; cache disabled without writes");
    ready_ = false;
    return false;
  }
  ready_ = true;
  Serial.printf("[RADIODNS] cache ready at /RDS on partition '%s' (anchor: %s)\n",
                StorageDiscovery::partitionLabel(), StorageDiscovery::anchorPath());
  return true;
}

String RadioDnsCache::safePath(const char* cacheKey, const char* extension) const {
  if (!isSafeKey(cacheKey) || !isSafeExtension(extension)) return String();
  String path = "/RDS/";
  path += cacheKey;
  path += '.';
  path += extension;
  return path;
}
String RadioDnsCache::assetPath(const char* cacheKey, const char* extension) const {
  return safePath(cacheKey, extension);
}
bool RadioDnsCache::assetExists(const char* cacheKey, const char* extension) const {
  if (!ready_) return false;
  const String path = safePath(cacheKey, extension);
  return path.length() && LittleFS.exists(path);
}
bool RadioDnsCache::saveAsset(const char* cacheKey, const char* extension,
                              const uint8_t* data, size_t length) {
  if (!ready_ || !data || length == 0) return false;
  const String path = safePath(cacheKey, extension);
  if (!path.length()) return false;
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.printf("[RADIODNS] cannot open cache file: %s\n", path.c_str());
    return false;
  }
  const size_t written = file.write(data, length);
  file.close();
  if (written != length) {
    Serial.printf("[RADIODNS] incomplete cache write: %s (%u/%u bytes)\n",
                  path.c_str(), static_cast<unsigned>(written),
                  static_cast<unsigned>(length));
    LittleFS.remove(path);
    return false;
  }
  Serial.printf("[RADIODNS] cached %s (%u bytes)\n",
                path.c_str(), static_cast<unsigned>(length));
  return true;
}
bool RadioDnsCache::saveMetadata(const char* cacheKey, const char* text) {
  if (!ready_ || !text) return false;
  const String path = safePath(cacheKey, "txt");
  if (!path.length()) return false;
  File file = LittleFS.open(path, "w");
  if (!file) return false;
  const size_t length = strlen(text);
  const size_t written = file.write(reinterpret_cast<const uint8_t*>(text), length);
  file.close();
  if (written != length) {
    LittleFS.remove(path);
    return false;
  }
  return true;
}
String RadioDnsCache::readMetadata(const char* cacheKey) const {
  if (!ready_) return String();
  const String path = safePath(cacheKey, "txt");
  if (!path.length()) return String();
  File file = LittleFS.open(path, "r");
  if (!file) return String();
  const String result = file.readString();
  file.close();
  return result;
}


bool RadioDnsCache::appendLog(const char* line) {
  if (!ready_ || !line) return false;
  File file = LittleFS.open("/RDS/radiodns-debug.log", "a");
  if (!file) return false;
  const size_t length = strlen(line);
  const size_t written = file.write(reinterpret_cast<const uint8_t*>(line), length);
  file.close();
  return written == length;
}

String RadioDnsCache::readLog() const {
  if (!ready_) return String();
  File file = LittleFS.open("/RDS/radiodns-debug.log", "r");
  if (!file) return String();
  String result = file.readString();
  file.close();
  return result;
}

void RadioDnsCache::clearLog() {
  if (ready_ && LittleFS.exists("/RDS/radiodns-debug.log"))
    LittleFS.remove("/RDS/radiodns-debug.log");
}
