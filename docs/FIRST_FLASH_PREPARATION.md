# Erstflash-Vorbereitung – ATS Mini OS (nur Review, kein Flash)

**Arbeitszweig:** `dev/first-build-preparation`  
**Stand:** 2026-11-02  
**Status:** Vorbereitungsprotokoll; keine Flash-Freigabe  
**Ausführung:** Nur Quellcode-/Konfigurationsprüfung. Kein Build, kein GitHub-Actions-Start, kein Gerätezugriff.

## Ziel und tatsächlicher Ausgangspunkt

Der Nutzer verwendet **nicht mehr den DiamondRadio-Bootloader**. Er hat sein zuvor über die YDiamondRadio-Webseite erstelltes Backup des früheren Bootloader-/Partitionszustands zurückgespielt. Diese bekannte, wiederhergestellte Konfiguration ist die Ausgangsbasis für die weitere Planung.

Der Nutzer nennt das Gerät als AMNVOLT ATS Mini V4 mit 16 MB Flash. Für den ersten Flash-Test ist ausschließlich das bekannte **16-MB-OSPI-Setup** vorgesehen; die 8-MB-Konfiguration bleibt experimentell. Bootloader und Partitionierung sollen unangetastet bleiben. Der genaue Bootloadername und die tatsächlichen aktiven Partitionen müssen anhand des wiederhergestellten Setups belegt werden, statt sie zu erraten.

## Was aus dem Repository bekannt ist

Die Datei `firmware/ats-mini-os/partitions.csv` verwendet aktuell folgende 8-MiB-Referenzaufteilung:

| Bereich | Offset | Größe |
|---|---:|---:|
| NVS | `0x9000` | `0x5000` |
| OTA-Metadaten (`otadata`) | `0xE000` | `0x2000` |
| APP0 | `0x10000` | `0x300000` |
| APP1 | `0x310000` | `0x300000` |
| LittleFS | `0x610000` | `0x1D0000` |
| Settings | `0x7E0000` | `0x10000` |
| Coredump | `0x7F0000` | `0x10000` |

Die Tabelle reicht bis `0x800000` (8 MiB). Sie entspricht der dokumentierten ATS-Mini-DEV-Referenz, ist aber **nicht** als bestätigte Partitionstabelle des wiederhergestellten 16-MB-Setups nachgewiesen. Sie darf nicht ungeprüft auf das Gerät geschrieben oder als sichere APP0-/APP1-Zuordnung behandelt werden.

Das bevorzugte Buildprofil für die Zielhardware ist OSPI-DE: ESP32-S3, Arduino-ESP32 3.3.12, `FlashSize=8M`, `PSRAM=opi`, `CPUFreq=80`, `FlashMode=qio`. Die Projektanforderung, für den späteren Test das bekannte 16-MB-OSPI-Setup beizubehalten, bedeutet nicht, dass wir die FlashSize- oder Partitionswerte eigenmächtig ändern dürfen.

## Flash-Adresshinweise – nicht als Schreibanweisung verwenden

Für übliche Arduino-ESP32-Images kommen häufig die Adressen Bootloader `0x0`, Partitionstabelle `0x8000`, `boot_app0.bin` `0xE000` und App `0x10000` vor. Diese Standardwerte belegen nicht, welche Adressen der wiederhergestellte Bootloader verwendet.

Vor einem Flash-Schritt müssen wir anhand der wiederhergestellten Boot-/Partitionskonfiguration und des tatsächlichen Installationswegs nachweisen:
- welcher Bootloader und welches Partitionslayout aktiv sind;
- welche Partitionen/Slots existieren und wie sie ausgewählt werden;
- welche Bereiche das gewählte Flash-Verfahren tatsächlich schreibt;
- ob ein reines App-Image in den vorgesehenen Slot passt und vom vorhandenen Bootpfad akzeptiert wird.

Insbesondere dürfen `otadata`, Bootloader und Partitionstabelle nicht versuchsweise verändert werden, um einen Slot zu erzwingen.

## Freigabesperren vor jedem Flash-Versuch

- [ ] Die zurückgespielte Sicherung ist vollständig, extern vorhanden und lesbar; Dateiname, Größe und SHA-256 sind protokolliert.
- [ ] Der **tatsächlich wiederhergestellte Bootloader** und seine Version sind identifiziert.
- [ ] Das aktive Partitionslayout des wiederhergestellten 16-MB-Setups ist durch einen verlässlichen Nachweis bestätigt.
- [ ] Die Zuordnung der App-Slots und die Boot-/Recovery-Logik sind dokumentiert.
- [ ] Der konkrete Flash-/Recovery-Weg und seine Schreibbereiche sind bekannt.
- [ ] Das OSPI-/PSRAM-Profil der konkreten Hardware ist belegt.
- [ ] Das exportierte App-Image ist eindeutig identifiziert; seine Größe ist kleiner/gleich der nachgewiesenen Zielpartition.
- [ ] SHA-256, Git-Commit, Profil und Dateiname des geprüften Images sind festgehalten.
- [ ] Es ist nachgewiesen, dass der Installationsweg keine nicht freigegebenen Bereiche verändert.
- [ ] Der Nutzer erteilt eine **separate, ausdrückliche Freigabe** für genau diesen Flash-Schritt.

Solange ein Punkt offen ist: **nicht flashen**.

## Sicherer nächster Schritt

Die zuvor wiederhergestellte Konfiguration als maßgeblich behandeln und die Hinweise auf DiamondRadio als aktiven Bootloader aus der Erstflash-Planung entfernen. Anschließend nur lesend die verfügbaren Backup-/Recovery-Informationen und das passende Flash-Verfahren untersuchen. Danach eine kurze Entscheidungsvorlage mit nachgewiesenen Schreibbereichen, offenem Prüfbedarf und klarer Rückfallstrategie erstellen.

## Explizit ausgeschlossen

- Kein DiamondRadio-Bootloader als aktuelle Ausgangsbasis voraussetzen.
- Kein Bootloader-Flash.
- Keine Partitionstabelle schreiben.
- Keine Änderung von `otadata`.
- Kein Merged-Image als App-Slot-Image verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Kein neuer Build und kein Start von GitHub Actions im Rahmen dieses Reviews.
