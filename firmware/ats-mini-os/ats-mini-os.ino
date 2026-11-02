#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "src/radio/RadioCore.h"
#include "src/system/PerformanceManager.h"
#include "src/display/DisplayDriver.h"
#include "src/input/EncoderInput.h"
#include "src/network/WifiClient.h"
#include "src/system/PartitionDiagnostics.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEFAULT_FM_FREQUENCY_KHZ = 95400;
PerformanceManager performance;
RadioCore radio;
DisplayDriver display;
EncoderInput encoder;
WifiClient wifi;
uint32_t lastStatusMs = 0;
uint32_t encoderEventCount = 0;
const char* lastEncoderEvent = "WAIT";
}
void setup() {
  Serial.begin(SERIAL_BAUD); delay(250); performance.begin();
  Serial.println(); Serial.println("ATS Mini OS - UKW/RDS hardware test");
  Serial.println("First milestone: display, SI4735 audio, RDS. No memories/EiBi storage.");
  PartitionDiagnostics::printToSerial();

  encoder.begin();
  if (display.begin()) Serial.println("[DISPLAY] first UI initialized.");
  if (radio.begin(DEFAULT_FM_FREQUENCY_KHZ))
    Serial.printf("Radio ready: %.1f MHz\n",radio.snapshot().frequencyKHz/1000.0f);
  else Serial.println("[TUNER] init failed; check I2C, radio power and GPIO configuration.");
  wifi.begin();
  Serial.printf("CPU profile: %s\n",performance.profileName());
}
void loop() {
  performance.update();
  wifi.update();
  encoder.update();
  EncoderEvent inputEvent;
  while (encoder.poll(inputEvent)) {
    ++encoderEventCount;
    switch (inputEvent.type) {
      case EncoderEventType::RotateClockwise:
      case EncoderEventType::RotateCounterClockwise:
        lastEncoderEvent = inputEvent.type == EncoderEventType::RotateClockwise ? "CW" : "CCW";
        if (radio.nextChannel(inputEvent.delta))
          Serial.printf("[INPUT] tune step %s -> %.1f MHz\n",
            inputEvent.delta > 0 ? "CW" : "CCW",
            radio.snapshot().frequencyKHz / 1000.0f);
        break;
      case EncoderEventType::ShortPress:
        lastEncoderEvent = "SHORT";
        Serial.println("[INPUT] short press");
        break;
      case EncoderEventType::LongPress:
        lastEncoderEvent = "LONG";
        Serial.println("[INPUT] long press");
        break;
      case EncoderEventType::None:
      default:
        break;
    }
  }
  radio.update();
  display.render(radio.snapshot(),radio.isReady(),lastEncoderEvent,encoderEventCount);
  const uint32_t now=millis();
  if(now-lastStatusMs>=2000) {
    lastStatusMs=now; const RadioSnapshot& s=radio.snapshot();
    Serial.printf("[RADIO] ready=%s freq=%.1f MHz RSSI=%d SNR=%d stereo=%s cpu=%u MHz\n",
      radio.isReady()?"yes":"no",s.frequencyKHz/1000.0f,s.rssi,s.snr,
      s.stereo?"yes":"no",static_cast<unsigned>(getCpuFrequencyMhz()));
    Serial.printf("[RDS] available=%s PI=%04X PTY=%u PS=\"%s\" RT=\"%s\"\n",
      s.rdsAvailable?"yes":"no",static_cast<unsigned>(s.piCode),
      static_cast<unsigned>(s.pty),s.stationName,s.radioText);
  }
  delay(2);
}
