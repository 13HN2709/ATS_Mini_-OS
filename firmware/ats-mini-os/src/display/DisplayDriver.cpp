#include "DisplayDriver.h"
#include <LittleFS.h>

namespace {
constexpr uint16_t BLACK=0x0000, WHITE=0xFFFF, GREEN=0x07E0, CYAN=0x07FF, DARK_GREEN=0x0320, YELLOW=0xFFE0;

template <typename Canvas>
void text(Canvas& d,const char* s,int x,int y,uint16_t c,const lgfx::IFont* f) {
  d.setTextColor(c,BLACK);
  d.setFont(f);
  d.drawString(s ? s : "",x,y);
}

template <typename Canvas>
void centered(Canvas& d,const char* s,int x,int y,uint16_t c,const lgfx::IFont* f) {
  d.setTextColor(c,BLACK);
  d.setFont(f);
  d.drawCenterString(s ? s : "",x,y);
}

bool getLogoDimensions(const uint8_t* data, size_t length, uint16_t& width, uint16_t& height) {
  width = height = 0;
  if (!data || length < 10) return false;

  // PNG stores its dimensions as big-endian integers in the IHDR chunk.
  static const uint8_t pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  if (length >= 24 && memcmp(data, pngSignature, sizeof(pngSignature)) == 0) {
    const uint32_t w = (static_cast<uint32_t>(data[16]) << 24) |
                       (static_cast<uint32_t>(data[17]) << 16) |
                       (static_cast<uint32_t>(data[18]) << 8) | data[19];
    const uint32_t h = (static_cast<uint32_t>(data[20]) << 24) |
                       (static_cast<uint32_t>(data[21]) << 16) |
                       (static_cast<uint32_t>(data[22]) << 8) | data[23];
    if (!w || !h || w > 65535 || h > 65535) return false;
    width = static_cast<uint16_t>(w);
    height = static_cast<uint16_t>(h);
    return true;
  }

  // JPEG dimensions are carried by a Start Of Frame marker.
  if (length < 4 || data[0] != 0xFF || data[1] != 0xD8) return false;
  size_t pos = 2;
  while (pos + 4 <= length) {
    if (data[pos] != 0xFF) { ++pos; continue; }
    while (pos < length && data[pos] == 0xFF) ++pos;
    if (pos >= length) break;
    const uint8_t marker = data[pos++];
    if (marker == 0xD9 || marker == 0xDA) break;
    if (marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) continue;
    if (pos + 2 > length) break;
    const uint16_t segmentLength = (static_cast<uint16_t>(data[pos]) << 8) | data[pos + 1];
    if (segmentLength < 2 || pos + segmentLength > length) break;
    const bool isSof = (marker >= 0xC0 && marker <= 0xC3) ||
                       (marker >= 0xC5 && marker <= 0xC7) ||
                       (marker >= 0xC9 && marker <= 0xCB) ||
                       (marker >= 0xCD && marker <= 0xCF);
    if (isSof && segmentLength >= 7) {
      height = (static_cast<uint16_t>(data[pos + 3]) << 8) | data[pos + 4];
      width = (static_cast<uint16_t>(data[pos + 5]) << 8) | data[pos + 6];
      return width > 0 && height > 0;
    }
    pos += segmentLength;
  }
  return false;
}

template <typename Canvas>
void drawFileMenu(Canvas& c,const char* path,FileManager::Entry* entries,uint8_t count,uint8_t selection,uint8_t mode,uint8_t action,const char* selected,const char* message){
  const int w=c.width(),h=c.height();
  c.fillScreen(BLACK);
  text(c,"DATEIMANAGER",8,2,CYAN,&lgfx::fonts::Font2);
  text(c,path,8,25,GREEN,&lgfx::fonts::Font0);
  char usage[32];
  snprintf(usage,sizeof(usage),"%lu/%lu KB",(unsigned long)(LittleFS.usedBytes()/1024),(unsigned long)(LittleFS.totalBytes()/1024));
  c.setTextColor(YELLOW,BLACK);
  c.setFont(&lgfx::fonts::Font0);
  c.drawRightString(usage,w-8,25);
  c.drawFastHLine(8,40,w-16,DARK_GREEN);

  if(mode==1){
    const char* items[]={"Zurueck","Kopieren","Verschieben","Umbenennen","Loeschen","Neuer Ordner"};
    text(c,selected,8,45,YELLOW,&lgfx::fonts::Font2);
    const int actionFirstY = 68;
    const int actionRowHeight = 25;
    const int actionFooterTop = h - 25;
    int actionVisibleRows = (actionFooterTop - actionFirstY - 20) / actionRowHeight + 1;
    if (actionVisibleRows < 1) actionVisibleRows = 1;
    if (actionVisibleRows > 6) actionVisibleRows = 6;
    const uint8_t actionVisible = static_cast<uint8_t>(actionVisibleRows);
    const uint8_t actionFirst = action >= actionVisible
      ? static_cast<uint8_t>(action - actionVisible + 1) : 0;
    for(uint8_t row=0; row<actionVisible && actionFirst+row<6; ++row){
      const uint8_t n = actionFirst + row;
      const int y = actionFirstY + row*actionRowHeight;
      if(n==action){c.fillRoundRect(5,y-2,w-10,22,3,0x0320);c.drawRoundRect(5,y-2,w-10,22,3,GREEN);}
      text(c,items[n],13,y,n==action?GREEN:WHITE,&lgfx::fonts::Font2);
    }
    text(c,"Drehen: Menue  |  Klick: Auswaehlen",8,h-18,YELLOW,&lgfx::fonts::Font0);
  } else if(mode==2){
    centered(c,"DATEI LOESCHEN?",w/2,54,YELLOW,&lgfx::fonts::Font2);
    centered(c,selected,w/2,83,WHITE,&lgfx::fonts::Font2);
    centered(c,action?"JA":"NEIN",w/2,121,action?YELLOW:GREEN,&lgfx::fonts::Font4);
    text(c,"Drehen: Ja/Nein  |  Klick: Bestaetigen",8,h-18,YELLOW,&lgfx::fonts::Font0);
  } else if(mode==4){
    centered(c,"NAME / ORDNER",w/2,48,CYAN,&lgfx::fonts::Font2);
    text(c,selected,8,82,GREEN,&lgfx::fonts::Font2);
    text(c,"Drehen: Zeichen aendern",8,119,WHITE,&lgfx::fonts::Font2);
    text(c,"Kurz: Naechstes Zeichen",8,146,WHITE,&lgfx::fonts::Font2);
    text(c,"Lang: Speichern",8,173,YELLOW,&lgfx::fonts::Font2);
  } else {
    text(c,mode==3?"ZIELORDNER: Klick waehlt":"Klick: Ordner oeffnen / Datei-Menue",8,44,YELLOW,&lgfx::fonts::Font0);
    const uint8_t total=static_cast<uint8_t>(count+(mode==0?3:1));
    // The ATS Mini V4 panel is only 170 px high. Keep list rows above
    // the two-line footer and calculate the visible window from real height.
    // This gives three rows on the 170 px panel and up to five on taller panels.
    const int footerTop = h - 30;
    const int firstRowY = 64;
    const int rowHeight = 27;
    int visibleRows = (footerTop - firstRowY - 22) / rowHeight + 1;
    if (visibleRows < 1) visibleRows = 1;
    if (visibleRows > 5) visibleRows = 5;
    const uint8_t visible = static_cast<uint8_t>(visibleRows);
    uint8_t first=selection>=visible?static_cast<uint8_t>(selection-visible+1):0;
    for(uint8_t row=0;row<visible && first+row<total;row++){
      const uint8_t ix=first+row;
      const int y=64+row*27;
      char line[48];
      const char* label=nullptr;
      if(ix==0) label=mode==3?"[DIESEN ORDNER WAEHLEN]":"[..] Ordner hoch";
      else if(mode==0 && ix==count+1) label="[+] NEUER ORDNER";
      else if(mode==0 && ix==count+2) label="[X] DATEIMANAGER BEENDEN";
      else {
        const uint8_t entryIndex=ix-1;
        if(entryIndex<count){
          FileManager::Entry& e=entries[entryIndex];
          if(e.directory) snprintf(line,sizeof(line),"[+] %.27s",e.name);
          else snprintf(line,sizeof(line),"%.21s  %luB",e.name,(unsigned long)e.size);
          label=line;
        }
      }
      if(!label) continue;
      if(ix==selection){c.fillRoundRect(4,y-3,w-8,24,3,0x0320);c.drawRoundRect(4,y-3,w-8,24,3,GREEN);}
      text(c,label,10,y,ix==selection?GREEN:WHITE,&lgfx::fonts::Font2);
    }
    if(mode==3) text(c,"Drehen: Zielordner  |  Lang: Zurueck",8,h-18,YELLOW,&lgfx::fonts::Font0);
    else {
      text(c,"Drehen: Blaettern  |  Kurz: Oeffnen",8,h-30,WHITE,&lgfx::fonts::Font0);
      if(mode==0) text(c,"Lang druecken: Dateimanager beenden",8,h-16,YELLOW,&lgfx::fonts::Font0);
    }
  }
  if(message&&message[0]) text(c,message,8,h-16,YELLOW,&lgfx::fonts::Font0);
}

template <typename Canvas>
void drawMenu(Canvas& canvas, uint8_t page, uint8_t selection, uint8_t volume, uint8_t layoutMode) {
  const int w = canvas.width();
  const int h = canvas.height();
  static const char* const mainItems[] = {
    "Zurueck", "Lautstaerke", "Anzeige", "Radio", "WLAN", "System"
  };
  static const char* const volumeItems[] = {
    "Zurueck", "Lautstaerke", "Stumm (spaeter)"
  };
  static const char* const displayItems[] = {
    "Zurueck", "Helligkeit (spaeter)"
  };
  static const char* const radioItems[] = {
    "Zurueck", "Schrittweite (spaeter)", "Bandplan (spaeter)"
  };
  static const char* const wifiItems[] = {
    "Zurueck", "WLAN-Modus (spaeter)", "Web Control (spaeter)"
  };
  static const char* const systemItems[] = {
    "Zurueck", "CPU-Profil (spaeter)", "Systeminfo (spaeter)", "Dateimanager",
    "UTC+01:00 * (Sommer +02:00)"
  };
  (void)layoutMode;
  const char* const* items = mainItems;
  uint8_t count = sizeof(mainItems) / sizeof(mainItems[0]);
  const char* title = "HAUPTMENUE";
  switch (page) {
    case 1: items=volumeItems; count=sizeof(volumeItems)/sizeof(volumeItems[0]); title="AUDIO"; break;
    case 2: items=displayItems; count=sizeof(displayItems)/sizeof(displayItems[0]); title="ANZEIGE"; break;
    case 3: items=radioItems; count=sizeof(radioItems)/sizeof(radioItems[0]); title="RADIO"; break;
    case 4: items=wifiItems; count=sizeof(wifiItems)/sizeof(wifiItems[0]); title="WLAN"; break;
    case 5: items=systemItems; count=sizeof(systemItems)/sizeof(systemItems[0]); title="SYSTEM"; break;
    default: break;
  }
  canvas.fillScreen(BLACK);
  text(canvas,"ATS MINI OS",10,3,CYAN,&lgfx::fonts::Font2);
  text(canvas,title,w-88,7,GREEN,&lgfx::fonts::Font0);
  canvas.drawFastHLine(8,30,w-16,DARK_GREEN);

  const int rowTop=37;
  const int rowHeight=36;
  const int visibleRows=(h-58)/rowHeight;
  int first = 0;
  if (selection >= visibleRows) first = selection-visibleRows+1;
  for (int row=0; row<visibleRows && first+row<count; ++row) {
    const uint8_t index=static_cast<uint8_t>(first+row);
    const int y=rowTop+row*rowHeight;
    const bool selected=index==selection;
    if (selected) {
      canvas.fillRoundRect(7,y,w-14,31,4,0x0320);
      canvas.drawRoundRect(7,y,w-14,31,4,GREEN);
    }
    text(canvas,items[index],15,y+8,selected?GREEN:WHITE,&lgfx::fonts::Font2);
    if (page==1 && index==1) {
      char value[12];
      snprintf(value,sizeof(value),"%u/63",static_cast<unsigned>(volume));
      canvas.setTextColor(YELLOW,BLACK);
      canvas.setFont(&lgfx::fonts::Font2);
      canvas.drawRightString(value,w-18,y+8);
      const int barX=15, barY=y+25, barW=w-32;
      canvas.drawRect(barX,barY,barW,4,0x7BEF);
      const int fillW=(barW-2)*volume/63;
      if(fillW>0) canvas.fillRect(barX+1,barY+1,fillW,2,GREEN);
    }
  }
  canvas.drawFastHLine(8,h-20,w-16,DARK_GREEN);
  text(canvas,page==1?"Drehen: Lautstaerke":"Drehen: Auswahl",10,h-15,0x7BEF,&lgfx::fonts::Font0);
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
  sprite_.setPsram(true);
  sprite_.setColorDepth(16);
  useSprite_ = sprite_.createSprite(device_.width(), device_.height());
  if (useSprite_) {
    sprite_.fillSprite(BLACK);
    sprite_.pushSprite(0, 0);
  } else {
    Serial.println("[DISPLAY] PSRAM sprite allocation failed; using direct rendering.");
  }
  device_.fillScreen(BLACK);
  Serial.printf("[DISPLAY] ready, panel ID=0x%06lX, size=%ldx%ld\n",
    static_cast<unsigned long>(device_.getDisplayId()),
    static_cast<long>(device_.width()),static_cast<long>(device_.height()));
  layoutMode_ = 0;
  Serial.println("[DISPLAY] active layout: FM");
  return true;
}

void DisplayDriver::setLayoutMode(uint8_t mode) {
  (void)mode;
  layoutMode_ = 0;
  lastGeneration_ = UINT32_MAX;
  Serial.println("[DISPLAY] FM is the only active layout");
}

void DisplayDriver::render(const RadioSnapshot& s,bool radioReady,bool wifiConnected,
                           const char* lastInputEvent,uint32_t inputEventCount,
                           StationLogoClient* logoClient, bool menuOpen,
                           uint8_t menuPage, uint8_t menuSelection, uint8_t volume, const char* filePath, FileManager::Entry* fileEntries, uint8_t fileCount, uint8_t fileSelection, uint8_t fileMode, uint8_t fileActionSelection, const char* fileSelectedName, const char* fileMessage) {
  if(!ready_)return;
  const uint32_t now=millis();
  const bool menuStateChanged = menuOpen != lastMenuOpen_ || menuPage != lastMenuPage_ ||
      menuSelection != lastMenuSelection_ || volume != lastMenuVolume_ || layoutMode_ != lastMenuLayout_;
  // Redraw the file manager promptly while the encoder is moving; this also
  // keeps the selection frame visually attached to the active row.
  const uint32_t redrawInterval = menuOpen ? ((menuPage==6) ? 20 : 500) : 40;
  if(s.generation==lastGeneration_ && now-lastDrawMs_<redrawInterval && !menuStateChanged)return;
  lastGeneration_=s.generation;
  lastDrawMs_=now;
  lastMenuOpen_=menuOpen;
  lastMenuPage_=menuPage;
  lastMenuSelection_=menuSelection;
  lastMenuVolume_=volume;
  lastMenuLayout_=layoutMode_;
  if (menuOpen) {
    if(menuPage==6){if(useSprite_){sprite_.fillSprite(BLACK);drawFileMenu(sprite_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);sprite_.pushSprite(0,0);}else{device_.fillScreen(BLACK);drawFileMenu(device_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);}return;}
    if (useSprite_) {
      sprite_.fillSprite(BLACK);
      drawMenu(sprite_, menuPage, menuSelection, volume, layoutMode_);
      sprite_.pushSprite(0,0);
    } else {
      device_.fillScreen(BLACK);
      drawMenu(device_, menuPage, menuSelection, volume, layoutMode_);
    }
    return;
  }

  // Keep the RDS ticker independent from radio-state polling. The SI4735
  // delivers text in bursts; the display animation must advance between them.
  char tickerText[72];
  snprintf(tickerText, sizeof(tickerText), "%s    ", s.radioText);
  const char* measuredText = tickerText;
  int textWidth = 0;
  if (useSprite_) {
    sprite_.setFont(&lgfx::fonts::Font2);
    textWidth = sprite_.textWidth(measuredText);
  } else {
    device_.setFont(&lgfx::fonts::Font2);
    textWidth = device_.textWidth(measuredText);
  }
  const int textViewportWidth = device_.width() - 126;
  if (strcmp(lastRadioTextRow_, s.radioText) != 0) {
    strncpy(lastRadioTextRow_, s.radioText, sizeof(lastRadioTextRow_) - 1);
    lastRadioTextRow_[sizeof(lastRadioTextRow_) - 1] = '\0';
    radioTextOffset_ = 0;
    radioTextHoldStartMs_ = now;
    radioTextLastStepMs_ = now;
  }
  const bool scrollRadioText = s.radioText[0] && textWidth > textViewportWidth;
  if (!scrollRadioText) {
    radioTextOffset_ = 0;
  } else if (now - radioTextHoldStartMs_ >= 1000 && now - radioTextLastStepMs_ >= 25) {
    --radioTextOffset_;
    if (radioTextOffset_ <= -textWidth) radioTextOffset_ = 0;
    radioTextLastStepMs_ = now;
  }

  const uint8_t* logoData = nullptr;
  size_t logoLength = 0;
  bool logoIsJpg = false;
  char logoKey[40] = {};
  if (s.piCode != 0 && s.frequencyKHz != 0) {
    snprintf(logoKey, sizeof(logoKey), "%04X_%lu",
      static_cast<unsigned>(s.piCode), static_cast<unsigned long>(s.frequencyKHz));
  }
  const bool logoLocked = logoClient && logoKey[0] &&
    logoClient->lockLogo(logoKey, logoData, logoLength, logoIsJpg);
  draw(s,radioReady,wifiConnected,lastInputEvent,inputEventCount,
       logoLocked ? logoData : nullptr,logoLocked ? logoLength : 0,logoIsJpg,
       radioTextOffset_, scrollRadioText, millis() < tuningUntilMs_);
  if (logoLocked) logoClient->unlockLogo();
}



template <typename Canvas>
void drawLayout(Canvas& canvas,const RadioSnapshot& s,bool radioReady,bool wifiConnected,
                const char* lastInputEvent,uint32_t inputEventCount,
                const uint8_t* logoData,size_t logoLength,bool logoIsJpg,
                int16_t radioTextOffset,bool scrollRadioText,bool frequencyBlue) {
  const int w=canvas.width();
  const int h=canvas.height();
  (void)lastInputEvent; (void)inputEventCount; (void)frequencyBlue;

  canvas.fillScreen(BLACK);

  // Header: band, future RDS clock position, WLAN and estimated battery.
  text(canvas,"FM",10,5,GREEN,&lgfx::fonts::Font2);
  // Show only a valid RDS CT clock; otherwise keep the center completely blank.
  if (s.rdsClockValid) {
    char clockText[6];
    snprintf(clockText,sizeof(clockText),"%02u:%02u",
      static_cast<unsigned>(s.rdsClockHour),
      static_cast<unsigned>(s.rdsClockMinute));
    centered(canvas,clockText,w/2,5,WHITE,&lgfx::fonts::Font2);
  }
  text(canvas,wifiConnected?"WLAN+":"WLAN-",w-91,7,wifiConnected?CYAN:0x7BEF,&lgfx::fonts::Font0);

  uint32_t adcTotal=0;
  for(uint8_t i=0;i<10;++i) adcTotal+=analogRead(4);
  const float batteryVolts=(static_cast<float>(adcTotal)/10.0f)*1.702f/1000.0f;
  int batteryPercent=static_cast<int>((batteryVolts-3.30f)*100.0f/0.90f+0.5f);
  batteryPercent=constrain(batteryPercent,0,100);
  char batteryText[8];
  snprintf(batteryText,sizeof(batteryText),"%d%%",batteryPercent);
  text(canvas,batteryText,w-43,7,batteryPercent<=15?YELLOW:GREEN,&lgfx::fonts::Font0);
  canvas.drawFastHLine(8,25,w-16,DARK_GREEN);

  // Station name sits directly above the frequency; its left edge aligns
  // with the first frequency digit.
  const char* station=s.stationName[0]?s.stationName:(radioReady?"--":"RADIO OFFLINE");
  canvas.fillRect(124,29,w-132,22,BLACK);
  text(canvas,station,124,30,WHITE,&lgfx::fonts::Font4);

  // Left-hand station logo. Source files remain at their original resolution;
  // the renderer scales them proportionally into this display viewport.
  constexpr int logoX=8, logoY=51, logoW=108, logoH=76;
  canvas.fillRoundRect(logoX,logoY,logoW,logoH,6,WHITE);
  bool logoDrawn=false;
  if(logoData&&logoLength){
    uint16_t sourceW=0,sourceH=0;
    if(getLogoDimensions(logoData,logoLength,sourceW,sourceH)){
      const float sx=static_cast<float>(logoW-8)/sourceW;
      const float sy=static_cast<float>(logoH-8)/sourceH;
      const float scale=sx<sy?sx:sy;
      const int drawW=static_cast<int>(sourceW*scale+0.5f);
      const int drawH=static_cast<int>(sourceH*scale+0.5f);
      const int drawX=logoX+(logoW-drawW)/2;
      const int drawY=logoY+(logoH-drawH)/2;
      if(logoIsJpg)
        logoDrawn=canvas.drawJpg(logoData,logoLength,drawX,drawY,0,0,0,0,scale,scale);
      else
        logoDrawn=canvas.drawPng(logoData,logoLength,drawX,drawY,0,0,0,0,scale,scale);
    }
  }
  if(!logoDrawn){
    centered(canvas,logoData?"LOGO FEHLER":"LOGO SUCHT...",logoX+logoW/2,logoY+20,0x7BEF,&lgfx::fonts::Font2);
    centered(canvas,"PI / LittleFS",logoX+logoW/2,logoY+42,DARK_GREEN,&lgfx::fonts::Font0);
  }

  // Large frequency readout, inspired by the supplied concept.
  char frequency[24];
  snprintf(frequency,sizeof(frequency),"%lu.%02lu",
    static_cast<unsigned long>(s.frequencyKHz/1000),
    static_cast<unsigned long>((s.frequencyKHz%1000)/10));
  canvas.setTextColor(GREEN,BLACK);
  canvas.setFont(&lgfx::fonts::Font4);
  canvas.drawString(frequency,124,58);
  text(canvas,"MHz",w-43,75,WHITE,&lgfx::fonts::Font2);

  // Stereo and PTY share the full row; the redundant RDS badge is removed.
  const int badgeY=103;
  canvas.drawRoundRect(124,badgeY,78,20,5,GREEN);
  centered(canvas,s.stereo?"STEREO":"MONO",163,badgeY+6,GREEN,&lgfx::fonts::Font0);

  static const char* const ptyNames[32]={
    "NONE","NEWS","CURRENT AFFAIRS","INFO","SPORT","EDUCATION","DRAMA","CULTURE",
    "SCIENCE","VARIED","POP MUSIC","ROCK MUSIC","EASY LISTENING","LIGHT CLASSICS",
    "SERIOUS CLASSICS","OTHER MUSIC","WEATHER","FINANCE","CHILDREN","SOCIAL",
    "RELIGION","PHONE IN","TRAVEL","LEISURE","JAZZ","COUNTRY","NATIONAL MUSIC",
    "OLDIES","FOLK MUSIC","DOCUMENTARY","ALARM TEST","ALARM"
  };
  const char* pty=ptyNames[s.pty<32?s.pty:0];
  canvas.drawRoundRect(210,badgeY,w-218,20,5,WHITE);
  canvas.setFont(&lgfx::fonts::Font0);
  canvas.setTextColor(WHITE,BLACK);
  canvas.drawCenterString(pty,210+(w-218)/2,badgeY+6);

  // Bottom RT ticker in its own neon-green rounded frame.
  constexpr int rtX=8,rtY=130,rtW=304,rtH=32;
  canvas.drawRoundRect(rtX,rtY,rtW,rtH,7,GREEN);
  const char* rt=s.radioText;
  canvas.setFont(&lgfx::fonts::Font2);
  canvas.setTextColor(WHITE,BLACK);
  canvas.setClipRect(rtX+10,rtY+3,rtW-20,rtH-6);
  if(scrollRadioText){
    char ticker[72];
    snprintf(ticker,sizeof(ticker),"%s    ",s.radioText);
    const int tickerWidth=canvas.textWidth(ticker);
    canvas.drawString(ticker,rtX+14+radioTextOffset,rtY+8);
    canvas.drawString(ticker,rtX+14+radioTextOffset+tickerWidth,rtY+8);
  } else {
    canvas.drawString(rt,rtX+14,rtY+8);
  }
  canvas.clearClipRect();
}
void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady,bool wifiConnected,const char* lastInputEvent,uint32_t inputEventCount,const uint8_t* logoData,size_t logoLength,bool logoIsJpg,int16_t radioTextOffset,bool scrollRadioText,bool frequencyBlue) {
  if (useSprite_) {
    sprite_.fillSprite(BLACK);
    drawLayout(sprite_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoData,logoLength,logoIsJpg,radioTextOffset,scrollRadioText,frequencyBlue);
    sprite_.pushSprite(0,0);
  } else {
    device_.fillScreen(BLACK);
    drawLayout(device_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoData,logoLength,logoIsJpg,radioTextOffset,scrollRadioText,frequencyBlue);
  }
}
