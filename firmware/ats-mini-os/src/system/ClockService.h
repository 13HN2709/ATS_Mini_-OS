#pragma once

#include <Arduino.h>
#include <time.h>

// The system epoch is always UTC. Local conversions use Europe/Berlin rules.
class ClockService {
public:
  void begin();
  void update(bool wifiConnected);

  bool synchronized() const;
  time_t utcEpoch() const;
  bool getUtc(struct tm& out) const;
  bool getLocal(struct tm& out) const;
  bool utcToLocal(time_t utc, struct tm& out) const;
  bool localToUtc(const struct tm& local, time_t& utc) const;
  bool setUtcEpoch(time_t utc);

private:
  bool sntpStarted_ = false;
  bool lastSyncState_ = false;
  uint32_t lastStatusLogMs_ = 0;
};
