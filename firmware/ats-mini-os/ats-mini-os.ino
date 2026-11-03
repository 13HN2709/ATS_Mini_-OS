#include <Arduino.h>
#include <WebServer.h>
#include <LovyanGFX.hpp>
#include "src/radio/RadioCore.h"
#include "src/system/PerformanceManager.h"
#include "src/display/DisplayDriver.h"
#include "src/input/EncoderInput.h"
#include "src/network/WifiClient.h"
#include "src/radiodns/RadioDnsCache.h"
#include "src/radiodns/RadioDnsClient.h"
#include "src/system/PartitionDiagnostics.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEFAULT_FM_FREQUENCY_KHZ = 95400;
PerformanceManager performance;
RadioCore radio;
DisplayDriver display;
EncoderInput encoder;
WifiClient wifi;
RadioDnsCache radioDnsCache;
RadioDnsClient radioDnsClient;
WebServer diagnosticsServer(80);
bool diagnosticsServerStarted = false;
uint32_t lastStatusMs = 0;
uint32_t encoderEventCount = 0;
const char* lastEncoderEvent = "WAIT";
}
void setup() {
  Serial.begin(SERIAL_BAUD); delay(250); performance.begin();
  Serial.println(); Serial.println("ATS Mini OS - UKW/RDS hardware test");
  Serial.println("First milestone: display, SI4735 audio, RDS. No memories/EiBi storage.");
  PartitionDiagnostics::printToSerial();
  radioDnsCache.begin();
  radioDnsClient.begin(&radioDnsCache);

  encoder.begin();
  if (display.begin()) Serial.println("[DISPLAY] first UI initialized.");
  if (radio.begin(DEFAULT_FM_FREQUENCY_KHZ))
    Serial.printf("Radio ready: %.1f MHz\n",radio.snapshot().frequencyKHz/1000.0f);
  else Serial.println("[TUNER] init failed; check I2C, radio power and GPIO configuration.");
  wifi.begin();
  diagnosticsServer.on("/radiodns-log", HTTP_GET, []() {
    const String log = radioDnsCache.readLog();
    diagnosticsServer.sendHeader("Content-Disposition", "attachment; filename=ATS-Mini-OS-RadioDNS-log.txt");
    diagnosticsServer.send(200, "text/plain; charset=utf-8", log.length() ? log : "RadioDNS log is empty or LittleFS is unavailable.\n");
  });
  diagnosticsServer.on("/radiodns-status", HTTP_GET, []() {
    const RadioSnapshot& state = radio.snapshot();
    String body = String("ATS Mini OS RadioDNS status\nFrequency kHz: ") + String(state.frequencyKHz) + "\nPI: " + String(state.piCode, HEX) + "\nECC valid: " + (state.eccValid ? "yes" : "no") + "\nGCC: " + String(state.gcc) + "\nWiFi connected: " + (wifi.connected() ? "yes" : "no") + "\nLittleFS ready: " + (radioDnsCache.ready() ? "yes" : "no") + "\n";
    diagnosticsServer.send(200, "text/plain; charset=utf-8", body);
  });
  Serial.printf("CPU profile: %s\n",performance.profileName());
}
void loop() {
  performance.update();
  wifi.update();
  if (wifi.connected() && !diagnosticsServerStarted) {
    diagnosticsServer.begin();
    diagnosticsServerStarted = true;
    Serial.printf("[DIAG] RadioDNS log download: http://%s/radiodns-log\\n", WiFi.localIP().toString().c_str());
    Serial.printf("[DIAG] RadioDNS status: http://%s/radiodns-status\\n", WiFi.localIP().toString().c_str());
  }
  if (diagnosticsServerStarted) diagnosticsServer.handleClient();
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
  radioDnsClient.update(radio.radioDnsIdentity(), wifi.connected());
  display.render(radio.snapshot(),radio.isReady(),wifi.connected(),lastEncoderEvent,encoderEventCount);
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
