#pragma once
#include <Arduino.h>
#include "../radio/RadioSnapshot.h"
#include <string.h>

// ATS Mini DEV LW/MW/KW layouts adapted to the OS RadioSnapshot contract.
// Each renderer is isolated from the FM/DAB and TEF6686 renderers.
namespace SWLayouts {
constexpr uint16_t BG=0x0000, WHITE=0xFFFF, GREEN=0x07E0, CYAN=0x07FF;
constexpr uint16_t YELLOW=0xFFE0, RED=0xF800, GREY=0x7BEF, BLUE=0x001F;

template<class C> void header(C& d,const RadioSnapshot& s,bool wifi) {
  d.setTextDatum(TL_DATUM); d.setTextColor(GREEN,BG);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b); d.drawString(s.bandName,8,3);
  d.setFont(&lgfx::fonts::Font0); d.setTextColor(wifi?CYAN:GREY,BG);
  d.drawRightString(wifi?"WLAN+":"WLAN-",312,5);
}
template<class C> void frequency(C& d,const RadioSnapshot& s,int x,int y,float scale=0.72f) {
  char f[24];
  if(s.modulation==RadioModulation::USB || s.modulation==RadioModulation::LSB)
    snprintf(f,sizeof(f),"%lu.000",static_cast<unsigned long>(s.frequencyKHz));
  else snprintf(f,sizeof(f),"%lu",static_cast<unsigned long>(s.frequencyKHz));
  d.setFont(&lgfx::fonts::Font7); d.setTextColor(GREEN,BG); d.setTextDatum(TL_DATUM);
  d.setTextSize(scale); d.drawString(f,x,y); d.setTextSize(1.0f);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b); d.setTextColor(WHITE,BG);
  d.drawString("kHz",250,78);
}
template<class C> void meter(C& d,const RadioSnapshot& s,int x,int y,int width) {
  const int v=constrain(static_cast<int>(s.rssi),0,60);
  const int fill=map(v,0,60,0,width);
  d.drawRect(x,y,width,13,WHITE); d.fillRect(x+1,y+1,width-2,11,0x2104);
  const int g=width*65/100, a=width*82/100;
  if(fill>0)d.fillRect(x+1,y+1,min(fill,g),11,GREEN);
  if(fill>g)d.fillRect(x+1+g,y+1,min(fill-g,a-g),11,YELLOW);
  if(fill>a)d.fillRect(x+1+a,y+1,fill-a,11,RED);
}
inline const char* bandplan(uint32_t f) {
  if((f>=1839&&f<=1841)||(f>=3572&&f<=3574)||(f>=5356&&f<=5358)||
     (f>=7073&&f<=7075)||(f>=10135&&f<=10137)||(f>=14070&&f<=14077)||
     (f>=18099&&f<=18101)||(f>=21073&&f<=21075)||(f>=24914&&f<=24916)||
     (f>=28070&&f<=28080)) return "FT8 - USB";
  if((f>=1800&&f<=1837)||(f>=3500&&f<=3560)||(f>=7000&&f<=7040)||
     (f>=10100&&f<=10130)||(f>=14000&&f<=14069)||(f>=18068&&f<=18095)||
     (f>=21000&&f<=21069)||(f>=24890&&f<=24910)||(f>=28000&&f<=28069)) return "CW - MORSE";
  if((f>=1843&&f<=2000)||(f>=3600&&f<=3800)||(f>=7060&&f<=7200)) return "QSO - LSB";
  if((f>=5354&&f<=5366)||(f>=14100&&f<=14350)||(f>=18110&&f<=18168)||
     (f>=21150&&f<=21450)||(f>=24931&&f<=24990)||(f>=28300&&f<=29100)) return "QSO - USB";
  if((f>=3950&&f<=4000)||(f>=5900&&f<=6200)||(f>=7200&&f<=7450)||
     (f>=9400&&f<=9900)||(f>=11600&&f<=12100)||(f>=13570&&f<=13870)||
     (f>=15100&&f<=15800)||(f>=17480&&f<=17900)||(f>=21450&&f<=21850)) return "SW RADIO AM";
  if(f>=26565&&f<=27405)return "CB - AM / FM / SSB";
  return "MIXED";
}

