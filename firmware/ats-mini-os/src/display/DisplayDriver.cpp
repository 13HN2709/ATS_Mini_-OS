#include "DisplayDriver.h"

namespace {
constexpr uint16_t BLACK=0x0000, WHITE=0xFFFF, GREEN=0x07E0, CYAN=0x07FF, DARK_GREEN=0x0320, YELLOW=0xFFE0;

void text(lgfx::LGFX_Device& d,const char* s,int x,int y,uint16_t c,const lgfx::IFont* f) {
  d.setTextColor(c,BLACK);
  d.setFont(f);
  d.drawString(s ? s : "",x,y);
}

void centered(lgfx::LGFX_Device& d,const char* s,int x,int y,uint16_t c,const lgfx::IFont* f) {
  d.setTextColor(c,BLACK);
  d.setFont(f);
  d.drawCenterString(s ? s : "",x,y);
}
}

bool DisplayDriver::begin() {
#if !defined(LILYGO_SI473X)
  // ATS Mini V4 has a separate backlight MOSFET on GPIO38; panel brightness
  // alone does not drive this external backlight pin.
  ledcAttach(38, 16000, 8);
  ledcWrite(38, 0);
#endif
  ready_=device_.init();
  if(!ready_){
    Serial.println("[DISPLAY] init failed; continuing with serial diagnostics.");
    return false;
  }
  // Match ATS Mini Dev: the ATS Mini V4 panel is correctly oriented at rotation 3.
  device_.setRotation(3);
  device_.setBrightness(180);
#if !defined(LILYGO_SI473X)
  ledcWrite(38, 180);
#endif
  device_.fillScreen(BLACK);
  Serial.printf("[DISPLAY] ready, panel ID=0x%06lX, size=%ldx%ld\n",
    static_cast<unsigned long>(device_.getDisplayId()),
    static_cast<long>(device_.width()),static_cast<long>(device_.height()));
  return true;
}

void DisplayDriver::render(const RadioSnapshot& s,bool radioReady,const char* lastInputEvent,uint32_t inputEventCount) {
  if(!ready_)return;
  const uint32_t now=millis();
  if(s.generation==lastGeneration_ && now-lastDrawMs_<500)return;
  lastGeneration_=s.generation;
  lastDrawMs_=now;
  draw(s,radioReady,lastInputEvent,inputEventCount);
}

void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady,const char* lastInputEvent,uint32_t inputEventCount) {
  const int w=device_.width();
  // The frame/background are drawn once in begin(); avoid clearing the full
  // panel on every refresh, which makes the display appear to reload/flicker.
  device_.drawRoundRect(5,5,w-10,160,7,GREEN);

  text(device_,"ATS MINI OS  |  RDS TEST",13,8,CYAN,&lgfx::fonts::Font2);

  char freq[20];
  if(s.frequencyKHz>=1000) {
    snprintf(freq,sizeof(freq),"%lu.%03lu MHz",
      static_cast<unsigned long>(s.frequencyKHz/1000),
      static_cast<unsigned long>(s.frequencyKHz%1000));
  } else {
    snprintf(freq,sizeof(freq),"%lu kHz",static_cast<unsigned long>(s.frequencyKHz));
  }
  centered(device_,freq,w/2,25,GREEN,&lgfx::fonts::Font7);

  device_.drawFastHLine(13,67,w-26,DARK_GREEN);

  // Clear only the PS row so shorter station names do not leave old characters.
  device_.fillRect(12,70,w-24,18,BLACK);
  text(device_,"PS:",13,72,YELLOW,&lgfx::fonts::Font2);
  text(device_,s.stationName[0]?s.stationName:(radioReady?"Warte auf PS...":"RADIO OFFLINE"),
       45,72,WHITE,&lgfx::fonts::Font2);

  // PI and PTY are displayed as raw values to make decoding verifiable.
  char rdsIds[48];
  if(s.piCode) {
    snprintf(rdsIds,sizeof(rdsIds),"PI:%04X  PTY:%02u",static_cast<unsigned>(s.piCode),
             static_cast<unsigned>(s.pty));
  } else {
    snprintf(rdsIds,sizeof(rdsIds),"PI:----  PTY:%02u",static_cast<unsigned>(s.pty));
  }
  text(device_,rdsIds,13,91,CYAN,&lgfx::fonts::Font2);

  device_.drawFastHLine(13,111,w-26,DARK_GREEN);
  // RT changes independently of PS; clear its own row before drawing new text.
  device_.fillRect(12,113,w-24,18,BLACK);
  text(device_,"RT:",13,115,YELLOW,&lgfx::fonts::Font2);
  // RT is kept independent of PS. Long text will be clipped for now; scrolling
  // can be added after the first hardware validation of the received RDS data.
  text(device_,s.radioText[0]?s.radioText:"Warte auf Radiotext...",
       43,115,WHITE,&lgfx::fonts::Font2);

  char inputStatus[40];
  snprintf(inputStatus,sizeof(inputStatus),"ENC:%s  EVENTS:%lu",
    lastInputEvent ? lastInputEvent : "WAIT",static_cast<unsigned long>(inputEventCount));
  text(device_,inputStatus,13,132,CYAN,&lgfx::fonts::Font0);

  char status[72];
  snprintf(status,sizeof(status),"%s RSSI:%d SNR:%d RDS:%s",
    s.stereo?"ST":"MO",static_cast<int>(s.rssi),static_cast<int>(s.snr),
    s.rdsAvailable?"RX":"--");
  text(device_,status,13,148,GREEN,&lgfx::fonts::Font0);

  text(device_,radioReady?"TUNER:READY":"TUNER:ERROR",w-100,148,
       radioReady?GREEN:0xF800,&lgfx::fonts::Font0);
}
