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
