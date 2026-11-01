#include <Arduino.h>
#include "src/radio/RadioCore.h"
#include "src/system/PerformanceManager.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEFAULT_FM_FREQUENCY_KHZ = 103900;

PerformanceManager performance;
RadioCore radio;
uint32_t lastStatusMs = 0;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(250);
  performance.begin();

  Serial.println();
  Serial.println("ATS Mini OS - modular FM bring-up");
  Serial.println("No display or encoder is enabled in this test phase.");

  if (radio.begin(DEFAULT_FM_FREQUENCY_KHZ)) {
    Serial.printf("Radio ready: %.1f MHz\n",
                  radio.snapshot().frequencyKHz / 1000.0f);
  } else {
    Serial.println("Radio init failed; tuner module reports not ready.");
  }
  Serial.printf("CPU profile: %s\n", performance.profileName());
}

void loop() {
  performance.update();
  radio.update();

  const uint32_t now = millis();
  if (now - lastStatusMs >= 2000) {
    lastStatusMs = now;
    const RadioSnapshot& state = radio.snapshot();
    Serial.printf("[RADIO] ready=%s freq=%.1f MHz RSSI=%d SNR=%d stereo=%s cpu=%u MHz\n",
                  radio.isReady() ? "yes" : "no",
                  state.frequencyKHz / 1000.0f,
                  state.rssi,
                  state.snr,
                  state.stereo ? "yes" : "no",
                  static_cast<unsigned>(getCpuFrequencyMhz()));
  }
  delay(10);
}
