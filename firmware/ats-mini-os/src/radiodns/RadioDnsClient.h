#pragma once
#include <Arduino.h>
#include "RadioDnsIdentity.h"
#include "RadioDnsCache.h"

// Runs blocking DNS/HTTP work on its own FreeRTOS task, never in the radio/UI loop.
class RadioDnsClient {
public:
  bool begin(RadioDnsCache* cache);
  void update(const RadioDnsFmIdentity& identity, bool wifiConnected);
  bool busy() const { return busy_; }
private:
  static void taskEntry(void* arg);
  void runLookup(const RadioDnsFmIdentity& identity);
  RadioDnsCache* cache_ = nullptr;
  volatile bool busy_ = false;
  bool haveRequested_ = false;
  char lastRequestedFqdn_[64] = {};
  uint32_t lastAttemptMs_ = 0;
};
