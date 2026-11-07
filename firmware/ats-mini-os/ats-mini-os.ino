#include <Arduino.h>
#include <WebServer.h>
#include <LovyanGFX.hpp>
#include <LittleFS.h>
#include "src/radio/RadioCore.h"
#include "src/system/PerformanceManager.h"
#include "src/system/ClockService.h"
#include "src/display/DisplayDriver.h"
#include "src/input/EncoderInput.h"
#include "src/network/WifiClient.h"
#include "src/logo/StationLogoClient.h"
#include "src/system/PartitionDiagnostics.h"
#include "src/storage/StorageDiscovery.h"
#include "src/storage/FileManager.h"
#include "src/settings/SettingsStore.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t DEFAULT_FM_FREQUENCY_KHZ = 95400;
PerformanceManager performance;
ClockService systemClock;
RadioCore radio;
DisplayDriver display;
SettingsStore settingsStore;
bool brightnessSavePending = false;
bool themeSavePending = false;
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
FileManager::Entry fmEntries[FileManager::MAX_ENTRIES];
char fmPath[96] = "/";
char fmMessage[40] = "LittleFS bereit";
char fmSelectedPath[96] = "";
char fmSelectedName[48] = "";
uint8_t fmCount = 0, fmSelection = 0, fmActionSelection = 0, fmMode = 0;
uint8_t fmPendingAction = 0, fmEditCursor = 0;
char fmEditName[48] = "";
void saveBrightnessIfPending() {
  if (!brightnessSavePending) return;
  if (settingsStore.saveDisplayBrightness(display.brightness())) {
    Serial.printf("[SETTINGS] display brightness saved: %u/255\n",
      static_cast<unsigned>(display.brightness()));
  } else {
    Serial.println("[SETTINGS] display brightness save failed; current session remains active.");
  }
  brightnessSavePending = false;
}
void saveThemeIfPending() {
  if (!themeSavePending) return;
  if (settingsStore.saveDisplayTheme(display.themeIndex())) {
    Serial.printf("[SETTINGS] display theme saved: %s\n", ThemeManager::at(display.themeIndex()).name);
  } else {
    Serial.println("[SETTINGS] display theme save failed; current session remains active.");
  }
  themeSavePending = false;
}
void fmRefresh(){ fmCount=0; if(!FileManager::list(fmPath,fmEntries,FileManager::MAX_ENTRIES,fmCount)) snprintf(fmMessage,sizeof(fmMessage),"Ordner nicht lesbar"); }
void fmMessageSet(const char* m){ strncpy(fmMessage,m,sizeof(fmMessage)-1); fmMessage[sizeof(fmMessage)-1]=0; }
void fmParent(){ if(strcmp(fmPath,"/")==0)return; char* p=strrchr(fmPath,47); if(p==fmPath)fmPath[1]=0; else if(p)*p=0; fmSelection=0; fmRefresh(); }
void fmOpen(){ fmMode=0; fmSelection=0; fmActionSelection=0; fmMessageSet(""); fmRefresh(); menuPage=6; menuSelection=0; }
void fmChoose(uint8_t visual){ if(visual==0){ if(fmMode==3)return; fmParent(); return; } if(fmMode==0 && visual==fmCount+1){fmPendingAction=5;fmEditName[0]=0;fmEditCursor=0;fmMode=4;return;} if(fmMode==0 && visual==fmCount+2){menuOpen=false;menuPage=0;menuSelection=0;fmMessageSet("Dateimanager beendet");return;} uint8_t i=visual-1; if(i>=fmCount)return; if(fmEntries[i].directory){strncpy(fmPath,fmEntries[i].path,sizeof(fmPath)-1);fmPath[sizeof(fmPath)-1]=0;fmSelection=0;fmRefresh();} else {strncpy(fmSelectedPath,fmEntries[i].path,sizeof(fmSelectedPath)-1);fmSelectedPath[sizeof(fmSelectedPath)-1]=0;strncpy(fmSelectedName,fmEntries[i].name,sizeof(fmSelectedName)-1);fmSelectedName[sizeof(fmSelectedName)-1]=0;fmMode=1;fmActionSelection=0;} }
}
void setup() {
  Serial.begin(SERIAL_BAUD); delay(250); performance.begin();
  systemClock.begin();
  Serial.println(); Serial.println("ATS Mini OS - UKW/RDS hardware test");
  Serial.println("First milestone: display, SI4735 audio, RDS. No memories/EiBi storage.");
  PartitionDiagnostics::printToSerial();
  StorageDiscovery::begin();
  FileManager::begin();
  stationLogoClient.begin();

  encoder.begin();
  const bool settingsReady = settingsStore.begin();
  Serial.printf("[SETTINGS] dedicated NVS %s\n", settingsReady ? "ready" : "unavailable; using defaults");
  const uint8_t savedBrightness = settingsStore.displayBrightness(180);
  const uint8_t savedTheme = settingsStore.displayTheme(0);
  if (display.begin(savedBrightness)) {
    display.setTheme(savedTheme);
    Serial.printf("[DISPLAY] first UI initialized; brightness=%u/255\n",
      static_cast<unsigned>(display.brightness()));
  }
  if (radio.begin(DEFAULT_FM_FREQUENCY_KHZ)) {
    const bool agcAuto = settingsStore.agcAutomatic(true);
    const uint8_t agcIndex = settingsStore.agcGainIndex(20);
    const uint8_t softMuteDb = settingsStore.softMuteAttenuationDb(8);
    const uint8_t avcGain = settingsStore.avcGain(90);
    if (agcAuto) radio.setAgcAutomatic(true);
    else radio.setAgcManualGain(agcIndex);
    radio.setSoftMuteAttenuation(softMuteDb);
    radio.setAvcGain(avcGain); // Remembered in FM, applied automatically in AM/SSB.
    Serial.printf("[SETTINGS] RX restored: AGC=%s/%u SoftMute=%u dB AVC=%u\n",
      agcAuto ? "AUTO" : "MANUAL", static_cast<unsigned>(agcIndex),
      static_cast<unsigned>(softMuteDb), static_cast<unsigned>(avcGain));
    Serial.printf("Radio ready: %.1f MHz\n",radio.snapshot().frequencyKHz/1000.0f);
  } else Serial.println("[TUNER] init failed; check I2C, radio power and GPIO configuration.");
  wifi.begin();
  Serial.printf("CPU profile: %s\n",performance.profileName());
}
void loop() {
  performance.update();
  wifi.update();
  systemClock.update(wifi.connected());
  if (wifi.connected() && !diagnosticsServerStarted) {
    diagnosticsServer.begin();
    diagnosticsServerStarted = true;
    Serial.printf("[DIAG] Web status server: http://%s/ logo status only\n", WiFi.localIP().toString().c_str());
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
          if(menuPage==6){ if(fmMode==1){int v=(int)fmActionSelection+inputEvent.delta;v%=6;if(v<0)v+=6;fmActionSelection=(uint8_t)v;} else if(fmMode==2)fmActionSelection=fmActionSelection?0:1; else if(fmMode==4){const char* chars="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 _-.";size_t n=strlen(chars);char ch=fmEditName[fmEditCursor];const char* q=strchr(chars,ch?ch:65);int pos=q?(int)(q-chars):0;pos=(pos+inputEvent.delta)%(int)n;if(pos<0)pos+=(int)n;fmEditName[fmEditCursor]=chars[pos];fmEditName[fmEditCursor+1]=0;} else {uint8_t n=fmCount+(fmMode==3?1:3);int v=(int)fmSelection+inputEvent.delta;v%=n;if(v<0)v+=n;fmSelection=(uint8_t)v;} }
          else if (menuPage == 2 && menuSelection == 1) {
            const int16_t requested = static_cast<int16_t>(display.brightness()) + inputEvent.delta * 5;
            const uint8_t nextBrightness = static_cast<uint8_t>(constrain(requested, 10, 255));
            display.setBrightness(nextBrightness);
            brightnessSavePending = true;
            Serial.printf("[MENU] display brightness=%u/255 (pending save)\n",
              static_cast<unsigned>(display.brightness()));
          } else if (menuPage == 1 && menuSelection == 1) {
            const int16_t requested = static_cast<int16_t>(radio.volume()) + inputEvent.delta;
            const uint8_t nextVolume = static_cast<uint8_t>(constrain(requested, 0, 63));
            if (radio.setVolume(nextVolume))
              Serial.printf("[MENU] volume=%u/63\n", static_cast<unsigned>(radio.volume()));
          } else {
            const int16_t itemCount = menuPage == 0 ? 6 : (menuPage==2 ? 3 : (menuPage==10 ? 4 : (menuPage==11 ? (radio.snapshot().modulation==RadioModulation::FM ? 10 : 14) : (menuPage==5 ? 5 : (menuPage==3 ? 5 : (menuPage==7 ? radio.bandwidthChoiceCount()+1 : (menuPage==8 ? radio.bandCount()+1 : (menuPage==9 ? radio.stepChoiceCount()+1 : 3))))))));
            int16_t next = (static_cast<int16_t>(menuSelection) + inputEvent.delta) % itemCount;
            if (next < 0) next += itemCount;
            menuSelection = static_cast<uint8_t>(next);
          }
        } else if (radio.nextChannel(inputEvent.delta)) {
          display.notifyTuning();
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
        } else if(menuPage==6){ if(fmMode==1){ if(fmActionSelection==0)fmMode=0; else if(fmActionSelection==1||fmActionSelection==2){fmPendingAction=fmActionSelection;fmMode=3;fmSelection=0;} else if(fmActionSelection==4){fmMode=2;fmActionSelection=0;} else {fmPendingAction=fmActionSelection==5?5:3; if(fmPendingAction==5)fmEditName[0]=0;else strncpy(fmEditName,fmSelectedName,sizeof(fmEditName)-1);fmEditName[sizeof(fmEditName)-1]=0;fmEditCursor=0;fmMode=4;} } else if(fmMode==2){if(fmActionSelection==1){bool ok=FileManager::removeFile(fmSelectedPath);fmMessageSet(ok?"Datei geloescht":"Loeschen fehlgeschlagen");fmRefresh();}fmMode=0;} else if(fmMode==4){if(fmEditCursor<sizeof(fmEditName)-2){++fmEditCursor;if(fmEditCursor>=strlen(fmEditName)){fmEditName[fmEditCursor]=65;fmEditName[fmEditCursor+1]=0;}}} else if(fmMode==3){if(fmSelection==0){const char* b=strrchr(fmSelectedPath,47);b=b?b+1:fmSelectedPath;char dest[96];snprintf(dest,sizeof(dest),"%s%s%s",fmPath,strcmp(fmPath,"/")==0?"":"/",b);bool ok=fmPendingAction==1?FileManager::copyFile(fmSelectedPath,dest):FileManager::moveFile(fmSelectedPath,dest);fmMessageSet(ok?"Erledigt":"Aktion fehlgeschlagen");fmMode=0;fmRefresh();}else {uint8_t idx=fmSelection-1;if(idx<fmCount&&fmEntries[idx].directory){strncpy(fmPath,fmEntries[idx].path,sizeof(fmPath)-1);fmPath[sizeof(fmPath)-1]=0;fmSelection=0;fmRefresh();}}}else fmChoose(fmSelection);
        } else if (menuPage==5 && menuSelection==3) {fmOpen();
        } else if (menuPage == 0) {
          if (menuSelection == 0) {
            menuOpen = false;
            Serial.println("[MENU] closed via Back");
          } else {
            menuPage = menuSelection;
            menuSelection = 0;
            Serial.printf("[MENU] opened submenu %u\n", static_cast<unsigned>(menuPage));
          }
        } else if (menuPage == 2 && menuSelection == 2) {
          menuPage = 10;
          menuSelection = static_cast<uint8_t>(display.themeIndex() + 1);
        } else if (menuPage == 10 && menuSelection == 0) {
          menuPage = 2;
          menuSelection = 0;
        } else if (menuPage == 10 && menuSelection > 0) {
          display.setTheme(static_cast<uint8_t>(menuSelection - 1));
          themeSavePending = true;
        } else if (menuPage == 3 && menuSelection == 4) {
          menuPage = 11;
          menuSelection = 0;
        } else if (menuPage == 11 && menuSelection == 0) {
          menuPage = 3;
          menuSelection = 4;
        } else if (menuPage == 11 && menuSelection == 1) {
          if (radio.setAgcAutomatic(true)) {
            const bool saved = settingsStore.saveAgcAutomatic(true);
            Serial.printf("[SETTINGS] AGC AUTO %s\n", saved ? "saved" : "save failed");
          }
        } else if (menuPage == 11 && menuSelection >= 2 && menuSelection <= 5) {
          static const uint8_t agcIndexes[] = {0, 8, 16, 24};
          const uint8_t index = agcIndexes[menuSelection - 2];
          if (radio.setAgcManualGain(index)) {
            const bool modeSaved = settingsStore.saveAgcAutomatic(false);
            const bool indexSaved = settingsStore.saveAgcGainIndex(index);
            Serial.printf("[SETTINGS] AGC manual index=%u %s\n",
              static_cast<unsigned>(index), modeSaved && indexSaved ? "saved" : "save failed");
          }
        } else if (menuPage == 11 && menuSelection >= 6 && menuSelection <= 9) {
          const uint8_t attenuation = static_cast<uint8_t>((menuSelection - 6) * 4);
          if (radio.setSoftMuteAttenuation(attenuation)) {
            const bool saved = settingsStore.saveSoftMuteAttenuationDb(attenuation);
            Serial.printf("[SETTINGS] Soft Mute=%u dB %s\n",
              static_cast<unsigned>(attenuation), saved ? "saved" : "save failed");
          }
        } else if (menuPage == 11 && menuSelection >= 10 && menuSelection <= 13) {
          static const uint8_t avcGains[] = {12, 30, 60, 90};
          const uint8_t gain = avcGains[menuSelection - 10];
          if (radio.setAvcGain(gain)) {
            const bool saved = settingsStore.saveAvcGain(gain);
            Serial.printf("[SETTINGS] AVC=%u %s\n",
              static_cast<unsigned>(gain), saved ? "saved" : "save failed");
          }
        } else if (menuPage == 3 && menuSelection == 1) {
          menuPage = 8;
          menuSelection = static_cast<uint8_t>(radio.currentBandIndex() + 1);
        } else if (menuPage == 8 && menuSelection == 0) {
          menuPage = 3;
          menuSelection = 0;
        } else if (menuPage == 8 && menuSelection > 0) {
          if (radio.selectBand(static_cast<uint8_t>(menuSelection - 1))) {
            display.notifyTuning();
            Serial.printf("[MENU] band=%s freq=%lu kHz\n", radio.snapshot().bandName,
              static_cast<unsigned long>(radio.snapshot().frequencyKHz));
            menuPage = 3;
            menuSelection = 1;
          }
        } else if (menuPage == 3 && menuSelection == 3) {
          menuPage = 9;
          menuSelection = static_cast<uint8_t>(radio.stepChoice() + 1);
        } else if (menuPage == 9 && menuSelection == 0) {
          menuPage = 3;
          menuSelection = 0;
        } else if (menuPage == 9 && menuSelection > 0) {
          if (radio.setStepChoice(static_cast<uint8_t>(menuSelection - 1))) {
            menuPage = 3;
            menuSelection = 3;
          }
        } else if (menuPage == 3 && menuSelection == 2) {
          menuPage = 7;
          menuSelection = static_cast<uint8_t>(radio.bandwidthChoice() + 1);
        } else if (menuPage == 7 && menuSelection == 0) {
          menuPage = 3;
          menuSelection = 0;
        } else if (menuPage == 7 && menuSelection > 0) {
          if (radio.setBandwidthChoice(static_cast<uint8_t>(menuSelection - 1))) {
            Serial.printf("[MENU] bandwidth choice=%u\n", static_cast<unsigned>(menuSelection - 1));
            menuPage = 3;
            menuSelection = 2;
          }
        } else if (menuPage == 2 && menuSelection == 0) {
          saveBrightnessIfPending();
          saveThemeIfPending();
          menuPage = 0;
          menuSelection = 0;
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
          if (menuPage == 6 && fmMode == 4) {
            if (fmEditName[0]) {
              char dest[96];
              snprintf(dest,sizeof(dest),"%s%s%s",fmPath,strcmp(fmPath,"/")==0?"":"/",fmEditName);
              bool ok = fmPendingAction == 5 ? FileManager::makeDirectory(dest) : FileManager::renamePath(fmSelectedPath,dest);
              fmMessageSet(ok ? "Gespeichert" : "Fehler / Ziel vorhanden");
              fmRefresh();
            } else fmMessageSet("Name fehlt");
            fmMode = 0;
          } else if (menuPage == 6 && fmMode == 0) {
            menuOpen = false;
            menuPage = 0;
            menuSelection = 0;
            fmMessageSet("Dateimanager beendet");
          } else if (menuPage == 6 && fmMode == 3) {
            fmParent();
          } else if (menuPage == 6 && fmMode != 0) {
            fmMode = 0;
            fmMessageSet("Abgebrochen");
          } else {
            saveBrightnessIfPending();
            saveThemeIfPending();
            menuOpen = false;
            menuPage = 0;
            menuSelection = 0;
            Serial.println("[MENU] closed via long press");
          }
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
  display.render(radio.snapshot(),radio.isReady(),wifi.connected(),lastEncoderEvent,encoderEventCount,&stationLogoClient,menuOpen,menuPage,menuSelection,radio.volume(),fmPath,fmEntries,fmCount,fmSelection,fmMode,fmActionSelection,(fmMode==4?fmEditName:fmSelectedName),fmMessage);
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
