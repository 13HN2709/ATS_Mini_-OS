#pragma once
#include <Arduino.h>

enum class CpuProfile : uint8_t {
  Auto = 0,
  FixedLow,
  FixedMedium,
  FixedHigh
};

enum class PerformanceReason : uint8_t {
  RadioDns = 0,
  ImageDecode,
  SpectrumScan,
  UiAnimation,
  Count
};

// Central owner of CPU profile policy. UI/services must request temporary
// performance through this interface rather than changing the clock directly.
class PerformanceManager {
public:
  void begin();
  void update();

  bool setProfile(CpuProfile profile);
  CpuProfile profile() const { return selectedProfile_; }
  const char* profileName() const;

  // Temporary leases support overlapping work. Every successful request must
  // be released; update() also expires leases to prevent a stuck boost.
  bool request(PerformanceReason reason, uint32_t timeoutMs);
  void release(PerformanceReason reason);

private:
  struct Lease {
    bool active = false;
    uint32_t expiresAt = 0;
  };

  CpuProfile selectedProfile_ = CpuProfile::Auto;
  Lease leases_[static_cast<uint8_t>(PerformanceReason::Count)]{};

  void applyPolicy();
  bool anyLeaseActive() const;
};
