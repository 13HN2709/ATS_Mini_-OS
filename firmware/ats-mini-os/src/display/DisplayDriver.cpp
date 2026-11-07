#include "DisplayDriver.h"
#include "ThemeManager.h"
#include "DisplayLayoutAM.h"
#include "../radio/BandManager.h"
#include <LittleFS.h>

namespace {
uint16_t BLACK=0x0000, WHITE=0xFFFF, GREEN=0x07E0, CYAN=0x07FF, DARK_GREEN=0x0320, YELLOW=0xFFE0, MUTED=0x7BEF, FREQ_COLOR=0x07E0, UNIT_COLOR=0x07E0, RDS_COLOR=0xFFFF;
void applyThemePalette(uint8_t index) { const ThemePalette& p=ThemeManager::at(index); BLACK=p.background; WHITE=p.text; GREEN=p.accent; CYAN=p.secondary; DARK_GREEN=p.divider; YELLOW=p.warning; MUTED=p.muted; FREQ_COLOR=p.frequency; UNIT_COLOR=p.frequencyUnit; RDS_COLOR=p.radioText; }

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
void drawMenu(Canvas& canvas, uint8_t page, uint8_t selection, uint8_t volume, uint8_t brightness, uint8_t layoutMode, RadioModulation modulation, uint8_t themeIndex) {
  const int w = canvas.width();
  const int h = canvas.height();
  static const char* const mainItems[] = {
    "Zurueck", "Lautstaerke", "Anzeige", "Radio", "WLAN", "System"
  };
  static const char* const volumeItems[] = {
    "Zurueck", "Lautstaerke", "Stumm (spaeter)"
  };
  static const char* const displayItems[] = {
    "Zurueck", "Display-Helligkeit", "Farbschema"
  };
  static const char* const radioItems[] = {
    "Zurueck", "Band waehlen", "Bandbreite", "Schrittweite", "Empfangseinstellungen"
  };
  static const char* const fmBandwidthItems[] = { "Zurueck", "Auto", "110 kHz", "84 kHz", "60 kHz", "40 kHz" };
  static const char* const amBandwidthItems[] = { "Zurueck", "1.0 kHz", "1.8 kHz", "2.0 kHz", "2.5 kHz", "3.0 kHz", "4.0 kHz", "6.0 kHz" };
  static const char* const ssbBandwidthItems[] = { "Zurueck", "0.5 kHz", "1.0 kHz", "1.2 kHz", "2.2 kHz", "3.0 kHz", "4.0 kHz" };
  static const char* const radioSettingsItems[] = { "Zurueck", "AGC Auto", "AGC Manual", "Soft Mute Ein", "Soft Mute Aus", "AVC 90", "AVC 60", "AVC 30" };\n  static const char* const themeItems[] = { "Zurueck", "Classic", "TEF Blue", "Amber" };
  static const char* const fmStepItems[] = { "Zurueck", "10 kHz (DX)", "50 kHz", "100 kHz", "200 kHz", "1 MHz" };
  static const char* const amStepItems[] = { "Zurueck", "1 kHz", "5 kHz", "9 kHz", "10 kHz" };
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
    case 7:
      if (modulation == RadioModulation::FM) { items=fmBandwidthItems; count=sizeof(fmBandwidthItems)/sizeof(fmBandwidthItems[0]); }
      else if (modulation == RadioModulation::AM) { items=amBandwidthItems; count=sizeof(amBandwidthItems)/sizeof(amBandwidthItems[0]); }
      else { items=ssbBandwidthItems; count=sizeof(ssbBandwidthItems)/sizeof(ssbBandwidthItems[0]); }
      title="BANDBREITE"; break;
    case 8: count=static_cast<uint8_t>(BandManager::count()+1); title="BAND WAEHLEN"; break;
    case 11: items=radioSettingsItems; count=(modulation==RadioModulation::FM?5:8); title="EMPFANG"; break;\n    case 10: items=themeItems; count=sizeof(themeItems)/sizeof(themeItems[0]); title="FARBSCHEMA"; break;
    case 9:
      if (modulation == RadioModulation::FM) { items=fmStepItems; count=sizeof(fmStepItems)/sizeof(fmStepItems[0]); }
      else { items=amStepItems; count=sizeof(amStepItems)/sizeof(amStepItems[0]); }
      title="SCHRITTWEITE"; break;
    default: break;
  }

