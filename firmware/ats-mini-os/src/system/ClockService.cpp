#include "ClockService.h"

#include <sys/time.h>
#include <stdlib.h>

namespace {
constexpr char kTimezone[] = "CET-1CEST,M3.5.0/2,M10.5.0/3";
constexpr char kNtpServer1[] = "pool.ntp.org";
constexpr char kNtpServer2[] = "time.google.com";
constexpr char kNtpServer3[] = "time.cloudflare.com";
constexpr time_t kValidEpochFloor = 1700000000; // Reject an unset/default system clock.
}

void ClockService::begin() {
  // Keep the system epoch in UTC; localtime_r/mktime apply the timezone.
  setenv("TZ", kTimezone, 1);
  tzset();
  sntpStarted_ = false;
  lastSyncState_ = synchronized();
  Serial.println("[CLOCK] UTC system time; Europe/Berlin local time with automatic CET/CEST");
}

void ClockService::update(bool wifiConnected) {
  if (wifiConnected && !sntpStarted_) {
    // SNTP runs asynchronously and disciplines the system UTC clock.
    configTzTime(kTimezone, kNtpServer1, kNtpServer2, kNtpServer3);
    sntpStarted_ = true;
    Serial.println("[CLOCK] SNTP started; waiting for UTC synchronization");
  }

  const bool synced = synchronized();
  if (synced != lastSyncState_) {
    lastSyncState_ = synced;
    Serial.printf("[CLOCK] UTC synchronization %s\n", synced ? "available" : "unavailable");
  }

  const uint32_t now = millis();
  if (wifiConnected && !synced && now - lastStatusLogMs_ >= 30000) {
    lastStatusLogMs_ = now;
    Serial.println("[CLOCK] waiting for NTP time");
  }
}

bool ClockService::synchronized() const {
  return time(nullptr) >= kValidEpochFloor;
}

time_t ClockService::utcEpoch() const {
  return synchronized() ? time(nullptr) : 0;
}

bool ClockService::getUtc(struct tm& out) const {
  const time_t now = utcEpoch();
  return now && gmtime_r(&now, &out) != nullptr;
}

bool ClockService::getLocal(struct tm& out) const {
  const time_t now = utcEpoch();
  return now && localtime_r(&now, &out) != nullptr;
}

bool ClockService::utcToLocal(time_t utc, struct tm& out) const {
  return utc >= kValidEpochFloor && localtime_r(&utc, &out) != nullptr;
}

bool ClockService::localToUtc(const struct tm& local, time_t& utc) const {
  struct tm fields = local;
  fields.tm_isdst = -1; // Resolve CET/CEST from the timezone rules.
  const time_t converted = mktime(&fields);
  if (converted < kValidEpochFloor) return false;
  utc = converted;
  return true;
}

bool ClockService::setUtcEpoch(time_t utc) {
  if (utc < kValidEpochFloor) return false;
  timeval value{};
  value.tv_sec = utc;
  value.tv_usec = 0;
  return settimeofday(&value, nullptr) == 0;
}
