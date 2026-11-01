# Vorbereitung bis zum ersten Build

**Arbeitszweig:** `dev/first-build-preparation`  
**Status:** Recherche und Planungsnachweise gesammelt; noch nicht buildbereit.  
**Build/Actions/Flash:** Nicht ausgeführt.

## Gesicherte Quellcode-Erkenntnisse

Das private Referenzprojekt `13HN2709/ats-mini-dev` wurde nur lesend geprüft.

Quelle: `ats-mini/sketch.yaml`
- Arduino-ESP32 3.3.12
- PU2CLR SI4735 2.1.8
- LovyanGFX 1.2.32
- Async TCP 3.5.0, ESP Async WebServer 3.12.1, NTPClient 3.2.1
- getrennte Referenzprofile für ESP32-S3 OSPI und QSPI; nicht austauschbar, da PSRAM-Konfiguration unterschiedlich ist.

Quelle: `ats-mini/Common.h`
- Referenz-Displaygröße 320×170.
- Tuner-Klasse `SI4735_fixed`.
- ATS-Referenz-Pinbelegung: SI473x Reset GPIO16, I²C SCL GPIO17, SDA GPIO18, Radio-LDO GPIO15; Encoder A/B GPIO2/GPIO1, Encoder-Taste GPIO21; Audio-Mute GPIO3, Verstärker-Enable GPIO10, Backlight GPIO38.

Quelle: `ats-mini/Display.h`
- Das Referenzdisplay nutzt LovyanGFX mit parallelem 8-Bit-Bus für das ATS Mini (nicht die T-Embed-SPI-Konfiguration).
- Referenzdatenleitungen GPIO39–42 und GPIO45–48, WR GPIO8, RD GPIO9, RS/DC GPIO7, CS GPIO6, Reset GPIO5.
- Das Referenzprojekt unterstützt unterschiedliche Display-Controller-Varianten und erkennt sie beim Start. Diese spezielle Initialisierung sollte nicht durch eine vereinfachte, ungetestete Panel-Konfiguration ersetzt werden.

Diese Werte sind dokumentierte Referenzdaten, aber noch keine unabhängige Bestätigung der konkreten Hardware-Revision des Nutzers.

## Gewählte Buildstrategie – vorläufig

Arduino CLI mit dem Arduino-ESP32-Profil aus dem Referenzprojekt ist zunächst der risikoärmste Kandidat, weil die ATS-Mini-Hardwareanbindung dort vorhanden ist. Vor der ersten Kompilierung müssen jedoch Quellcode-/Lizenzübernahme, vollständige Dateiliste, Buildprofil, Partitionsanforderungen und Font-Renderer geprüft werden.

Es wird kein bestehender Firmware-Ordner ungeprüft kopiert. Das neue OS soll zunächst einen klar abgegrenzten, minimalen Radio-Grundbetrieb bekommen; RadioDNS und weitere Dienste folgen stufenweise.

## Font

Ausgewählter Kandidat: DSEGstended 14-Segment Regular, Upstream https://github.com/ZeusOfTheCrows/DSegstended. Die Projektseite nennt SIL Open Font License 1.1 und erweiterte lateinische Zeichen, darunter `ß`, sowie griechische und kyrillische Großbuchstaben.

Noch erforderlich:
- tatsächliche Quelldatei/Release-Datei identifizieren und lizenzkonform einbinden;
- prüfen, ob die gewünschte konkrete 14-Segment-Datei die erforderlichen Kleinbuchstaben und Zeichen `Ä Ö Ü ä ö ü ß À È É Ì Ò Ù à è é ì ò ù ç` enthält;
- Renderweg für LovyanGFX ermitteln (TTF direkt oder konvertierte Bitmap-Fonts); Speicherbedarf und Lesbarkeit für mehrere Größen abschätzen;
- erst dann in das Diagnose-Testlayout integrieren.

## Kritische offene Punkte vor dem ersten Build

