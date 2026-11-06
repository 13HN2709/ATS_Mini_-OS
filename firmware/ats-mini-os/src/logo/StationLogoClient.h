#pragma once
#include <Arduino.h>
#include "../radio/RadioSnapshot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// RAM-only station-logo loader: FM-DX PI lookup first, Radio Browser fallback.
class StationLogoClient {
public:
  bool begin();
  void update(const RadioSnapshot& state, bool wifiConnected);
  bool lockLogo(const char* cacheKey, const uint8_t*& data, size_t& length, bool& isJpg);
  bool isLookupPending(const char* cacheKey) const;
  void unlockLogo();
private:
  static void taskEntry(void* arg);
  void runLookup(const RadioSnapshot& state);
  bool storeLogoInRam(const String& logo, const char* key, const char* source);
  volatile bool busy_ = false;
  bool haveRequested_ = false;
  char lastRequestKey_[40] = {};
  uint32_t lastAttemptMs_ = 0;
  SemaphoreHandle_t logoMutex_ = nullptr;
  uint8_t* logoData_ = nullptr;
  size_t logoLength_ = 0;
  bool logoIsJpg_ = false;
  char logoKey_[40] = {};
};