  // High-contrast monochrome menu: pure black background, bold white text,
  // thin white selection frame and no glow, gradients or colored highlights.
  canvas.fillScreen(BLACK);
  canvas.setTextColor(WHITE, BLACK);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.drawString("ATS MINI OS", 8, 3);
  canvas.drawRightString(title, w-8, 3);
  canvas.drawFastHLine(8, 27, w-16, WHITE);

  // Larger rows intentionally show only two entries at a time on the 320x170 panel.
  const int rowTop = 34;
  const int rowHeight = 54;
  const int visibleRows = (h - 58) / rowHeight;
  int first = 0;
  if (selection >= visibleRows) first = selection-visibleRows+1;
  for (int row=0; row<visibleRows && first+row<count; ++row) {
    const uint8_t index = static_cast<uint8_t>(first+row);
    const int y = rowTop + row*rowHeight;
    const bool selected = index == selection;
    if (selected) canvas.drawRoundRect(7, y+1, w-14, 48, 3, WHITE);

    canvas.setTextColor(WHITE, BLACK);
    canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    if (page == 8) {
      if (index == 0) {
        canvas.drawString("Zurueck", 16, y+13);
      } else {
        const RadioBandDefinition& band = BandManager::at(static_cast<uint8_t>(index-1));
        char bandLabel[40];
        if (band.type == RadioBandType::FM) {
          snprintf(bandLabel, sizeof(bandLabel), "%s  %lu-%lu MHz", band.name,
            static_cast<unsigned long>(band.minimumKHz/1000UL),
            static_cast<unsigned long>(band.maximumKHz/1000UL));
        } else {
          snprintf(bandLabel, sizeof(bandLabel), "%s  %lu-%lu kHz", band.name,
            static_cast<unsigned long>(band.minimumKHz),
            static_cast<unsigned long>(band.maximumKHz));
        }
        if (index-1 == BandManager::currentIndex()) {
          char currentLabel[44];
          snprintf(currentLabel, sizeof(currentLabel), "* %s", bandLabel);
          canvas.drawString(currentLabel, 16, y+13);
        } else {
          canvas.drawString(bandLabel, 16, y+13);
        }
      }
    } else if (page == 10 && index > 0 && index - 1 == ThemeManager::normalize(themeIndex)) {
      char selectedTheme[32];
      snprintf(selectedTheme, sizeof(selectedTheme), "* %s", items[index]);
      canvas.drawString(selectedTheme, 16, y+13);
    } else {
      canvas.drawString(items[index], 16, y+13);
    }

    if ((page==1 || page==2) && index==1) {
      const uint8_t level = page==1 ? volume : brightness;
      const uint8_t maximum = page==1 ? 63 : 255;
      char value[12];
      if (page==1) snprintf(value,sizeof(value),"%u/63",static_cast<unsigned>(level));
      else snprintf(value,sizeof(value),"%u%%",static_cast<unsigned>((static_cast<uint16_t>(level)*100U)/255U));
      canvas.drawRightString(value,w-17,y+13);
      const int barX=16, barY=y+36, barW=w-33;
      canvas.drawRect(barX,barY,barW,5,WHITE);
      const int fillW=(barW-2)*level/maximum;
      if(fillW>0) canvas.fillRect(barX+1,barY+1,fillW,3,WHITE);
    }
  }

  canvas.drawFastHLine(8,h-23,w-16,WHITE);
  canvas.setTextColor(WHITE,BLACK);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.drawString(page==1?"Drehen: Lautstaerke":(page==2?"Drehen: Helligkeit":"Drehen: Auswahl"),10,h-19);
}

}

