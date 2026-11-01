#include "PerformanceManager.h"

void PerformanceManager::begin() {
  // Keep the framework's safe startup clock until supported operating points
  // and power/timing behavior are verified on both OSPI and QSPI variants.
  applyPolicy();
}

void PerformanceManager::update() {
  const uint32_t now = millis();
  bool changed = false;
  for (auto &lease : leases_) {
    if (lease.active && static_cast<int32_t>(now - lease.expiresAt) >= 0) {
      lease.active = false;
      changed = true;
    }
  }
  if (changed) applyPolicy();
}

bool PerformanceManager::setProfile(CpuProfile profile) {
  if (static_cast<uint8_t>(profile) > static_cast<uint8_t>(CpuProfile::FixedHigh)) {
    return false;
  }
  selectedProfile_ = profile;
  applyPolicy();
  return true;
}

const char* PerformanceManager::profileName() const {
  switch (selectedProfile_) {
    case CpuProfile::Auto: return "Auto";
    case CpuProfile::FixedLow: return "FixedLow";
    case CpuProfile::FixedMedium: return "FixedMedium";
    case CpuProfile::FixedHigh: return "FixedHigh";
  }
  return "Unknown";
}

bool PerformanceManager::request(PerformanceReason reason, uint32_t timeoutMs) {
  const auto index = static_cast<uint8_t>(reason);
  if (index >= static_cast<uint8_t>(PerformanceReason::Count) || timeoutMs == 0) {
    return false;
  }
  // In fixed modes requests are recorded but must not override the user's clock.
  if (selectedProfile_ == CpuProfile::Auto) {
    leases_[index].active = true;
    leases_[index].expiresAt = millis() + timeoutMs;
    applyPolicy();
  }
  return true;
}

void PerformanceManager::release(PerformanceReason reason) {
  const auto index = static_cast<uint8_t>(reason);
  if (index >= static_cast<uint8_t>(PerformanceReason::Count)) return;
  if (leases_[index].active) {
    leases_[index].active = false;
    applyPolicy();
  }
}

bool PerformanceManager::anyLeaseActive() const {
  for (const auto &lease : leases_) {
    if (lease.active) return true;
  }
  return false;
}

void PerformanceManager::applyPolicy() {
  // Deliberately no setCpuFrequencyMhz() call yet. First verify supported
  // clock points and power/timing constraints for the target Arduino-ESP32
  // core. The four-mode API and lease policy are in place for integration.
  (void)anyLeaseActive();
}
