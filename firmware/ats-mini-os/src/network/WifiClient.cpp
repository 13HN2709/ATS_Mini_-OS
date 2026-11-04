#include "WifiClient.h"

#include "WifiTestConfig.h"

void WifiClient::begin() {
  WiFi.persistent(false);
  Serial.println("[WIFI] standby; Wi-Fi is enabled only when a remote logo lookup is needed");
}

void WifiClient::update() {
  if (WifiTestConfig::SSID[0] == '\0') return;

  const wl_status_t status = WiFi.status();
  const uint32_t now = millis();

  if (status != lastStatus_) {
    lastStatus_ = status;
    switch (status) {
      case WL_CONNECTED:
        Serial.printf("[WIFI] connected; IP=%s RSSI=%d dBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("[WIFI] configured SSID not found");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("[WIFI] connection failed; auto-reconnect remains enabled");
        break;
      case WL_DISCONNECTED:
        Serial.println("[WIFI] disconnected; waiting for reconnect");
        break;
      default:
        Serial.printf("[WIFI] status changed: %d\n", static_cast<int>(status));
        break;
    }
  }

  // Keep diagnostics sparse while joining; never block the radio/UI loop.
  if (status != WL_CONNECTED && now - lastLogMs_ >= 10000) {
    lastLogMs_ = now;
    Serial.printf("[WIFI] waiting for network (status=%d)\n",
                  static_cast<int>(status));
  }
}

bool WifiClient::connected() const {
  return WifiTestConfig::SSID[0] != '\0' && WiFi.status() == WL_CONNECTED;
}