template<class C> void drawDefault(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG); header(d,s,wifi);
  d.setTextColor(WHITE,BG); d.setFont(&lgfx::fonts::Font7); d.setTextSize(0.85f);
  d.drawString(s.modulation==RadioModulation::AM?"AM":s.modulation==RadioModulation::USB?"USB":"LSB",8,24);
  d.setTextSize(1.0f); frequency(d,s,8,48,0.78f);
  d.setTextColor(GREEN,BG); d.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  d.drawString(s.stationName[0]?s.stationName:(ready?"":"RADIO OFFLINE"),8,92);
  meter(d,s,45,119,230);
  d.setFont(&lgfx::fonts::Font0); d.setTextColor(WHITE,BG);
  d.drawString(s.radioText,8,143);
}
template<class C> void drawClean(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG); header(d,s,wifi);
  d.fillRoundRect(9,22,50,19,4,GREEN); d.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  d.setTextDatum(MC_DATUM); d.setTextColor(BG,GREEN);
  const char* mode=s.modulation==RadioModulation::AM?"AM":s.modulation==RadioModulation::USB?"USB":"LSB";
  d.drawString(mode,34,31);
  d.setTextDatum(TL_DATUM); d.setTextColor(GREEN,BG);
  frequency(d,s,74,30,0.67f);
  d.drawRoundRect(18,101,284,39,5,GREEN);
  d.setTextColor(WHITE,BG); d.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  d.setTextDatum(MC_DATUM); d.drawString(bandplan(s.frequencyKHz),160,112);
  d.setTextDatum(TL_DATUM); meter(d,s,50,151,220);
  if(!ready){d.setFont(&lgfx::fonts::Font0);d.setTextColor(YELLOW,BG);d.drawString("RADIO OFFLINE",8,82);}
}
template<class C> void drawNewUI(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG); header(d,s,wifi);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b); d.setTextColor(WHITE,BG);
  d.drawCenterString(s.modulation==RadioModulation::AM?"AM":s.modulation==RadioModulation::USB?"USB":"LSB",160,22);
  frequency(d,s,12,48,0.82f);
  d.setTextDatum(MC_DATUM); d.setTextColor(GREEN,BG);
  d.drawString(s.stationName[0]?s.stationName:(ready?"":"RADIO OFFLINE"),160,94);
  const int x=12,y=132,w=296,parts=19;
  d.drawRoundRect(x,y,w,23,4,WHITE);
  const int active=constrain(map(max(0,static_cast<int>(s.rssi)),0,60,0,parts),0,parts);
  for(int i=0;i<parts;i++){
    const uint16_t col=i<6?GREEN:(i<13?YELLOW:RED);
    const int px=x+4+i*15;
    d.fillRect(px,y+5,11,13,i<active?col:0x2104);
  }
  d.setTextDatum(TL_DATUM); d.setFont(&lgfx::fonts::Font0);
}
template<class C> void drawSMeter(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG); header(d,s,wifi);
  frequency(d,s,8,28,0.72f);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b); d.setTextColor(WHITE,BG);
  d.drawString(s.stationName[0]?s.stationName:(ready?"":"RADIO OFFLINE"),8,78);
  d.setFont(&lgfx::fonts::Font0);
  for(int i=0;i<=15;i++){
    const int x=12+i*19; d.drawFastVLine(x,111,5,GREY);
    if(i%2==0){char n[8];snprintf(n,sizeof(n),"%s%d",i>9?"+":"",i>9?(i-9)*10:i/2);d.drawCenterString(n,x,101);}
  }
  const int v=constrain(static_cast<int>(s.rssi),0,60);
  const int bars=map(v,0,60,0,49);
  for(int i=0;i<49;i++)d.fillRect(12+i*6,119,4,12,i<bars?(i<28?GREEN:YELLOW):0x2104);
  d.setTextColor(CYAN,BG); d.drawString("RSSI",12,140);
  char sn[16];snprintf(sn,sizeof(sn),"SNR %d",s.snr);d.drawString(sn,75,140);
  d.setTextColor(WHITE,BG);d.drawString(s.radioText,12,155);
}
template<class C> void drawDecoder(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG); header(d,s,wifi);
  frequency(d,s,8,26,0.70f);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b);d.setTextColor(GREEN,BG);
  d.drawCenterString(s.stationName[0]?s.stationName:(ready?"":"RADIO OFFLINE"),160,88);
  constexpr int x=20,y=126,w=280,h=34;
  d.drawRect(x,y,w,h,WHITE);
  static uint8_t history[w]={}; static uint32_t last=0;
  if(millis()-last>=120){for(int i=0;i<w-1;i++)history[i]=history[i+1];history[w-1]=static_cast<uint8_t>(constrain(s.rssi,0,60));last=millis();}
  for(int i=1;i<w;i++){
    const int y1=y+h-2-map(history[i-1],0,60,0,h-5);
    const int y2=y+h-2-map(history[i],0,60,0,h-5);
    d.drawLine(x+i-1,y1,x+i,y2,history[i]>40?RED:GREEN);
  }
  d.setFont(&lgfx::fonts::Font0);d.setTextColor(WHITE,BG);
}
template<class C> void drawTiny(C& d,const RadioSnapshot& s,bool ready,bool wifi) {
  d.fillScreen(BG);
  const char* labels[]={"LW","MW","KW"}; const int xx[]={4,42,80};
  for(int i=0;i<3;i++){const bool selected=strcmp(s.bandName,labels[i])==0;d.drawRect(xx[i],3,34,18,selected?CYAN:GREY);d.setTextDatum(MC_DATUM);d.setTextColor(selected?CYAN:GREY,BG);d.drawString(labels[i],xx[i]+17,8);}
  d.setTextDatum(TR_DATUM);d.setTextColor(wifi?CYAN:GREY,BG);d.drawString(wifi?"WLAN+":"WLAN-",312,6);
  frequency(d,s,8,34,0.80f);
  d.setFont(&lgfx::fonts::FreeSansBold9pt7b);d.setTextDatum(TL_DATUM);d.setTextColor(CYAN,BG);
  d.drawString(s.modulation==RadioModulation::AM?"AM":s.modulation==RadioModulation::USB?"USB":"LSB",8,72);
  d.setTextColor(GREEN,BG);d.drawString(s.stationName[0]?s.stationName:(ready?"":"RADIO OFFLINE"),8,98);
  d.setFont(&lgfx::fonts::Font0);d.setTextColor(WHITE,BG);d.drawString(s.radioText,8,116);
  d.drawFastHLine(0,142,320,GREY);
  const uint32_t lo=s.bandMinimumKHz,hi=s.bandMaximumKHz;
  if(hi>lo){for(int i=0;i<=20;i++){int px=i*16;d.drawFastVLine(px,142,i%4==0?12:6,GREY);}const int marker=constrain(map(s.frequencyKHz,lo,hi,0,319),0,319);d.fillCircle(marker,142,4,RED);}
  d.setTextDatum(TL_DATUM);
}
} // namespace SWLayouts
