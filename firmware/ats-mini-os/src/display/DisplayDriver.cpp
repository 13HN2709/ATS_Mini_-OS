#include "DisplayDriver.h"
#include "ThemeManager.h"
#include "DisplayLayoutAM.h"
#include "DisplayLayoutHJB.h"
#include "../radio/BandManager.h"
#include "../menus/MenuSSB.h"
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

// ATS Mini DEV-style digital menu text. Orbitron does not need UTF-8
// umlaut glyphs: draw the base letter and add the two dots explicitly.
// German sharp-s is rendered as "SS", matching the DEV menu renderer.
template <typename Canvas>
int drawMenuDigitalText(Canvas& d,const char* value,int x,int y,uint16_t color,const lgfx::IFont* font) {
  d.setFont(font);
  d.setTextColor(color,BLACK);
  d.setTextDatum(TL_DATUM);
  int cursor=x;
  while(value && *value) {
    char glyph[2]={0,0};
    bool umlaut=false, sharpS=false;
    char ch=*value++;
    if(static_cast<uint8_t>(ch)==0xC3 && *value) {
      const uint8_t next=static_cast<uint8_t>(*value++);
      switch(next) {
        case 0x84: ch='A'; umlaut=true; break;
        case 0x96: ch='O'; umlaut=true; break;
        case 0x9C: ch='U'; umlaut=true; break;
        case 0xA4: ch='a'; umlaut=true; break;
        case 0xB6: ch='o'; umlaut=true; break;
        case 0xBC: ch='u'; umlaut=true; break;
        case 0x9F: ch='S'; sharpS=true; break;
        default: ch='?'; break;
      }
    }
    glyph[0]=ch;
    const int width=d.textWidth(glyph);
    d.drawString(glyph,cursor,y);
    d.drawString(glyph,cursor+1,y);
    if(sharpS) {
      d.drawString("S",cursor+width,y);
      d.drawString("S",cursor+width+1,y);
      cursor+=width*2;
      continue;
    }
    if(umlaut) {
      const int center=cursor+width/2;
      d.fillCircle(center-3,y+2,1,color);
      d.fillCircle(center+3,y+2,1,color);
    }
    cursor+=width;
  }
  return cursor-x;
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
    const char* items[]={"Zurück","Kopieren","Verschieben","Umbenennen","Löschen","Neuer Ordner"};
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
    text(c,"Drehen: Menü  |  Klick: Auswählen",8,h-18,YELLOW,&lgfx::fonts::Font0);
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
    text(c,mode==3?"ZIELORDNER: Klick waehlt":"Klick: Ordner oeffnen / Datei-Menü",8,44,YELLOW,&lgfx::fonts::Font0);
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
      if(ix==0) label=mode==3?"[DIESEN ORDNER WÄHLEN]":"[..] Ordner hoch";
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
    if(mode==3) text(c,"Drehen: Zielordner  |  Lang: Zurück",8,h-18,YELLOW,&lgfx::fonts::Font0);
    else {
      text(c,"Drehen: Blaettern  |  Kurz: Oeffnen",8,h-30,WHITE,&lgfx::fonts::Font0);
      if(mode==0) text(c,"Lang druecken: Dateimanager beenden",8,h-16,YELLOW,&lgfx::fonts::Font0);
    }
  }
  if(message&&message[0]) text(c,message,8,h-16,YELLOW,&lgfx::fonts::Font0);
}

