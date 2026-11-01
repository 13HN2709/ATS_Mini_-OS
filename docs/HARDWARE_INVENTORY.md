# Hardware-Inventar

**Status:** Offen – Angaben müssen anhand konkreter Hardware und Quellcode verifiziert werden.

## Zielgerät

- Hauptziel: ATS Mini V4
- Genannte MCU-Familie: ESP32-S3
- Genannte Flash-Größe: 16 MB, noch anhand des tatsächlichen Geräts/Layouts zu bestätigen

## Inventartabelle

| Bereich | Zu ermittelnde Angaben | Status |
|---|---|---|
| MCU | genaue Variante, Revision, CPU-/RAM-Konfiguration | Offen |
| Flash | Hersteller/Typ, tatsächliche Kapazität, Anschlussart | Offen |
| Speicherbus | OSPI oder QSPI, Controller und belegte Pins | Offen |
| Display | Controller, Auflösung, Bus, Pins, Rotation | Offen |
| Tuner | Chipvariante, Bus, Reset-/Enable-Pins | Offen |
| Audio | Codec/Verstärker, Steuerung, Signalweg | Offen |
| Encoder/Tasten | Pins, Pull-ups, Interrupts, Bedienlogik | Offen |
| Netzwerk | WLAN/Bluetooth-Nutzung und Modi | Offen |
| RTC/Zeit | vorhandene Hardware und Softwaredienste | Offen |
| Weitere Peripherie | Batterie, Ladezustand, Sensoren, LEDs | Offen |
| Bootsystem | Bootloader/Bootmanager und Startreihenfolge | Offen |

## Erforderliche Nachweise

Für jeden Eintrag möglichst Quellcodepfad, Schaltplan, Herstellerdokument oder reproduzierbare Messung angeben. Vermutungen ausdrücklich als solche kennzeichnen.

## Sicherheitsgrenzen

- Keine Hardwarepins allein aus ähnlich benannten Projekten übernehmen.
- Keine Flash- oder Boot-Experimente durchführen.
- Bestehende Firmware-Slots nicht verändern.
- OSPI/QSPI sind die einzigen im Projektumfang vorgesehenen Speicherbus-Optionen; tatsächliche Hardwareunterstützung ist noch zu prüfen.


## Referenzprojekt – vorläufige Erkenntnisse (nur lesend)

Referenz: https://github.com/13HN2709/ats-mini-dev, Branch `main`. Die folgenden Werte stammen aus dessen `ats-mini/Common.h` und `ats-mini/sketch.yaml`; sie sind Hinweise für ATS Mini V4, nicht automatisch freigegebene OS-Konfiguration.

- Display im Referenzprojekt: 320×170.
- Tuner: SI4735_fixed / SI4732-SI4735-Familie.
- I²C: SCL GPIO17, SDA GPIO18; Reset GPIO16; Radio-LDO GPIO15.
- Encoder: GPIO2/GPIO1; Push GPIO21.
- Audio mute GPIO3, Verstärker enable GPIO10, Backlight GPIO38.
- Arduino-ESP32 Plattform 3.3.12, PU2CLR SI4735 2.1.8, LovyanGFX 1.2.32.
- Referenzprofile sind getrennt in ESP32-S3 OSPI und QSPI. Sie setzen unterschiedliche PSRAM-Konfigurationen voraus.

**Vorläufig, noch nicht freigegeben:** Das Inventar muss vor einem echten Gerätebuild bestätigen, dass diese Pinbelegung zur konkreten ATS Mini V4 Hardware des Nutzers passt. Die Referenz-Profile dürfen nicht blind mit dem Boot-/Partitionslayout des neuen OS gleichgesetzt werden.


## Display – Referenzanalyse (nur lesend, noch nicht am Gerät bestätigt)

Referenzdatei: `13HN2709/ats-mini-dev/ats-mini/Display.h`, Branch `main`.

- LovyanGFX mit benutzerdefiniertem 8-Bit-Parallelbus für die Standard-ATS-Mini-Variante; eine separate SPI-Konfiguration existiert für `LILYGO_SI473X` und darf nicht mit dem ATS Mini V4 verwechselt werden.
- Referenzauflösung: 320×170 Pixel.
- Standard-ATS-Mini Parallelbus: WR GPIO8, RD GPIO9, RS/DC GPIO7; Daten D0–D7 auf GPIO39, GPIO40, GPIO41, GPIO42, GPIO45, GPIO46, GPIO47, GPIO48.
- Referenz-Panel: CS GPIO6, Reset GPIO5, Panelbreite 170, X-Offset 35.
- Die Referenz versucht die LCD-ID per `RDDID (0x04)` zu lesen und unterscheidet unter anderem ST7789-Varianten sowie die gespiegelte GC9307-ID. Die tatsächliche Controller-ID des konkreten Geräts ist **nicht** gemessen.
- Der Referenzbus enthält eine spezielle ESP32-S3 LCD_CAM/GPIO-Leseumschaltung. Deshalb nicht einfach eine generische LovyanGFX-Parallelbus-Konfiguration einsetzen.

## Encoder – Referenzanalyse (nur lesend, noch nicht am Gerät bestätigt)

Referenzdateien: `13HN2709/ats-mini-dev/ats-mini/Common.h`, `Rotary.h`, `Button.h` und `ats-mini.ino`.

- Standard-ATS-Mini-Referenz: Encoder A GPIO2, B GPIO1, Drucktaste GPIO21.
- Referenz verwendet die `Rotary`-Bibliothek; A/B-Reihenfolge und Half-Step-Einstellung beeinflussen die Drehrichtung bzw. Schrittfolge.
- Referenz-Tastenlogik: Entprellung 50 ms, Klick unter 500 ms, Short-Press ab 500 ms, Long-Press ab 2000 ms (Long-Press wird als gedrückt erkannt).
- Diese Zeitgrenzen sind Referenzverhalten, keine bestätigten Vorgaben für das neue OS. Die neue `EncoderInput`-Schnittstelle ist derzeit nur ein leerer Scaffold ohne GPIO-Konfiguration.

## Konsequenz für den nächsten Implementierungsschritt

1. Die Displaykonfiguration muss als eigener Treiber gekapselt werden; Controller-Erkennung und Panel-Initialisierung gehören in diesen Treiber.
2. Die erste Testansicht soll erst nach erfolgreicher Displayinitialisierung gezeichnet werden und keine Menüs oder Scanner voraussetzen.
3. Der Encoder wird separat implementiert und liefert Ereignisse (Drehen, kurzer Druck, langer Druck), ohne direkt UI oder Tuner zu steuern.
4. Pinwerte und Panel-ID bleiben als Referenz markiert, bis das konkrete ATS Mini V4 verlässlich bestätigt wurde. Keine Flash-/Partitionsänderung und kein Workflow-Start im Zuge dieser Inventarisierung.
