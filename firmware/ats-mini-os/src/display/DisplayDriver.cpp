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
  return true;
}

void DisplayDriver::render(const RadioSnapshot& s,bool radioReady,bool wifiConnected,const char* lastInputEvent,uint32_t inputEventCount) {
  if(!ready_)return;
  const uint32_t now=millis();
  if(s.generation==lastGeneration_ && now-lastDrawMs_<500)return;
  lastGeneration_=s.generation;
  lastDrawMs_=now;
  char logoKey[32] = {};
  char logoPath[48] = {};
  bool logoIsJpg = false;
  const RadioDnsFmIdentity identity = RadioDnsIdentity::buildFm(
    s.frequencyKHz, s.piCode, s.eccValid ? s.gcc : "");
  if (identity.status == RadioDnsIdentityStatus::Ready) {
    snprintf(logoKey, sizeof(logoKey), "%s_%s_%s",
      identity.gcc, identity.pi, identity.frequency);
    snprintf(logoPath, sizeof(logoPath), "/RDS/%s.png", logoKey);
    if (!LittleFS.exists(logoPath)) {
      snprintf(logoPath, sizeof(logoPath), "/RDS/%s.jpg", logoKey);
      if (LittleFS.exists(logoPath)) logoIsJpg = true;
      else logoPath[0] = 0;
    }
  }
  draw(s,radioReady,wifiConnected,lastInputEvent,inputEventCount,
       logoPath[0] ? logoPath : nullptr,logoIsJpg);
}


template <typename Canvas>
void drawLayout(Canvas& canvas,const RadioSnapshot& s,bool radioReady,bool wifiConnected,
                const char* lastInputEvent,uint32_t inputEventCount,
                const char* logoPath,bool logoIsJpg) {
  const int w=canvas.width();
  const int h=canvas.height();
  canvas.fillRect(7,4,116,24,BLACK);
  canvas.fillRect(198,4,w-205,24,BLACK);

  // Header: two decimal places for FM, with compact status indicators.
  char freq[20];
  snprintf(freq,sizeof(freq),"%lu.%02lu MHz",
    static_cast<unsigned long>(s.frequencyKHz/1000),
    static_cast<unsigned long>((s.frequencyKHz%1000)/10));
  text(canvas,freq,10,7,GREEN,&lgfx::fonts::Font2);

  text(canvas,"BT",204,8,CYAN,&lgfx::fonts::Font0);
  text(canvas,wifiConnected?"WiFi+":"WiFi-",226,8,wifiConnected?CYAN:0x7BEF,&lgfx::fonts::Font0);
  // ATS-Mini-DEV hardware reference: battery monitor is wired to GPIO4.
  // Match DEV's ADC correction factor and voltage thresholds; no fabricated percentage.
  uint32_t adcTotal = 0;
  for (uint8_t i = 0; i < 10; ++i) adcTotal += analogRead(4);
  const float batteryVolts = (static_cast<float>(adcTotal) / 10.0f) * 1.702f / 1000.0f;
  const bool charging = batteryVolts > 4.3f;
  const uint8_t batterySegments = batteryVolts < 3.68f ? 1 : batteryVolts < 3.78f ? 2 : batteryVolts < 3.88f ? 3 : 4;
  canvas.drawRoundRect(w-31,7,20,10,2,CYAN);
  canvas.fillRect(w-10,10,2,4,CYAN);
  if (charging) {
    canvas.fillRoundRect(w-29,9,16,6,1,CYAN);
    canvas.drawLine(w-22,8,w-24,12,BLACK);
    canvas.drawLine(w-24,12,w-21,12,BLACK);
    canvas.drawLine(w-21,12,w-23,15,BLACK);
  } else {
    for (uint8_t i = 0; i < batterySegments; ++i)
      canvas.fillRect(w-28 + i*4,9,3,6,batteryVolts < 3.68f ? YELLOW : GREEN);
  }

  canvas.drawFastHLine(10,31,w-20,DARK_GREEN);

  // Display a cached RadioDNS station logo when one has been downloaded.
  canvas.fillRoundRect(12,42,86,76,4,BLACK);
  canvas.drawRoundRect(11,41,88,78,5,DARK_GREEN);
  bool logoDrawn = false;
  if (logoPath && logoPath[0]) {
    if (logoIsJpg)
      logoDrawn = canvas.drawJpgFile(LittleFS, logoPath, 13, 43, 84, 74);
    else
      logoDrawn = canvas.drawPngFile(LittleFS, logoPath, 13, 43, 84, 74);
  }
  if (!logoDrawn) {
    centered(canvas,logoPath ? "LOGO ERR" : "NO LOGO",55,58,0x7BEF,&lgfx::fonts::Font2);
    centered(canvas,"RadioDNS",55,80,DARK_GREEN,&lgfx::fonts::Font0);
  }

  const char* station=s.stationName[0]?s.stationName:(radioReady?"Warte auf PS...":"RADIO OFFLINE");
  canvas.fillRect(105,39,w-115,27,BLACK);
  text(canvas,station,109,44,WHITE,&lgfx::fonts::Font2);

  canvas.fillRect(105,67,w-115,51,BLACK);
  text(canvas,"RT",109,69,YELLOW,&lgfx::fonts::Font0);
  const char* rt=s.radioText[0]?s.radioText:"Warte auf Radiotext...";
  text(canvas,rt,109,82,WHITE,&lgfx::fonts::Font2);

  // Standard RDS PTY labels (0-31), shown at the bottom of the layout.
  static const char* const ptyNames[32]={
    "NONE","NEWS","CURRENT AFFAIRS","INFO","SPORT","EDUCATION","DRAMA","CULTURE",
    "SCIENCE","VARIED","POP MUSIC","ROCK MUSIC","EASY LISTENING","LIGHT CLASSICS",
    "SERIOUS CLASSICS","OTHER MUSIC","WEATHER","FINANCE","CHILDREN","SOCIAL",
    "RELIGION","PHONE IN","TRAVEL","LEISURE","JAZZ","COUNTRY","NATIONAL MUSIC",
    "OLDIES","FOLK MUSIC","DOCUMENTARY","ALARM TEST","ALARM"
  };
  canvas.drawFastHLine(10,127,w-20,DARK_GREEN);
  canvas.fillRect(10,131,w-20,18,BLACK);
  text(canvas,"PTY",11,135,CYAN,&lgfx::fonts::Font0);
  text(canvas,ptyNames[s.pty<32?s.pty:0],36,133,GREEN,&lgfx::fonts::Font2);

  // Small diagnostic footer retained during alpha testing.
  char status[48];
  snprintf(status,sizeof(status),"%s  PI:%04X  %s",
    s.stereo?"ST":"MO",static_cast<unsigned>(s.piCode),
    s.rdsAvailable?"RDS":"NO RDS");
  canvas.fillRect(10,h-16,w-20,15,BLACK);
  text(canvas,status,11,h-13,0x7BEF,&lgfx::fonts::Font0);
}
void DisplayDriver::draw(const RadioSnapshot& s,bool radioReady,bool wifiConnected,const char* lastInputEvent,uint32_t inputEventCount,const char* logoPath,bool logoIsJpg) {
  if (useSprite_) {
    sprite_.fillSprite(BLACK);
    drawLayout(sprite_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoPath,logoIsJpg);
    sprite_.pushSprite(0,0);
  } else {
    device_.fillScreen(BLACK);
    drawLayout(device_,s,radioReady,wifiConnected,lastInputEvent,inputEventCount,logoPath,logoIsJpg);
  }
}
