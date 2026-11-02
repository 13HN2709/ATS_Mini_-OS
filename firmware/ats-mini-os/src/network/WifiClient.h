#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WifiClient {
public:
  void begin();
  void update();
  bool connected() const;

private:
  wl_status_t lastStatus_ = WL_IDLE_STATUS;
  uint32_t lastLogMs_ = 0;
};
