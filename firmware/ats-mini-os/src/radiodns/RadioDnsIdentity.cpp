#include "RadioDnsIdentity.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

namespace {
bool isHexString(const char* value, size_t requiredLength) {
  if (!value || strlen(value) != requiredLength) return false;
  for (size_t i = 0; i < requiredLength; ++i) {
    if (!isxdigit(static_cast<unsigned char>(value[i]))) return false;
  }
  return true;
}

void copyUpperHex(char* dst, const char* src, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    dst[i] = static_cast<char>(toupper(static_cast<unsigned char>(src[i])));
  }
  dst[length] = '\0';
}
}

RadioDnsFmIdentity RadioDnsIdentity::buildFm(uint32_t frequencyKHz,
                                             uint16_t piCode,
                                             const char* gcc) {
  RadioDnsFmIdentity result{};

  // FM frequency must be represented in 100-kHz units and rendered as five
  // decimal digits (e.g. 103900 kHz -> "10390", 87500 kHz -> "08750").
  if (frequencyKHz < 87500 || frequencyKHz > 108000 ||
      frequencyKHz % 100 != 0) {
    result.status = RadioDnsIdentityStatus::InvalidFrequency;
    return result;
  }

  if (piCode == 0) {
    result.status = RadioDnsIdentityStatus::InvalidPi;
    return result;
  }

  if (!isHexString(gcc, 3)) {
    result.status = RadioDnsIdentityStatus::InvalidGcc;
    return result;
  }

  const uint32_t frequencyUnits = frequencyKHz / 10;
  snprintf(result.frequency, sizeof(result.frequency), "%05lu",
           static_cast<unsigned long>(frequencyUnits));
  snprintf(result.pi, sizeof(result.pi), "%04X", static_cast<unsigned>(piCode));
  copyUpperHex(result.gcc, gcc, 3);

  snprintf(result.fqdn, sizeof(result.fqdn), "%s.%s.%s.fm.radiodns.org",
           result.frequency, result.pi, result.gcc);
  snprintf(result.serviceIdentifier, sizeof(result.serviceIdentifier),
           "fm/%s/%s/%s", result.gcc, result.pi, result.frequency);
  snprintf(result.bearer, sizeof(result.bearer), "fm:%s.%s.%s",
           result.gcc, result.pi, result.frequency);
  result.status = RadioDnsIdentityStatus::Ready;
  return result;
}
