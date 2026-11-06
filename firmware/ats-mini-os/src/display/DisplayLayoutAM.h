#pragma once
#include <Arduino.h>
#include "../radio/RadioSnapshot.h"

// Dedicated AM/LW/MW/SW/ALL screen. Kept separate so the tested FM layout
// (including station logos and the RDS ticker) remains untouched.
template <typename Canvas>
void drawAmBandLayout(Canvas& canvas, const RadioSnapshot& s, bool radioReady,
                      bool wifiConnected) {
  constexpr uint16_t BLACK = 0x0000;
  constexpr uint16_t WHITE = 0xFFFF;
  constexpr uint16_t BLUE = 0x001F;
  constexpr uint16_t CYAN = 0x07FF;
  constexpr uint16_t GREEN = 0xBFE0;
  constexpr uint16_t GREY = 0x4208;
  constexpr uint16_t YELLOW = 0xFFE0;
  constexpr uint16_t RED = 0xF800;
  const int w = canvas.width();
  const int h = canvas.height();
  (void)h;

  canvas.fillScreen(BLACK);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(GREEN, BLACK);
  canvas.drawString(s.bandName, 12, 4);
  canvas.setFont(&lgfx::fonts::Font0);
  canvas.setTextColor(wifiConnected ? CYAN : GREY, BLACK);
  canvas.drawRightString(wifiConnected ? "WLAN+" : "WLAN-", w - 10, 7);
  canvas.drawFastHLine(8, 24, w - 16, BLUE);

  const char* mode = s.modulation == RadioModulation::AM ? "AM" :
                     s.modulation == RadioModulation::USB ? "USB" : "LSB";
  canvas.drawRoundRect(10, 34, 54, 22, 4, BLUE);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(GREEN, BLACK);
  canvas.drawCenterString(mode, 37, 38);

  canvas.setFont(&lgfx::fonts::Font7);
  canvas.setTextColor(GREEN, BLACK);
  canvas.setTextSize(s.frequencyKHz >= 10000 ? 0.62f : 0.78f);
  char frequency[20];
  snprintf(frequency, sizeof(frequency), "%lu",
           static_cast<unsigned long>(s.frequencyKHz));
  canvas.drawCenterString(frequency, 190, 39);
  canvas.setTextSize(1.0f);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(WHITE, BLACK);
  canvas.drawRightString("kHz", w - 10, 76);

  // Compact TEF-style signal meter.
  canvas.drawRoundRect(10, 78, w - 20, 31, 4, BLUE);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(GREEN, BLACK);
  canvas.drawString("S", 18, 84);
  const int barX = 43, barY = 91, barW = w - 62, barH = 8;
  canvas.fillRect(barX, barY, barW, barH, GREY);
  const int signal = constrain(s.rssi, 0, 60);
  const int fill = map(signal, 0, 60, 0, barW);
  const int greenEnd = barW * 65 / 100;
  const int yellowEnd = barW * 82 / 100;
  if (fill > 0) {
    canvas.fillRect(barX, barY, min(fill, greenEnd), barH, GREEN);
    if (fill > greenEnd)
      canvas.fillRect(barX + greenEnd, barY, min(fill, yellowEnd) - greenEnd, barH, YELLOW);
    if (fill > yellowEnd)
      canvas.fillRect(barX + yellowEnd, barY, fill - yellowEnd, barH, RED);
  }
  char signalText[20];
  snprintf(signalText, sizeof(signalText), "%d dB", s.rssi);
  canvas.setFont(&lgfx::fonts::Font0);
  canvas.setTextColor(WHITE, BLACK);
  canvas.drawRightString(signalText, w - 17, 96);

  // Frequency-dependent operating hints are recommendations only. They never
  // change the receiver mode or tune the radio automatically.
  const uint32_t f = s.frequencyKHz;
  const char* hint = "AM / SHORTWAVE";
  if ((f >= 14070 && f <= 14077) || (f >= 7070 && f <= 7079) ||
      (f >= 10132 && f <= 10137) || (f >= 21070 && f <= 21078) ||
      (f >= 28070 && f <= 28080)) hint = "FT8 - USB";
  else if ((f >= 1830 && f <= 1837) || (f >= 3500 && f <= 3560) ||
           (f >= 7000 && f <= 7040) || (f >= 14000 && f <= 14069) ||
           (f >= 21000 && f <= 21069)) hint = "CW - MORSE";
  else if ((f >= 3600 && f <= 3800) || (f >= 7060 && f <= 7200)) hint = "QSO - LSB";
  else if ((f >= 14101 && f <= 14350) || (f >= 18111 && f <= 18168) ||
           (f >= 21110 && f <= 21450) || (f >= 28300 && f <= 29000)) hint = "QSO - USB";

  canvas.drawRoundRect(10, 119, w - 20, 43, 6, BLUE);
  canvas.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  canvas.setTextColor(CYAN, BLACK);
  canvas.drawCenterString("BANDPLAN HINT", w / 2, 123);
  canvas.setTextColor(WHITE, BLACK);
  canvas.drawCenterString(hint, w / 2, 140);
  if (!radioReady) {
    canvas.setFont(&lgfx::fonts::Font0);
    canvas.setTextColor(YELLOW, BLACK);
    canvas.drawCenterString("RADIO OFFLINE", w / 2, 153);
  }
}