template <typename Canvas>
void drawMenu(Canvas& canvas, uint8_t page, uint8_t selection, uint8_t volume, uint8_t brightness, uint8_t layoutMode, RadioModulation modulation, uint32_t frequencyKHz, uint8_t themeIndex, bool sliderAdjustmentActive) {
  const int w = canvas.width();
  const int h = canvas.height();
  // Main-menu labels intentionally match ATS Mini Dev verbatim.
  // A trailing * marks entries that are not implemented in ATS Mini OS yet.
  static const char* const mainItems[] = {
    "Modulation", "Bänder", "Lautstärke", "Keypad*", "DUAL RX*", "VFO A/B*",
    "Frequenz*", "BFO", "Schrittweite", "Suche*", "Scannen*", "Speicher*", "Rauschsperre*",
    "Bandbreite", "AGC/Dämpfung", "Auto-Lautstärke", "Soft-Mute", "Einstellungen",
    "VHF Spektrum*", "Gesamtes Bandspektrum*"
  };
  static const char* const volumeItems[] = {
    "Zurück", "Lautstärke", "Stumm (später)"
  };
  static const char* const displayItems[] = {
    "Zurück", "Display-Helligkeit", "Layout", "Farbschema"
  };
  static const char* const layoutGroupItems[] = { "Zurück", "UKW / FM", "LW / MW / KW" };
  static const char* const fmLayoutItems[] = { "Zurück", "DAB Style", "TEF6686 -FM Version" };
  static const char* const amLayoutItems[] = { "Zurück", "HJB" };
  static const char* const bfoItems[] = { "Zurück", "BFO einstellen" };
  static const char* const radioItems[] = {
    "Zurück", "Modulation", "Band wählen", "Bandbreite", "Schrittweite",
    "Empfangseinstellungen", "SSB / BFO"
  };
  static const char* const fmModulationItems[] = { "Zurück", "FM" };
  static const char* const amModulationItems[] = { "Zurück", "AM", "USB", "LSB" };
  static const char* const fmBandwidthItems[] = { "Zurück", "Auto", "110 kHz", "84 kHz", "60 kHz", "40 kHz" };
  static const char* const amBandwidthItems[] = { "Zurück", "1.0 kHz", "1.8 kHz", "2.0 kHz", "2.5 kHz", "3.0 kHz", "4.0 kHz", "6.0 kHz" };
  static const char* const ssbBandwidthItems[] = { "Zurück", "0.5 kHz", "1.0 kHz", "1.2 kHz", "2.2 kHz", "3.0 kHz", "4.0 kHz" };
  static const char* const radioSettingsItems[] = { "Zurück", "AGC Auto", "AGC Manuell 0", "AGC Manuell 8", "AGC Manuell 16", "AGC Manuell 24", "Soft Mute 0 dB", "Soft Mute 4 dB", "Soft Mute 8 dB", "Soft Mute 12 dB", "AVC 12", "AVC 30", "AVC 60", "AVC 90" };
  static const char* const themeItems[] = { "Zurück", "Classic", "TEF Blue", "Amber" };
  static const char* const fmStepItems[] = { "Zurück", "10 kHz (DX)", "50 kHz", "100 kHz", "200 kHz", "1 MHz" };
  static const char* const amStepItems[] = { "Zurück", "1 kHz", "5 kHz", "9 kHz", "10 kHz" };
  static const char* const wifiItems[] = {
    "Zurück", "WLAN-Modus (später)", "Web Control (später)"
  };
  static const char* const systemItems[] = {
    "Zurück", "CPU-Profil (später)", "Systeminfo (später)", "Dateimanager",
    "UTC+01:00 * (Sommer +02:00)", "UKW / FM Layouts", "KW / MW / LW Layouts"
  };
  const char* const* items = mainItems;
  uint8_t count = sizeof(mainItems) / sizeof(mainItems[0]);
  const char* title = "HAUPTMENÜ";
  switch (page) {
    case 1: items=volumeItems; count=sizeof(volumeItems)/sizeof(volumeItems[0]); title="AUDIO"; break;
    case 2: items=displayItems; count=sizeof(displayItems)/sizeof(displayItems[0]); title="ANZEIGE"; break;
    case 13: items=layoutGroupItems; count=sizeof(layoutGroupItems)/sizeof(layoutGroupItems[0]); title="LAYOUT GRUPPE"; break;
    case 14: items=fmLayoutItems; count=sizeof(fmLayoutItems)/sizeof(fmLayoutItems[0]); title="UKW / FM LAYOUTS"; break;
    case 15: items=amLayoutItems; count=sizeof(amLayoutItems)/sizeof(amLayoutItems[0]); title="KW / MW / LW LAYOUTS"; break;
    case 3: items=radioItems; count=sizeof(radioItems)/sizeof(radioItems[0]); title="RADIO"; break;
    case 4: items=wifiItems; count=sizeof(wifiItems)/sizeof(wifiItems[0]); title="WLAN"; break;
    case 5: items=systemItems; count=sizeof(systemItems)/sizeof(systemItems[0]); title="EINSTELLUNGEN"; break;
    case 7:
      if (modulation == RadioModulation::FM) { items=fmBandwidthItems; count=sizeof(fmBandwidthItems)/sizeof(fmBandwidthItems[0]); }
      else if (modulation == RadioModulation::AM) { items=amBandwidthItems; count=sizeof(amBandwidthItems)/sizeof(amBandwidthItems[0]); }
      else { items=ssbBandwidthItems; count=sizeof(ssbBandwidthItems)/sizeof(ssbBandwidthItems[0]); }
      title="BANDBREITE"; break;
    case 8: count=static_cast<uint8_t>(BandManager::count()+1); title="BAND WÄHLEN"; break;
    case 11: items=radioSettingsItems; count=(modulation==RadioModulation::FM?10:14); title="EMPFANG"; break;
    case 12: items=bfoItems; count=sizeof(bfoItems)/sizeof(bfoItems[0]); title="SSB / BFO"; break;
    case 16:
      if (BandManager::current().type == RadioBandType::FM) { items=fmModulationItems; count=2; }
      else { items=amModulationItems; count=(frequencyKHz >= 1800 && frequencyKHz <= 30000) ? 4 : 2; }
      title="MODULATION"; break;
    case 10: items=themeItems; count=sizeof(themeItems)/sizeof(themeItems[0]); title="FARBSCHEMA"; break;
    case 9:
      if (modulation == RadioModulation::FM) { items=fmStepItems; count=sizeof(fmStepItems)/sizeof(fmStepItems[0]); }
      else { items=amStepItems; count=sizeof(amStepItems)/sizeof(amStepItems[0]); }
      title="SCHRITTWEITE"; break;
    default: break;
  }

  // ATS Mini Dev XXL full-screen menu style: black canvas, blue frame,
  // cyan Orbitron-like large text, centered selected entry and page counter.
  const uint16_t menuBlue = 0x001F;
  const uint16_t menuCyan = 0x07FF;
  canvas.fillScreen(BLACK);
  canvas.fillRoundRect(3, 3, w-6, h-6, 5, BLACK);
  canvas.drawRoundRect(3, 3, w-6, h-6, 5, menuBlue);

  canvas.setTextColor(menuCyan, BLACK);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextDatum(TL_DATUM);
  canvas.drawString("ATS MINI OS", 12, 8);
  canvas.setTextDatum(TR_DATUM);
  canvas.drawString(title, w-12, 8);

  char counter[16];
  snprintf(counter, sizeof(counter), "%02u / %02u",
    static_cast<unsigned>(selection + 1), static_cast<unsigned>(count));
  canvas.setTextDatum(TR_DATUM);
  canvas.setFont(&lgfx::fonts::Font0);
  canvas.drawString(counter, w-12, 27);

  const char* selectedLabel = (selection < count) ? items[selection] : "";
  char displayLabel[72];
  snprintf(displayLabel, sizeof(displayLabel), "%s", selectedLabel);
  for (size_t i=0; displayLabel[i]; ++i) {
    if (displayLabel[i] >= 'a' && displayLabel[i] <= 'z')
      displayLabel[i] = static_cast<char>(displayLabel[i] - ('a' - 'A'));
  }
  if (page == 8 && selection > 0) {
    const RadioBandDefinition& band = BandManager::at(static_cast<uint8_t>(selection-1));
    snprintf(displayLabel, sizeof(displayLabel), "%s", band.name);
  }
  if ((page == 14 || page == 15) && selection > 0) {
    // Keep the currently selected layout marked, as in the Dev menu.
    const bool active = page == 14 ? (selection-1 == layoutMode) :
      (selection-1 == layoutMode);
    if (active) {
      char marked[72];
      snprintf(marked, sizeof(marked), "* %s", displayLabel);
      snprintf(displayLabel, sizeof(displayLabel), "%s", marked);
    }
  }

  // Match the ATS Mini DEV menu: Orbitron digital lettering plus
  // explicit UTF-8 handling for Ä/Ö/Ü/ä/ö/ü/ß.
  const lgfx::IFont* menuFont=&lgfx::fonts::Orbitron_Light_24;
  int labelWidth=canvas.textWidth(displayLabel,menuFont);
  if(labelWidth>w-28) {
    menuFont=&lgfx::fonts::FreeSansBold9pt7b;
    labelWidth=canvas.textWidth(displayLabel,menuFont);
  }
  const int labelY=(h/2)-12;
  const int labelX=(w-labelWidth)/2;
  drawMenuDigitalText(canvas,displayLabel,labelX,labelY,menuCyan,menuFont);

  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&lgfx::fonts::Font0);
  canvas.setTextColor(menuCyan, BLACK);
  canvas.drawString("DREHEN: AUSWÄHLEN", 12, h-18);
  canvas.setTextDatum(TR_DATUM);
  canvas.drawString("KLICK: OK", w-12, h-18);

  // Do not leak menu text alignment/scale into the next radio frame.
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextSize(1.0f);
  canvas.setFont(&lgfx::fonts::Font0);
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
  fmLayout_ = 0;
  amLayout_ = 0;
  Serial.println("[DISPLAY] active layout groups: FM/UKW and LW/MW/KW (AM/USB/LSB)");
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
  Serial.printf("[DISPLAY] theme=%s\n", ThemeManager::at(themeIndex_).name);
}

