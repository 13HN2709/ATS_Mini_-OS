#pragma once

#include <Arduino.h>

// Pure FM RadioDNS identity builder. This module performs no network or UI work.
// frequencyKHz uses the RadioCore convention (e.g. 103900 for 103.9 MHz).
enum class RadioDnsIdentityStatus : uint8_t {
  InvalidFrequency,
  InvalidPi,
  InvalidGcc,
  Ready
};

struct RadioDnsFmIdentity {
  RadioDnsIdentityStatus status = RadioDnsIdentityStatus::InvalidFrequency;
  char frequency[6] = {};
  char pi[5] = {};
  char gcc[4] = {};
  char fqdn[64] = {};
  char serviceIdentifier[32] = {};
  char bearer[32] = {};
};

class RadioDnsIdentity {
public:
  // GCC must be supplied by a validated source/fallback; this function never
  // guesses a country from the device locale or a fixed regional setting.
  static RadioDnsFmIdentity buildFm(uint32_t frequencyKHz,
                                    uint16_t piCode,
                                    const char* gcc);
};
