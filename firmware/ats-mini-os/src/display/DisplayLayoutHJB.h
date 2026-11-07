#pragma once
#include <Arduino.h>
#include "../radio/RadioSnapshot.h"

// HJB layout ported from ats-mini-dev/Layout-HJB.cpp.
// It is deliberately logo-free and consumes only the published radio snapshot.
template <typename Canvas>
void drawHJBLayout(Canvas& d, const RadioSnapshot& s, bool radioReady, bool wifiConnected) {
  constexpr uint16_t BG=0x0000, YELLOW=0xFFE0, GREEN=0x07E0, BLUE=0x07FF;
  constexpr uint16_t RED=0xF800, MUTED=0x7BEF, GRID=0x4208;
  const int w=d.width();
  d.fillScreen(BG);

  d.setTextDatum(TL_DATUM);
  d.setTextColor(GREEN, BG);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  const char* mode=s.modulation==RadioModulation::FM?"FM":
    (s.modulation==RadioModulation::AM?"AM":
    (s.modulation==RadioModulation::USB?"USB":"LSB"));
  d.drawString(mode,7,5);
  d.setFont(&lgfx::fonts::Font0);
  d.setTextColor(wifiConnected?BLUE:MUTED,BG);
  d.drawRightString(wifiConnected?"WLAN+":"WLAN-",w-8,7);
  // Keep the battery readout from the OS renderer; no sender-logo resources
  // are allocated or reserved anywhere in this layout.
  uint32_t adcTotal=0;
  for(uint8_t i=0;i<10;++i) adcTotal+=analogRead(4);
  const float batteryVolts=(static_cast<float>(adcTotal)/10.0f)*1.702f/1000.0f;
  const int batteryPercent=constrain(static_cast<int>((batteryVolts-3.30f)*100.0f/0.90f+0.5f),0,100);
  char batteryText[8];
  snprintf(batteryText,sizeof(batteryText),"%d%%",batteryPercent);
  d.setTextColor(batteryPercent<=15?YELLOW:GREEN,BG);
  d.drawRightString(batteryText,w-55,7);

  static const char* const ptyNames[32]={
    "NONE","NEWS","CURRENT AFFAIRS","INFO","SPORT","EDUCATION","DRAMA","CULTURE",
    "SCIENCE","VARIED","POP MUSIC","ROCK MUSIC","EASY LISTENING","LIGHT CLASSICS",
    "SERIOUS CLASSICS","OTHER MUSIC","WEATHER","FINANCE","CHILDREN","SOCIAL",
    "RELIGION","PHONE IN","TRAVEL","LEISURE","JAZZ","COUNTRY","NATIONAL MUSIC",
    "OLDIES","FOLK","DOCUMENTARY","ALARM TEST","ALARM"
  };
  if(s.modulation==RadioModulation::FM && s.rdsAvailable) {
    d.setTextDatum(MC_DATUM);
    d.setTextColor(0xFFFF,BG);
    d.drawString(ptyNames[s.pty<32?s.pty:0],160,12,&lgfx::fonts::Font0);
  }

  char clockText[6]={};
  if(s.modulation==RadioModulation::FM && s.rdsClockValid) {
    snprintf(clockText,sizeof(clockText),"%02u:%02u",
      static_cast<unsigned>(s.rdsClockHour),static_cast<unsigned>(s.rdsClockMinute));
  }
  d.setTextDatum(MC_DATUM);
  d.setTextColor(YELLOW,BG);
  d.setFont(&lgfx::fonts::Font7);
  if(clockText[0]) d.drawString(clockText,160,48);
  else if(s.modulation==RadioModulation::FM) d.drawFloat(s.frequencyKHz/1000.0f,2,160,48);
  else if(s.modulation==RadioModulation::USB || s.modulation==RadioModulation::LSB) {
    char f[24]; snprintf(f,sizeof(f),"%lu.000",static_cast<unsigned long>(s.frequencyKHz));
    d.drawString(f,160,48);
  } else d.drawNumber(s.frequencyKHz,160,48);

  // Left signal history: RSSI trace with SNR reference line.
  constexpr int gx=7,gy=56,gw=62,gh=40;
  const int gl=gx+3,gr=gx+gw-3,gb=gy+gh-4,gt=gy+4;
  static uint8_t history[56]={};
  static uint8_t snrHold=0;
  static uint32_t lastSample=0;
  const uint32_t now=millis();
  const int rssi=constrain(static_cast<int>(s.rssi),0,100);
  const int snr=constrain(static_cast<int>(s.snr),0,40);
  if(now-lastSample>=100 || lastSample==0) {
    for(int i=0;i<55;++i) history[i]=history[i+1];
    history[55]=static_cast<uint8_t>(rssi);
    snrHold=static_cast<uint8_t>(snr);
    lastSample=now;
  }
  d.drawRect(gx,gy,gw,gh,GRID);
  const int sy=gb-map(static_cast<int>(snrHold),0,40,0,gb-gt);
  d.drawFastHLine(gl,sy,gr-gl,RED);
  for(int i=1;i<56;++i) {
    const int x1=gl+((i-1)*(gr-gl))/55, x2=gl+(i*(gr-gl))/55;
    const int y1=gb-map(static_cast<int>(history[i-1]),0,100,0,gb-gt);
    const int y2=gb-map(static_cast<int>(history[i]),0,100,0,gb-gt);
    d.drawLine(x1,y1,x2,y2,BLUE);
  }
  d.setTextDatum(TL_DATUM);
  d.setFont(&lgfx::fonts::Font0);
  d.setTextColor(BLUE,BG); d.drawString("RSSI",gx+2,gy-10);
  d.setTextColor(RED,BG); d.drawString("SNR",gx+34,gy-10);

  // Right signal bars.
  const int bx=258;
  d.setTextDatum(MC_DATUM);
  const bool stereo=s.modulation==RadioModulation::FM && s.stereo;
  d.setTextColor(stereo?GREEN:MUTED,BG);
  d.drawString(stereo?"STEREO":"MONO",bx+31,31,&lgfx::fonts::Font0);
  d.setTextDatum(TL_DATUM);
  d.setTextColor(MUTED,BG);
  d.drawString("RSSI",bx,48,&lgfx::fonts::Font0);
  d.drawString("SNR",bx,70,&lgfx::fonts::Font0);
  const int rb=constrain(map(rssi,0,100,0,8),0,8);
  const int sb=constrain(map(snr,0,40,0,8),0,8);
  for(int i=0;i<8;++i) {
    d.fillRect(bx+i*8,60,6,6,i<rb?(i<5?GREEN:YELLOW):0x2104);
    d.fillRect(bx+i*8,82,6,6,i<sb?(i<5?BLUE:YELLOW):0x2104);
  }

  // Station name and FM radio text; no logo area or logo client involved.
  const char* station=s.stationName[0]?s.stationName:(radioReady?"":"RADIO OFFLINE");
  d.setTextDatum(MC_DATUM);
  d.setTextColor(GREEN,BG);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  if(station && *station) d.drawString(station,160,96);
  if(s.modulation==RadioModulation::FM && s.radioText[0]) {
    char rt[48]; snprintf(rt,sizeof(rt),"%.44s",s.radioText);
    d.setTextColor(0xFFFF,BG);
    d.setFont(&lgfx::fonts::Font0);
    d.drawString(rt,160,119);
  } else if(!radioReady) {
    d.setTextColor(YELLOW,BG);
    d.setFont(&lgfx::fonts::Font0);
    d.drawString("RADIO OFFLINE",160,119);
  }

  // Frequency ruler, shared by all bands and modulation modes.
  const uint32_t lo=s.bandMinimumKHz, hi=s.bandMaximumKHz;
  const int left=4,right=316,y=146;
  d.drawFastHLine(left,y,right-left,GRID);
  d.drawFastHLine(left,168,right-left,GRID);
  if(hi>lo) {
    const uint32_t span=hi-lo;
    for(int i=0;i<=40;++i) {
      const int x=left+(i*(right-left))/40;
      const bool major=(i%4)==0;
      d.drawFastVLine(x,major?y:y+12,major?22:10,MUTED);
      if(major) {
        const uint32_t f=lo+(span*static_cast<uint32_t>(i))/40;
        d.setTextDatum(MC_DATUM);
        d.setTextColor(MUTED,BG);
        if(s.modulation==RadioModulation::FM) d.drawFloat(f/1000.0f,1,x,139,&lgfx::fonts::Font0);
        else d.drawNumber(f,x,139,&lgfx::fonts::Font0);
      }
    }
    const float pos=constrain((static_cast<float>(s.frequencyKHz)-lo)/static_cast<float>(span),0.0f,1.0f);
    const int mx=left+static_cast<int>(pos*(right-left));
    d.fillTriangle(mx-4,y-1,mx+4,y-1,mx,y+5,RED);
    d.drawFastVLine(mx,y+5,17,RED);
  }
}
