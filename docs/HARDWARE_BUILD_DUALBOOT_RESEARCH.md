# Hardware-, Build- und Dual-Boot-Recherche (ATS Mini V4, 16 MB Flash)

**Arbeitszweig:** `dev/first-build-preparation`  
**Stand:** 2026-11-01  
**Zweck:** Belegbare Grundlagen für zwei getrennte OS-Builds (ESP32-S3 OSPI und QSPI), ohne Build, Workflow-Start oder Flashvorgang.

## 1. Gesicherte Ausgangslage

- Das Zielgerät ist ein **AMNVOLT ATS Mini V4 mit 16 MB Flash** (Angabe des Nutzers).
- Der Nutzer verlangt mindestens zwei getrennte Artefakte: **OSPI** und **QSPI**. Weitere Hardwarevarianten wie LILYGO T-Embed gehören nicht in diesen Umfang.
- Die 16 MB Angabe beschreibt den Flash-Speicher, nicht die PSRAM-Art. Sie reicht **nicht** aus, um OSPI oder QSPI für ein konkretes Gerät zu bestimmen.
- Die vorhandene Referenzkonfiguration im privaten, nur lesend verwendeten Repository `13HN2709/ats-mini-dev` setzt in `ats-mini/sketch.yaml` für beide Profile `FlashSize=8M`:
  - `esp32s3-ospi`: `PSRAM=opi`
  - `esp32s3-qspi`: `PSRAM=enabled`
  - beide verwenden Arduino-ESP32 3.3.12, PU2CLR SI4735 2.1.8, LovyanGFX 1.2.32 sowie Async TCP 3.5.0, ESP Async WebServer 3.12.1 und NTPClient 3.2.1.
- Das ist ein wichtiger Unterschied: Ein physischer 16-MB-Flash-Chip und ein Build-Profil mit `FlashSize=8M` sind nicht automatisch widersprüchlich, weil Bootmanager, Partitionen und kompatible Firmware andere Grenzen setzen können. Für das neue OS darf `FlashSize=16M` nicht einfach angenommen oder erzwungen werden; die tatsächliche Flash-ID und das gewünschte Partitionslayout müssen berücksichtigt werden.

## 2. OSPI und QSPI eindeutig unterscheiden

Die ATS-Mini-Dokumentation und die Hardware-Installationsanleitung unterscheiden die Varianten über die PSRAM-Konfiguration:

- **OSPI / Octal PSRAM:** typischerweise 8 MB PSRAM; Arduino-Profil `PSRAM=opi`.
- **QSPI / Quad PSRAM:** typischerweise 2 MB PSRAM; Arduino-Profil `PSRAM=enabled`.
- Die Hersteller-/Chip-String-Angabe allein reicht nicht zuverlässig. Eine Hardware-Installationsanleitung beschreibt die Prüfung per `esptool flash-id` (bei esptool 4.x ggf. `flash_id`) anhand der `Features`-Zeile. Ein bereits bootendes Gerät kann außerdem unter Settings → About eine erkannte PSRAM-Menge anzeigen, sofern der aktuell laufende Build passt.

Quellen:
- ATS Mini Entwicklungsdokumentation: https://esp32-si4732.github.io/ats-mini/development.html
- ATS Mini Installationsanleitung (OSPI/QSPI, Flash-Profile und Flash-Artefakte): https://atsminiradio.com/en/guides/installing-firmware
- AMNVOLT ATS Mini V4/V4B Beschreibung: https://atsmini.github.io/

## 3. Erwartete Build-Artefakte

Für jeden der zwei Profile müssen wir die Artefakte nach dem Build getrennt halten und benennen, beispielsweise:

- `build/ospi/ats-mini-os-app.bin` – OSPI, nur wenn mit dem OSPI-Profil gebaut
- `build/qspi/ats-mini-os-app.bin` – QSPI, nur wenn mit dem QSPI-Profil gebaut

Die tatsächlichen Arduino-Exportnamen werden zunächst von Arduino CLI bestimmt; nach erfolgreichem Export müssen sie in variantenspezifische Ausgabeordner kopiert/umbenannt werden, ohne Inhalte zu verändern, und mit einer Textdatei/Manifest versehen werden, das Git-Commit, Profil, Framework- und Bibliotheksversion, Größe und SHA-256 festhält. Nie ein loses `.bin` ohne Variantennachweis verteilen.

Die offizielle ATS-Mini-Firmware-Dokumentation beschreibt die klassischen Einzeldateien (`bootloader.bin`, `partitions.bin`, `ats-mini.ino.bin`, teils `boot_app0.bin`) und ein zusammengeführtes Image. Das ist **nicht automatisch** die richtige Auslieferungsform für DiamondRadio-Dual-Boot.

## 4. Kritischer Punkt: DiamondRadio Dual-Boot

Aus dem früheren Projektverlauf ist festgehalten:

- ATS-Mini-Updates über DiamondRadio sollen nicht wie ein normales OTA-/Merged-Flash behandelt werden.
- Für den damaligen OSPI-DE-Test wurde die dritte Datei `ats-mini.ino.bin` ausgewählt und in DiamondRadio als **„Upload and Flash to APP0“** installiert.
- Slot A mit der bestehenden Firmware und Slot B mit DiamondRadio bleiben unangetastet.

Daher gilt für das neue OS:

1. Die Build-Konfiguration muss zuerst mit dem **tatsächlichen DiamondRadio-Partitionslayout** abgeglichen werden.
2. Der reine App-Build `ats-mini.ino.bin` ist der wahrscheinlich richtige erste Testgegenstand für „Upload and Flash to APP0“, aber erst nachdem die Größe des Images, der App0-Offset, die Slotgröße, die Bootmanager-Erwartungen und der gewählte Partition-CSV nachweislich passen.
3. **Nicht** `merged.bin`, Bootloader oder Partitionstabelle in einen App-Slot laden. Keine Flash-Adresse/Slotgröße raten.
4. Keine Änderungen an Bootloader, Partitionstabelle, Slot A, Slot B oder Bootmanager ohne ausdrückliche Freigabe und belegte Kompatibilität.
5. Der Nutzer hatte eine vollständige Sicherung namens `ATSMiniFULLBackup.nbin` mit 16 MB erwähnt. Vor einem späteren Flash-Schritt ist trotzdem zu prüfen, ob sie aktuell verfügbar, vollständig und außerhalb des Geräts gesichert ist.

Quelle zum Dual-Boot-Manager: https://diamondradio.web.app/atsmini.html  
Die öffentlich abrufbare Seite ist eine dynamische Web-App; die exakten Partitionsoffsets/Slotgrößen konnten daraus in dieser Recherche nicht belastbar extrahiert werden. Diese Werte bleiben ausdrücklich **offen**.

## 5. ATS Mini Hardware-Treiber, die für einen echten ersten Firmware-Build benötigt werden

Referenzwerte aus `13HN2709/ats-mini-dev` (noch nicht unabhängig gegen das konkrete Gerät gemessen):

- Display: 320×170; LovyanGFX, paralleler 8-Bit-Bus, nicht T-Embed-SPI.
- Display-Datenleitungen: GPIO39–42 und GPIO45–48; WR GPIO8; RD GPIO9; DC/RS GPIO7; CS GPIO6; Reset GPIO5.
- SI473x: Reset GPIO16; I²C SCL GPIO17; SDA GPIO18; Radio-LDO GPIO15.
- Encoder A/B GPIO2/GPIO1; Encoder-Taste GPIO21.
- Audio-Mute GPIO3; Verstärker-Enable GPIO10; Backlight GPIO38.
- Die Referenz initialisiert mehrere Displaycontroller-Varianten. Die originale ATS-Mini-Initialisierung muss sorgfältig übernommen bzw. modularisiert werden; ein generischer Display-Init ist nicht ausreichend.

## 6. Noch offene Nachweise vor dem ersten Build

- [ ] Relevante DiamondRadio-App-/Partitionskonfiguration aus vertrauenswürdigen Projektdateien oder einem vom Nutzer bereitgestellten Export auslesen.
- [ ] Prüfen, ob das konkrete AMNVOLT-Gerät PSRAM OSPI oder QSPI nutzt; der 16-MB-Flash allein sagt das nicht.
- [ ] Flash-Größe und Flash-ID aus einer vertrauenswürdigen Quelle für die Buildkonfiguration dokumentieren.
- [ ] Klären, ob die OS-Ausgabe für DiamondRadio ausschließlich ein App-Image pro Variante ist oder ob der Bootmanager weitere Metadaten erwartet.
- [ ] Prüfen, ob die Referenz-Buildprofile `FlashSize=8M` für das bestehende 16-MB-Gerät bewusst so gesetzt sind; nicht ohne Beleg auf 16M ändern.
- [ ] Aus dem Referenzprojekt nur die notwendigen Treiber/Radio-Core-Dateien samt Lizenzhinweisen und Abhängigkeiten übernehmen.
- [ ] Minimaler FM-Betrieb und Display initialisieren, bevor RDS, RadioDNS, Spectrum Scanner, Dual RX und Web Control integriert werden.
- [ ] OSPI- und QSPI-Builds unabhängig bauen und Größe/Hash/Profil dokumentieren.
- [ ] Vor erstem Build stoppen und Status melden; vor jeglichem Flashen separat Freigabe einholen.

## 7. Harte Sicherheitsgrenzen

- Kein Build und keine GitHub Actions in dieser Recherche ausgeführt.
- Kein Flashvorgang ausgeführt.
- Kein vorhandenes Firmware-Repository geändert.
- Keine Slot-/Partition-Offets erfunden.
- Kein Versprechen, dass ein Image bereits dual-boot-kompatibel ist.
