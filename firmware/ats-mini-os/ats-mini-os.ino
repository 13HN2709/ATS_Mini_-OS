#include <Arduino.h>
#include <WebServer.h>
#include <LovyanGFX.hpp>
#include "src/radio/RadioCore.h"
#include "src/system/PerformanceManager.h"
#include "src/display/DisplayDriver.h"
#include "src/input/EncoderInput.h"
#include "src/network/WifiClient.h"
#include "src/logo/StationLogoClient.h"
#include "src/system/PartitionDiagnostics.h"
#include "src/storage/StorageDiscovery.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEFAULT_FM_FREQUENCY_KHZ = 95400;
PerformanceManager performance;
RadioCore radio;
DisplayDriver display;
EncoderInput encoder;
WifiClient wifi;
StationLogoClient stationLogoClient;
WebServer diagnosticsServer(80);
bool diagnosticsServerStarted = false;
uint32_t lastStatusMs = 0;
uint32_t encoderEventCount = 0;
const char* lastEncoderEvent = "WAIT";
bool menuOpen = false;
uint8_t menuPage = 0;
uint8_t menuSelection = 0;
}
void setup() {
  Serial.begin(SERIAL_BAUD); delay(250); performance.begin();
  Serial.println(); Serial.println("ATS Mini OS - UKW/RDS hardware test");
  Serial.println("First milestone: display, SI4735 audio, RDS. No memories/EiBi storage.");
  PartitionDiagnostics::printToSerial();
  StorageDiscovery::begin();
  stationLogoClient.begin();

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
  if (wifi.connected() && !diagnosticsServerStarted) {
    diagnosticsServer.begin();
    diagnosticsServerStarted = true;
    Serial.printf("[DIAG] Web status server: http://%s/ logo status only\\n", WiFi.localIP().toString().c_str());
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
        if (menuOpen) {
          if (menuPage == 1 && menuSelection == 1) {
            const int16_t requested = static_cast<int16_t>(radio.volume()) + inputEvent.delta;
            const uint8_t nextVolume = static_cast<uint8_t>(constrain(requested, 0, 63));
            if (radio.setVolume(nextVolume))
              Serial.printf("[MENU] volume=%u/63\n", static_cast<unsigned>(radio.volume()));
          } else {
            const uint8_t itemCount = menuPage == 0 ? 6 : 3;
            int16_t next = static_cast<int16_t>(menuSelection) + (inputEvent.delta > 0 ? 1 : -1);
            if (next < 0) next = itemCount - 1;
            if (next >= itemCount) next = 0;
            menuSelection = static_cast<uint8_t>(next);
          }
        } else if (radio.nextChannel(inputEvent.delta)) {
          Serial.printf("[INPUT] tune step %s -> %.1f MHz\n",
            inputEvent.delta > 0 ? "CW" : "CCW",
            radio.snapshot().frequencyKHz / 1000.0f);
        }
        break;
      case EncoderEventType::ShortPress:
        lastEncoderEvent = "SHORT";
        if (!menuOpen) {
          menuOpen = true;
          menuPage = 0;
          menuSelection = 0;
          Serial.println("[MENU] opened");
        } else if (menuPage == 0) {
          if (menuSelection == 0) {
            menuOpen = false;
            Serial.println("[MENU] closed via Back");
          } else {
            menuPage = menuSelection;
            menuSelection = 0;
            Serial.printf("[MENU] opened submenu %u\n", static_cast<unsigned>(menuPage));
          }
        } else if (menuSelection == 0) {
          menuPage = 0;
          menuSelection = 0;
        } else if (menuPage == 1 && menuSelection == 1) {
          menuPage = 0;
          menuSelection = 1;
        }
        break;
      case EncoderEventType::LongPress:
        lastEncoderEvent = "LONG";
        if (menuOpen) {
          menuOpen = false;
          menuPage = 0;
          menuSelection = 0;
          Serial.println("[MENU] closed via long press");
        } else {
          Serial.println("[INPUT] long press (no action)");
        }
        break;
      case EncoderEventType::None:
      default:
        break;
    }
  }
  radio.update();
  stationLogoClient.update(radio.snapshot(), wifi.connected());
  display.render(radio.snapshot(),radio.isReady(),wifi.connected(),lastEncoderEvent,encoderEventCount,&stationLogoClient,menuOpen,menuPage,menuSelection,radio.volume());
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