void DisplayDriver::setFmLayout(uint8_t mode) {
  fmLayout_ = mode <= 2 ? mode : 0;
  lastGeneration_ = UINT32_MAX;
  Serial.printf("[DISPLAY] FM layout: %s\\n", fmLayout_ == 1 ? "HJB" : (fmLayout_ == 2 ? "TEF6686 -FM Version" : "DAB Style"));
}

void DisplayDriver::setAmLayout(uint8_t mode) {
  amLayout_ = mode <= 1 ? mode : 0;
  lastGeneration_ = UINT32_MAX;
  Serial.printf("[DISPLAY] LW/MW/KW layout: %s\\n", amLayout_ == 1 ? "HJB" : "Standard LW/MW/KW");
}

void DisplayDriver::render(const RadioSnapshot& s,bool radioReady,bool wifiConnected,
                           const char* lastInputEvent,uint32_t inputEventCount,
                           StationLogoClient* logoClient, bool menuOpen,
                           uint8_t menuPage, uint8_t menuSelection, uint8_t volume, const char* filePath, FileManager::Entry* fileEntries, uint8_t fileCount, uint8_t fileSelection, uint8_t fileMode, uint8_t fileActionSelection, const char* fileSelectedName, const char* fileMessage, bool sliderAdjustmentActive) {
  if(!ready_)return;
  const uint32_t now=millis();
  const bool menuStateChanged = menuOpen != lastMenuOpen_ || menuPage != lastMenuPage_ ||
      menuSelection != lastMenuSelection_ || volume != lastMenuVolume_ || sliderAdjustmentActive != lastMenuSliderAdjustmentActive_ || (fmLayout_ + (amLayout_ << 1)) != lastMenuLayout_ || brightness_ != lastMenuBrightness_ || themeIndex_ != lastMenuTheme_;
  // Redraw the file manager promptly while the encoder is moving; this also
  // keeps the selection frame visually attached to the active row.
  const uint32_t redrawInterval = menuOpen ? ((menuPage==6) ? 20 : 500) : 40;
  if(s.generation==lastGeneration_ && now-lastDrawMs_<redrawInterval && !menuStateChanged)return;
  lastGeneration_=s.generation;
  lastDrawMs_=now;
  lastMenuOpen_=menuOpen;
  lastMenuSliderAdjustmentActive_=sliderAdjustmentActive;
  lastMenuPage_=menuPage;
  lastMenuSelection_=menuSelection;
  lastMenuVolume_=volume;
  lastMenuLayout_=static_cast<uint8_t>(fmLayout_ + (amLayout_ << 1));
  lastMenuBrightness_=brightness_;
  lastMenuTheme_=themeIndex_;
  if (menuOpen) {
    if(menuPage==6){if(useSprite_){sprite_.fillSprite(BLACK);drawFileMenu(sprite_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);sprite_.pushSprite(0,0);}else{device_.fillScreen(BLACK);drawFileMenu(device_,filePath,fileEntries,fileCount,fileSelection,fileMode,fileActionSelection,fileSelectedName,fileMessage);}return;}
    if (useSprite_) {
      sprite_.fillSprite(BLACK);
      drawMenu(sprite_, menuPage, menuSelection, volume, brightness_, s.modulation == RadioModulation::FM ? fmLayout_ : amLayout_, s.modulation, s.frequencyKHz, themeIndex_, sliderAdjustmentActive);
      sprite_.pushSprite(0,0);
    } else {
      device_.fillScreen(BLACK);
      drawMenu(device_, menuPage, menuSelection, volume, brightness_, s.modulation == RadioModulation::FM ? fmLayout_ : amLayout_, s.modulation, s.frequencyKHz, themeIndex_, sliderAdjustmentActive);
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
  // HJB intentionally has no sender-logo area and must not trigger logo lookups.
  const bool useFmStandard = fmLayout_ == 0 && s.modulation == RadioModulation::FM;
  const bool logoLocked = useFmStandard && logoClient && logoKey[0] &&
    logoClient->lockLogo(logoKey, logoData, logoLength, logoIsJpg);
  const bool logoSearching = useFmStandard && !logoLocked && logoClient && logoClient->isLookupPending(logoKey);
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

  // A menu may leave the shared LovyanGFX canvas right-aligned or scaled.
  // Restore the radio layout's drawing defaults before placing any content.
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextSize(1.0f);
  canvas.setFont(&lgfx::fonts::Font0);

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
  const bool isSsb = s.modulation == RadioModulation::LSB ||
                     s.modulation == RadioModulation::USB;
  if (isFm) {
    snprintf(frequency,sizeof(frequency),"%lu.%02lu",
      static_cast<unsigned long>(s.frequencyKHz/1000),
      static_cast<unsigned long>((s.frequencyKHz%1000)/10));
  } else if (isSsb) {
    // Show the SSB carrier frequency in kHz with three decimal places.
    // The BFO remains a separate audio offset; this is display formatting,
    // not a claim of 1 Hz RF synthesizer resolution.
    snprintf(frequency,sizeof(frequency),"%lu.000",
      static_cast<unsigned long>(s.frequencyKHz));
  } else {
    snprintf(frequency,sizeof(frequency),"%lu",static_cast<unsigned long>(s.frequencyKHz));
  }
  canvas.setTextColor(frequencyBlue?0x249F:FREQ_COLOR,BLACK);
  canvas.setFont(&lgfx::fonts::Font7);
  canvas.setTextSize(isFm ? 0.78f : (isSsb ? 0.56f : 0.68f));
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
template <typename Canvas>
void drawTef6686FmVersion(Canvas& c, const RadioSnapshot& s, bool wifiConnected) {
  constexpr uint16_t BG=0x0000, BLUE=0x001F, CYAN=0x07FF;
  constexpr uint16_t GREEN_TEF=0xBFE0, GREY=0x4208, YELLOW_TEF=0xFFE0, RED_TEF=0xF800;
  const int w=c.width();
  c.fillScreen(BG);

  // Header and radio status, matching the original ATS Mini Dev TEF6686 layout.
  c.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  c.setTextDatum(TL_DATUM);
  c.setTextColor(GREEN_TEF,BG);
  c.drawString(s.bandName,16,3);
  c.setFont(&lgfx::fonts::Font0);
  c.setTextColor(wifiConnected?CYAN:GREY,BG);
  c.drawRightString(wifiConnected?"WLAN+":"WLAN-",w-10,5);

  // Reception mode, stereo/mono and frequency unit.
  c.drawRoundRect(10,24,48,20,3,BLUE);
  c.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  c.setTextDatum(MC_DATUM);
  c.setTextColor(GREEN_TEF,BG);
  c.drawString("FM",34,34);

  c.drawRoundRect(10,48,48,20,3,BLUE);
  c.setTextColor(s.stereo?RED_TEF:0xFFFF,BG);
  c.drawString(s.stereo?"ST":"MO",34,58);

  c.drawRect(242,24,68,20,BLUE);
  c.setTextColor(GREEN_TEF,BG);
  c.drawString("MHz",276,34);

  // Large frequency digits use the ATS Mini's seven-segment Font7.
  char frequency[20];
  snprintf(frequency,sizeof(frequency),"%lu.%02lu",
    static_cast<unsigned long>(s.frequencyKHz/1000UL),
    static_cast<unsigned long>((s.frequencyKHz%1000UL)/10UL));
  c.setFont(&lgfx::fonts::Font7);
  c.setTextDatum(MR_DATUM);
  c.setTextColor(GREEN_TEF,BG);
  c.setTextSize(0.78f);
  c.drawString(frequency,238,43);
  c.setTextSize(1.0f);

  // Signal meter follows the Dev's 150 ms falling decay and green/yellow/red scale.
  constexpr int meterX=10,meterY=76,meterW=300,meterH=32;
  constexpr int barX=43,barY=87,barW=250,barH=7;
  const int signal=constrain(static_cast<int>(s.rssi),0,60);
  const int targetW=map(signal,0,60,0,barW);
  static int displayedW=0;
  static uint32_t lastMeterUpdate=0;
  const uint32_t now=millis();
  if(!lastMeterUpdate)lastMeterUpdate=now;
  const uint32_t elapsed=now-lastMeterUpdate;
  if(targetW>=displayedW){displayedW=targetW;lastMeterUpdate=now;}
  else if(elapsed>=150){displayedW-=static_cast<int>(elapsed/150)*5;if(displayedW<targetW)displayedW=targetW;lastMeterUpdate=now;}
  displayedW=constrain(displayedW,0,barW);
  c.drawRect(meterX,meterY,meterW,meterH,BLUE);
  c.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  c.setTextDatum(TL_DATUM);
  c.setTextColor(GREEN_TEF,BG);
  c.drawString("S",18,meterY+5);
  c.fillRect(barX,barY,barW,barH,GREY);
  const int greenEnd=barW*65/100,yellowEnd=barW*82/100;
  if(displayedW>0){
    c.fillRect(barX,barY,min(displayedW,greenEnd),barH,GREEN_TEF);
    if(displayedW>greenEnd)c.fillRect(barX+greenEnd,barY,min(displayedW,yellowEnd)-greenEnd,barH,YELLOW_TEF);
    if(displayedW>yellowEnd)c.fillRect(barX+yellowEnd,barY,displayedW-yellowEnd,barH,RED_TEF);
  }
  const int scaleValues[]={0,10,20,30,40,50,60};
  for(int i=0;i<7;++i){int x=barX+map(scaleValues[i],0,60,0,barW);c.drawLine(x,barY-2,x,barY+barH+2,CYAN);}

  // PS field: live RDS station name.
  constexpr int psX=10,psY=116,psW=300,psH=21;
  c.drawRect(psX,psY,psW,psH,BLUE);
  c.setFont(&lgfx::fonts::FreeSans9pt7b);
  c.setTextDatum(TL_DATUM);
  c.setTextColor(CYAN,BG);
  c.drawString("PS:",psX+5,psY+2);
  if(s.stationName[0]){
    c.setTextColor(GREEN_TEF,BG);
    c.setClipRect(psX+38,psY+1,psW-43,psH-2);
    c.drawString(s.stationName,psX+40,psY+2);
    c.clearClipRect();
  }

  // RT field: Dev-compatible prepared RadioText, clipped to the original field.
  constexpr int rtX=10,rtY=141,rtW=300,rtH=22;
  c.drawRect(rtX,rtY,rtW,rtH,BLUE);
  c.setFont(&lgfx::fonts::FreeSans9pt7b);
  c.setTextDatum(TL_DATUM);
  c.setTextColor(CYAN,BG);
  c.drawString("RT:",rtX+5,rtY+2);
  if(s.radioText[0]){
    c.setTextColor(GREEN_TEF,BG);
    c.setClipRect(rtX+38,rtY+1,rtW-43,rtH-2);
    c.drawString(s.radioText,rtX+40,rtY+2);
    c.clearClipRect();
  }
}

void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady,bool wifiConnected,const char* lastInputEvent,uint32_t inputEventCount,const uint8_t* logoData,size_t logoLength,bool logoIsJpg,bool logoSearching,int16_t radioTextOffset,bool scrollRadioText,bool frequencyBlue) {
  // FM and AM/SSB keep independent layout selections. The established
  // FM logo/RDS screen remains untouched as the DAB Style option.
  if (s.modulation == RadioModulation::FM) {
    if (fmLayout_ == 2) {
      if (useSprite_) { sprite_.fillSprite(BLACK); drawTef6686FmVersion(sprite_, s, wifiConnected); sprite_.pushSprite(0, 0); }
      else { device_.fillScreen(BLACK); drawTef6686FmVersion(device_, s, wifiConnected); }
      return;
    }
    if (fmLayout_ == 1) {
      if (useSprite_) { sprite_.fillSprite(BLACK); drawHJBLayout(sprite_, s, radioReady, wifiConnected); sprite_.pushSprite(0, 0); }
      else { device_.fillScreen(BLACK); drawHJBLayout(device_, s, radioReady, wifiConnected); }
      return;
    }
  } else {
    if (useSprite_) {
      sprite_.fillSprite(BLACK);
      if (amLayout_ == 0) drawAmBandLayout(sprite_, s, radioReady, wifiConnected);
      else drawHJBLayout(sprite_, s, radioReady, wifiConnected);
      sprite_.pushSprite(0, 0);
    } else {
      device_.fillScreen(BLACK);
      if (amLayout_ == 0) drawAmBandLayout(device_, s, radioReady, wifiConnected);
      else drawHJBLayout(device_, s, radioReady, wifiConnected);
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