1. Die konkrete Board-/PSRAM-Variante (OSPI oder QSPI) muss aus verlässlicher Quelle bestätigt werden. Nicht anhand von Vermutungen auswählen.
2. Der Bootmanager-/Partitionierungsweg für das neue OS ist unbekannt. Ein lokaler Compile-Test kann zwar möglich sein, darf aber nicht als flashbares oder dual-boot-kompatibles Image interpretiert werden.
3. Display- und Tuner-Initialisierung müssen in einen minimalen, eigenständigen Radio-Core-/UI-Aufbau übertragen werden, ohne die Referenzfirmware zu verändern.
4. RadioDNS wird erst nach stabilem FM/RDS-Grundbetrieb implementiert.
5. Ein reiner Build ist von Image-Export und Flashen zu trennen. Flashen bleibt ausdrücklich ausgeschlossen.

## Ablauf bis zur Freigabe des ersten Builds

- [x] Eigenen Arbeitszweig `dev/first-build-preparation` angelegt.
- [x] Referenzkonfiguration und relevante Hardware-/Displayhinweise lesend ermittelt.
- [x] DSEGstended als ersten Font-Kandidaten ausgewählt und Lizenzhinweis dokumentiert.
- [ ] Vollständige Hardware-/Buildprofilentscheidung belegen.
- [ ] Minimalen, modularen FM-/Display-Kern anlegen.
- [ ] Fontdatei und Glyphenumfang prüfen, Renderpfad integrieren oder sauber als separaten Folgeschritt isolieren.
- [ ] Konfigurations- und Lizenzprüfung abschließen.
- [ ] Übergabeprotokoll aktualisieren und diff prüfen.
- [ ] Vor dem ersten Build kurz anhalten und Nutzer informieren.

## Sicherheitsgrenzen

- Keine GitHub Actions auslösen.
- Keine Firmware flashen und keine aktiven Slots wechseln.
- Slot A, DiamondRadio Slot B und bestehende Firmware-Repositories bleiben unverändert.
- Keine Image-Offsets, Slotgrößen oder PSRAM-Variante raten.

## Manueller Compile-Workflow vorbereitet

Die Datei `.github/workflows/ats-mini-os-first-compile.yml` ist angelegt. Sie startet nur manuell, verlangt die explizite Eingabe `COMPILE` und erstellt getrennte Compile-Jobs für OSPI und QSPI. Sie enthält keinen Flash-Schritt.

Der Workflow ist **noch nicht gestartet**. Vor der ersten Freigabe ist der Review des Workflow-/Profilpfads erforderlich. Der bestehende 16-MB-Dual-Boot-Aufbau wird nicht verändert; die mitgelieferte Partitionstabelle ist nur eine 8-MB-Referenz und ausdrücklich keine Freigabe zum Flashen.


## CPU-Takt: feste Vorgabe (2026-11-01)

Das vom Nutzer bereitgestellte Foto des DiamondRadio-Menüs bestätigt direkt die drei angebotenen Stufen: 80 MHz, 160 MHz und 240 MHz. Diese Werte stimmen mit der Espressif-Dokumentation zum ESP32-S3 überein; ESP-IDF nennt 80/160/240 MHz als unterstützte maximale CPU-Frequenzwerte bei Dynamic Frequency Scaling. Quellen: https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32s3/api-reference/system/power_management.html und https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-cpu.h.

Verbindliche OS-Menülogik: `Auto`, `Taktstufe 1` = 80 MHz, `Taktstufe 2` = 160 MHz, `Taktstufe 3` = 240 MHz. Im Auto-Modus dürfen Waterfall, RadioDNS/WLAN und Bilddekodierung über den zentralen Performance Manager zeitlich begrenzte Leistungs-Leases anfordern. Erst wenn alle aktiven Anforderungen enden, darf Auto wieder absenken. Feste Stufen bleiben von solchen Anforderungen unberührt. Die im alten Chat erwähnte Regel „Encoderaktivität → 240 MHz, nach 10 Sekunden Inaktivität → 80 MHz“ ist für DiamondRadio noch nicht anhand des Originalquellcodes verifiziert.

Die derzeitige Laufzeitumschaltung bleibt deaktiviert, bis die Arduino-ESP32-3.3.12-API und die Stabilität auf OSPI und QSPI geprüft wurden. Ein unterstützter Taktwert allein beweist noch keine Stabilität der konkreten Hardware-/Timing-Konfiguration.