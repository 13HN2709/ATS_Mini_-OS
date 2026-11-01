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
