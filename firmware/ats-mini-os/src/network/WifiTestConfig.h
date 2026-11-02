#pragma once

// Local-only Wi-Fi configuration.
// Keep real credentials out of committed source and downloadable source ZIPs.
// Define ATS_WIFI_SSID and ATS_WIFI_PASSWORD in a private build environment,
// or add a local-only config header excluded from version control.

#ifndef ATS_WIFI_SSID
#define ATS_WIFI_SSID ""
#endif

#ifndef ATS_WIFI_PASSWORD
#define ATS_WIFI_PASSWORD ""
#endif

namespace WifiTestConfig {
constexpr char SSID[] = ATS_WIFI_SSID;
constexpr char PASSWORD[] = ATS_WIFI_PASSWORD;
}  // namespace WifiTestConfig
