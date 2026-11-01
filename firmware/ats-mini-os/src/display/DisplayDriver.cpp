#include "DisplayDriver.h"

namespace {
constexpr uint16_t BLACK=0x0000, WHITE=0xFFFF, GREEN=0x07E0, CYAN=0x07FF, DARK_GREEN=0x0320;
void text(lgfx::LGFX_Device& d,const char* s,int x,int y,uint16_t c,const lgfx::IFont* f) {
  d.setTextColor(c,BLACK); d.setFont(f); d.drawString(s ? s : "",x,y);
}
}
bool DisplayDriver::begin() {
  ready_=device_.init();
  if(!ready_){Serial.println("[DISPLAY] init failed; continuing with serial diagnostics.");return false;}
  device_.setRotation(1); device_.setBrightness(180); device_.fillScreen(BLACK);
  Serial.printf("[DISPLAY] ready, panel ID=0x%06lX, size=%dx%d\n",
    static_cast<unsigned long>(device_.getDisplayId()),device_.width(),device_.height());
  return true;
}
void DisplayDriver::render(const RadioSnapshot& s,bool radioReady) {
  if(!ready_)return;
  const uint32_t now=millis();
  if(s.generation==lastGeneration_ && now-lastDrawMs_<500)return;
  lastGeneration_=s.generation; lastDrawMs_=now; draw(s,radioReady);
}
void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady) {
  const int w=device_.width(); device_.fillScreen(BLACK);
  device_.drawRoundRect(5,5,w-10,160,7,GREEN);
  text(device_,"ATS MINI OS",14,9,CYAN,&lgfx::fonts::Font2);
  char freq[20];
  if(s.frequencyKHz>=1000) snprintf(freq,sizeof(freq),"%lu.%03lu",
    static_cast<unsigned long>(s.frequencyKHz/1000),static_cast<unsigned long>(s.frequencyKHz%1000));
  else snprintf(freq,sizeof(freq),"%lu kHz",static_cast<unsigned long>(s.frequencyKHz));
  device_.setTextColor(GREEN,BLACK); device_.setFont(&lgfx::fonts::Font7);
  device_.drawCenterString(freq,w/2,27);
  device_.drawFastHLine(15,91,w-30,DARK_GREEN);
  device_.drawRoundRect(14,98,w-28,29,4,GREEN);
  text(device_,s.stationName[0]?s.stationName:(radioReady?"Warte auf RDS...":"RADIO NICHT BEREIT"),22,103,WHITE,&lgfx::fonts::Font2);
  text(device_,s.radioText[0]?s.radioText:"Radiotext wartet...",14,132,CYAN,&lgfx::fonts::Font2);
  char status[64]; snprintf(status,sizeof(status),"%s RSSI:%d SNR:%d",
    s.stereo?"STEREO":"MONO",static_cast<int>(s.rssi),static_cast<int>(s.snr));
  text(device_,status,14,150,GREEN,&lgfx::fonts::Font0);
}
