#pragma once

// Temporary credentials for the first RadioDNS connectivity test only.
// This file belongs to the private ATS Mini OS development repository.
// Do not copy it into a public repository or publish firmware containing
// these credentials. Replace with a settings/configuration page later.
// Keep namespace-scope const arrays (internal linkage) for Arduino toolchains
// that compile sketch sources with the GNU++11 language standard.

namespace WifiTestConfig {
constexpr char SSID[] = "S22";
constexpr char PASSWORD[] = "12345678";
}  // namespace WifiTestConfig
