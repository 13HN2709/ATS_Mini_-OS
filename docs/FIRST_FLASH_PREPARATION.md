# Erstflash-Vorbereitung – ATS Mini OS (nur Review, kein Flash)

**Arbeitszweig:** `dev/first-build-preparation`  
**Stand:** 2026-11-02  
**Status:** Vorbereitungsprotokoll; keine Flash-Freigabe  
**Ausführung:** Nur Quellcode-/Konfigurationsprüfung. Kein Build, kein GitHub-Actions-Start, kein Gerätezugriff.

## Ziel

Den späteren Test des ATS Mini OS so vorbereiten, dass weder der bestehende Bootloader noch die vorhandene Firmware in Slot A oder DiamondRadio in Slot B versehentlich überschrieben werden. Das Gerät ist laut Nutzer ein ATS Mini V4 mit 16 MB Flash. Der bestehende OSPI-DE-Aufbau ist die bevorzugte Testbasis.

## Gesicherte Referenzwerte

Die Projektdatei `firmware/ats-mini-os/partitions.csv` enthält derzeit folgende Referenzaufteilung:

| Bereich | Offset | Größe |
|---|---:|---:|
| NVS | `0x9000` | `0x5000` |
| OTA-Metadaten (`otadata`) | `0xE000` | `0x2000` |
| APP0 | `0x10000` | `0x300000` |
| APP1 | `0x310000` | `0x300000` |
| LittleFS | `0x610000` | `0x1D0000` |
| Settings | `0x7E0000` | `0x10000` |
| Coredump | `0x7F0000` | `0x10000` |

Die Tabelle reicht bis `0x800000` (8 MiB). Sie ist als Referenz-/Build-Konfiguration dokumentiert, **nicht** als Nachweis, dass der installierte Dual-Boot-Manager exakt diese Tabelle verwendet.

Buildprofil OSPI-DE: ESP32-S3, Arduino-ESP32 3.3.12, `FlashSize=8M`, `PSRAM=opi`, `CPUFreq=80`, `FlashMode=qio`. Das ist die derzeit bevorzugte Profilwahl; die 16-MB-Angabe allein beweist weder das Bootmanager-Layout noch den PSRAM-Modus.

## Flash-Adresshinweise – noch nicht freigegeben

Für ein übliches Arduino-ESP32-Image werden häufig folgende Adressen verwendet: Bootloader `0x0`, Partitionstabelle `0x8000`, `boot_app0.bin` `0xE000`, App-Image `0x10000`. Diese Standardadressen sind **keine** Flash-Anweisung für das installierte DiamondRadio-Dual-Boot-System.

APP0/APP1-Offsets aus der CSV dürfen erst dann als tatsächliche Geräte-Offsets verwendet werden, wenn das reale Bootmanager-/Partitionslayout belegt ist. Insbesondere darf `otadata` nicht versuchsweise verändert werden, um einen Slot zu erzwingen.

## Freigabesperren vor jedem Flash-Versuch

- [ ] Aktuelles, vollständiges 16-MB-Backup ist außerhalb des Geräts vorhanden und lesbar; Dateiname/Größe/Prüfsumme protokolliert.
- [ ] Tatsächlicher Bootmanager und seine Version sind identifiziert.
- [ ] Reale Partitionstabelle und Zuordnung Slot A/Slot B sind durch einen verlässlichen Nachweis bestätigt.
- [ ] Der konkrete Installationsweg „Upload and Flash to APP0“ ist dokumentiert; es ist bekannt, welche Flash-Bereiche er schreibt.
- [ ] OSPI-/PSRAM-Profil des konkreten Geräts ist belegt.
- [ ] Exportiertes App-Image ist als App-Image identifiziert; Größe ist kleiner/gleich der nachgewiesenen Slotgröße.
- [ ] SHA-256, Git-Commit, Profil und Dateiname des geprüften Images sind festgehalten.
- [ ] Es ist bestätigt, dass der Installationsweg weder Bootloader, Partitionstabelle, `otadata`, Slot A noch Slot B außerhalb des ausdrücklich gewählten Ziels verändert.
- [ ] Nutzer erteilt eine **separate, ausdrückliche Freigabe** für genau diesen Flash-Schritt.

Solange ein Punkt offen ist: **nicht flashen**.

## Sicherer nächster Schritt

Nur lesend die DiamondRadio-Installations-/Recovery-Dokumentation und die im Projekt verfügbaren Image-Artefakte untersuchen. Danach die konkreten Schreibbereiche und Boot-Slot-Auswahl als kurze Entscheidungsvorlage dokumentieren. Keine Actions auslösen und keine Dateien auf das Gerät schreiben.

## Explizit ausgeschlossen

- Kein Bootloader-Flash.
- Keine Partitionstabelle schreiben.
- Keine Änderung von `otadata`.
- Kein Merged-Image als App-Slot-Image verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Kein neuer Build und kein Start von GitHub Actions im Rahmen dieses Reviews.
