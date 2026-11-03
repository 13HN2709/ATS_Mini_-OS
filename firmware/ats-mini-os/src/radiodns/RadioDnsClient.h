#pragma once
#include <Arduino.h>
#include "RadioDnsIdentity.h"
#include "RadioDnsCache.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Runs blocking DNS/HTTP work on its own FreeRTOS task, never in the radio/UI loop.
class RadioDnsClient {
public:
  bool begin(RadioDnsCache* cache);
  void update(const RadioDnsFmIdentity& identity, bool wifiConnected);
  bool busy() const { return busy_; }
  bool lockLogo(const char* cacheKey, const uint8_t*& data, size_t& length, bool& isJpg);
  void unlockLogo();
private:
  static void taskEntry(void* arg);
  void runLookup(const RadioDnsFmIdentity& identity);
  RadioDnsCache* cache_ = nullptr;
  volatile bool busy_ = false;
  bool haveRequested_ = false;
  char lastRequestedFqdn_[64] = {};
  uint32_t lastAttemptMs_ = 0;
  SemaphoreHandle_t logoMutex_ = nullptr;
  uint8_t* logoData_ = nullptr;
  size_t logoLength_ = 0;
  bool logoIsJpg_ = false;
  char logoKey_[32] = {};
};