bool DisplayDriver::begin(uint8_t initialBrightness) {
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
  brightness_ = initialBrightness < 10 ? 180 : initialBrightness;
  applyThemePalette(themeIndex_);
  device_.setBrightness(brightness_);
#if !defined(LILYGO_SI473X)
  ledcWrite(38, brightness_);
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

void DisplayDriver::setBrightness(uint8_t value) {
  if (value < 10) value = 10;
  brightness_ = value;
  device_.setBrightness(brightness_);
#if !defined(LILYGO_SI473X)
  ledcWrite(38, brightness_);
#endif
}

void DisplayDriver::setTheme(uint8_t index) {
  themeIndex_ = ThemeManager::normalize(index);
  applyThemePalette(themeIndex_);
  lastGeneration_ = UINT32_MAX;
  Serial.printf("[DISPLAY] theme=%s\\n", ThemeManager::at(themeIndex_).name);
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
      menuSelection != lastMenuSelection_ || volume != lastMenuVolume_ || layoutMode_ != lastMenuLayout_ || brightness_ != lastMenuBrightness_ || themeIndex_ != lastMenuTheme_;
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
  lastMenuBrightness_=brightness_;
  lastMenuTheme_=themeIndex_;
  if (menuOpen) {
    if(menuPage==6){if(useSprite_){sprite_.fillSprite(BLACK);drawFileMenu(sprite_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);sprite_.pushSprite(0,0);}else{device_.fillScreen(BLACK);drawFileMenu(device_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);}return;}
    if (useSprite_) {
      sprite_.fillSprite(BLACK);
      drawMenu(sprite_, menuPage, menuSelection, volume, brightness_, layoutMode_, s.modulation, themeIndex_);
      sprite_.pushSprite(0,0);
    } else {
      device_.fillScreen(BLACK);
      drawMenu(device_, menuPage, menuSelection, volume, brightness_, layoutMode_, s.modulation, themeIndex_);
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
    sprite_.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    textWidth = sprite_.textWidth(measuredText);
  } else {
    device_.setFont(&lgfx::fonts::FreeSansBold9pt7b);
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
  const bool logoSearching = !logoLocked && logoClient && logoClient->isLookupPending(logoKey);
  draw(s,radioReady,wifiConnected,lastInputEvent,inputEventCount,
       logoLocked ? logoData : nullptr,logoLocked ? logoLength : 0,logoIsJpg,logoSearching,
       radioTextOffset_, scrollRadioText, millis() < tuningUntilMs_);
  if (logoLocked) logoClient->unlockLogo();
}



template <typename Canvas>
void drawLayout(Canvas& canvas,const RadioSnapshot& s,bool radioReady,bool wifiConnected,
                const char* lastInputEvent,uint32_t inputEventCount,
                const uint8_t* logoData,size_t logoLength,bool logoIsJpg,bool logoSearching,
                int16_t radioTextOffset,bool scrollRadioText,bool frequencyBlue) {
  const int w=canvas.width();
  const int h=canvas.height();
  (void)lastInputEvent; (void)inputEventCount;

  canvas.fillScreen(BLACK);

  // Header: band, future RDS clock position, WLAN and estimated battery.
  text(canvas,s.bandName,10,5,GREEN,&lgfx::fonts::Font2);
  // Show only a valid RDS CT clock; otherwise keep the center completely blank.
  if (s.rdsClockValid) {
    char clockText[6];
    snprintf(clockText,sizeof(clockText),"%02u:%02u",
      static_cast<unsigned>(s.rdsClockHour),
      static_cast<unsigned>(s.rdsClockMinute));
    centered(canvas,clockText,w/2,5,WHITE,&lgfx::fonts::Font2);
  }
  text(canvas,wifiConnected?"WLAN+":"WLAN-",w-91,7,wifiConnected?CYAN:MUTED,&lgfx::fonts::Font0);

  uint32_t adcTotal=0;
  for(uint8_t i=0;i<10;++i) adcTotal+=analogRead(4);
  const float batteryVolts=(static_cast<float>(adcTotal)/10.0f)*1.702f/1000.0f;
  int batteryPercent=static_cast<int>((batteryVolts-3.30f)*100.0f/0.90f+0.5f);
  batteryPercent=constrain(batteryPercent,0,100);
  char batteryText[8];
  snprintf(batteryText,sizeof(batteryText),"%d%%",batteryPercent);
  text(canvas,batteryText,w-43,7,batteryPercent<=15?YELLOW:GREEN,&lgfx::fonts::Font0);
  canvas.drawFastHLine(8,25,w-16,DARK_GREEN);

  // Keep the station-name row inside the right-hand content area so it
  // never paints over the station-logo frame.
  constexpr int stationX = 124;
  // Font7 has a little leading side-bearing on a three-digit MHz value.
  // Move only the frequency slightly left; keep clear of the logo border.
  const int frequencyX = (s.frequencyKHz / 1000UL) >= 100UL ? 116 : 124;
  const char* station=s.stationName[0]?s.stationName:(radioReady?"--":"RADIO OFFLINE");
  canvas.fillRect(stationX,29,w-stationX-8,22,BLACK);
  text(canvas,station,stationX,30,WHITE,&lgfx::fonts::FreeSansBold9pt7b);

  // Left-hand station logo. Source files remain at their original resolution;
  // the renderer scales them proportionally into this display viewport.
  constexpr int logoX=8, logoY=27, logoW=108, logoH=108;
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
    canvas.fillRoundRect(logoX,logoY,logoW,logoH,6,BLACK);
    canvas.drawRoundRect(logoX,logoY,logoW,logoH,6,0x7BEF);
    if (logoSearching) {
      centered(canvas,"SEARCHING",logoX+logoW/2,logoY+39,WHITE,&lgfx::fonts::FreeSansBold9pt7b);
      centered(canvas,"LOGO...",logoX+logoW/2,logoY+59,WHITE,&lgfx::fonts::FreeSansBold9pt7b);
    } else {
      centered(canvas,logoData?"LOGO ERROR":"NO LOGO",logoX+logoW/2,logoY+47,WHITE,&lgfx::fonts::FreeSansBold9pt7b);
    }
  }

  // ATS Mini DEV TEF layout 7-segment Font7, scaled to fit the FM panel.
  char frequency[24];
  const bool isFm = s.modulation == RadioModulation::FM;
  if (isFm) {
    snprintf(frequency,sizeof(frequency),"%lu.%02lu",
      static_cast<unsigned long>(s.frequencyKHz/1000),
      static_cast<unsigned long>((s.frequencyKHz%1000)/10));
  } else {
    snprintf(frequency,sizeof(frequency),"%lu",static_cast<unsigned long>(s.frequencyKHz));
  }
  canvas.setTextColor(frequencyBlue?0x249F:FREQ_COLOR,BLACK);
  canvas.setFont(&lgfx::fonts::Font7);
  canvas.setTextSize(isFm ? 0.78f : 0.68f);
  canvas.drawString(frequency,frequencyX,55);
  canvas.setTextSize(1.0f);

  // FM uses MHz; LW/MW/SW/ALL use kHz like the ATS Mini DEV receiver.
  canvas.setTextColor(UNIT_COLOR,BLACK);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.drawRightString(isFm ? "MHz" : "kHz",w-8,84);

  // Stereo and PTY share the full row; the redundant RDS badge is removed.
  const int badgeY=103;
  canvas.drawRoundRect(124,badgeY,78,20,5,GREEN);
  centered(canvas,isFm?(s.stereo?"STEREO":"MONO"):(s.modulation==RadioModulation::AM?"AM":(s.modulation==RadioModulation::USB?"USB":"LSB")),163,badgeY+6,GREEN,&lgfx::fonts::Font0);

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
  constexpr int rtX=8,rtY=138,rtW=304,rtH=26;
  canvas.drawRoundRect(rtX,rtY,rtW,rtH,7,GREEN);
  const char* rt=s.radioText;
  // Slightly larger bold lettering makes the RT line easier to read while
  // keeping enough vertical room inside the 26 px ticker frame.
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(RDS_COLOR,BLACK);
  canvas.setClipRect(rtX+10,rtY+2,rtW-20,rtH-4);
  if(scrollRadioText){
    char ticker[72];
    snprintf(ticker,sizeof(ticker),"%s    ",s.radioText);
    const int tickerWidth=canvas.textWidth(ticker);
    canvas.drawString(ticker,rtX+14+radioTextOffset,rtY+4);
    canvas.drawString(ticker,rtX+14+radioTextOffset+tickerWidth,rtY+4);
  } else {
    canvas.drawString(rt,rtX+14,rtY+4);
  }
  canvas.clearClipRect();
}
void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady,bool wifiConnected,const char* lastInputEvent,uint32_t inputEventCount,const uint8_t* logoData,size_t logoLength,bool logoIsJpg,bool logoSearching,int16_t radioTextOffset,bool scrollRadioText,bool frequencyBlue) {
  // Keep the tested FM logo/RDS screen unchanged; AM-family bands use their
  // own TEF-inspired layout module.
  if (s.modulation != RadioModulation::FM) {
    if (useSprite_) {
      sprite_.fillSprite(BLACK);
      drawAmBandLayout(sprite_, s, radioReady, wifiConnected);
      sprite_.pushSprite(0, 0);
    } else {
      device_.fillScreen(BLACK);
      drawAmBandLayout(device_, s, radioReady, wifiConnected);
    }
    return;
  }
  if (useSprite_) {
    sprite_.fillSprite(BLACK);
    drawLayout(sprite_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoData,logoLength,logoIsJpg,logoSearching,radioTextOffset,scrollRadioText,frequencyBlue);
    sprite_.pushSprite(0,0);
  } else {
    device_.fillScreen(BLACK);
    drawLayout(device_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoData,logoLength,logoIsJpg,logoSearching,radioTextOffset,scrollRadioText,frequencyBlue);
  }
}
