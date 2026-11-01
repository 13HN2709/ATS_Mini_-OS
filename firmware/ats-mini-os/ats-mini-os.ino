#include <Arduino.h>
#include "system/PerformanceManager.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
PerformanceManager performance;
uint32_t lastStatusMs = 0;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(150);
  performance.begin();

  Serial.println();
  Serial.println("ATS Mini OS - bring-up scaffold");
  Serial.println("Radio/display drivers are not wired in this phase.");
  Serial.printf("CPU profile: %s\n", performance.profileName());
}

void loop() {
  performance.update();

  const uint32_t now = millis();
  if (now - lastStatusMs >= 5000) {
    lastStatusMs = now;
    Serial.printf("[OS] profile=%s cpu=%u MHz\n",
                  performance.profileName(),
                  static_cast<unsigned>(getCpuFrequencyMhz()));
  }
  delay(10);
}
