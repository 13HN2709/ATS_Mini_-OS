# Erstflash-Vorbereitung – ATS Mini OS (nur Review, kein Flash)

**Arbeitszweig:** `dev/first-build-preparation`  
**Stand:** 2026-11-02  
**Status:** Vorbereitungsprotokoll; keine Flash-Freigabe  
**Ausführung:** Nur Quellcode-/Konfigurationsprüfung. Kein Build, kein GitHub-Actions-Start, kein Gerätezugriff.

## Ziel und tatsächlicher Ausgangspunkt

Der Nutzer bestätigt, dass auf seinem ATS Mini V4 der **GNBD ATS Mini Dualboot / DBFM in Version 4.2.2 (DES)** läuft. Der Nutzer bleibt vorerst bewusst bei **v4.2.2**; neuere Versionen werden nicht automatisch installiert. Referenz: https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot

Das Release-README beschreibt das Zielgerät als ESP32-S3 **N16R8** (16 MB Flash + 8 MB PSRAM) und ein 16-MB-Layout mit zwei App-Slots, Recovery und zwei DES-Partitionstabellen. Die dokumentierten Sollwerte sind unten zusammengefasst. Sie ersetzen nicht den Nachweis des tatsächlich aktiven Layouts auf dem konkreten Gerät.

## Nachgewiesene DBFM-v4.2.2-Angaben (aus README am Release-Tag)

- Recovery/Bootmanager: `0x860000`
- APP0: `0x010000`
- APP1: `0x390000`
- App-Slot-Größe: jeweils 3,5 MiB
- N16R8-Profil: `FlashSize=16M`, `PSRAM=opi`, `CPUFreq=80`, `FlashMode=qio`
- Als Slot-Datei ist ein ESP32-S3-OTA-App-Image vorgesehen, kein Merged-/Full-Flash-Image.
- Das Projekt warnt ausdrücklich, dass externe Full-Flash-/Merged-Images die DES-Tabellen beschädigen können.

Die dokumentierte Partitionstabelle (Table A / ENV0):

| Partition | Offset | Größe |
|---|---:|---:|
| Table A | `0x008000` | 4 KiB |
| `nvs` | `0x009000` | 20 KiB |
| `otadata` | `0x00E000` | 8 KiB |
| `app0` | `0x010000` | 3,5 MiB |
| `app1` | `0x390000` | 3,5 MiB |
| `ffat` | `0x710000` | 1,3 MiB |
| `recovery` | `0x860000` | 1,625 MiB |
| `littlefs` | `0xA00000` | 5,58 MiB |
| `rec_settings` | `0xF95000` | 32 KiB |
| `settings` (ENV0) | `0xF9D000` | 128 KiB |
| `settings` (ENV1) | `0xFBD000` | 128 KiB |
| `coredump` | `0xFDF000` | 128 KiB |
| Table B | `0xFFF000` | 4 KiB |

Table B unterscheidet sich laut README beim `settings`-Offset. Die tatsächliche Slot-/Tabellenauswahl muss vor dem Flash anhand des Geräts bzw. einer verlässlichen Auslesung bestätigt werden.

## Aktueller ATS Mini OS Stand: kritische Blocker

Die Datei `firmware/ats-mini-os/partitions.csv` ist weiterhin eine 8-MiB-Referenzpartitionierung, die bis `0x800000` reicht und andere App-Offsets verwendet. Sie ist **nicht mit dem DBFM-v4.2.2-DES-Layout kompatibel** und darf nicht auf das Gerät geschrieben werden.

Der manuelle Workflow `.github/workflows/ats-mini-os-first-compile.yml` baut aktuell explizit mit `FlashSize=8M`, prüft das erzeugte `flash_args` auf `8MB` und schreibt diese Angabe auch in die Build-Notizen. Das steht im Widerspruch zum dokumentierten N16R8-Profil von DBFM v4.2.2 (`FlashSize=16M`). Daher ist ein vorhandenes OSPI-DE-Buildartefakt aus diesem Workflow **nicht automatisch flashbereit** und darf nicht als freigegebenes App-Image betrachtet werden.

Außerdem beschreibt die Firmware-README den aktuellen Stand als Scaffold: Display-/Encoder-Ansteuerung und vollständige Radio-Firmware sind noch nicht implementiert. Ein erfolgreiches Kompilieren wäre deshalb lediglich ein technischer Zwischentest, kein Nachweis einer brauchbaren oder sicheren Erstflash-Firmware.

**Entscheidung:** Aktuell noch keine Flash-Datei freigeben. Erst Buildkonfiguration, Imageformat/Größe und tatsächliche Kompatibilität der App mit DBFM/DES gezielt korrigieren bzw. nachweisen.

## Sicherheitsregeln für den späteren App-Test

- Nur ein reines, passendes ESP32-S3-App-Image als Kandidat für APP0 oder APP1 betrachten.
- Imagegröße muss kleiner/gleich der bestätigten 3,5-MiB-Zielpartition sein.
- Bootloader (`0x0`), Partitionstabellen, `otadata`, Recovery (`0x860000`) und Settings-Bereiche nicht verändern.
- Keine Merged-/Full-Flash-Datei und keine Standard-Partitionstabelle des normalen ATS-Mini-Firmwareprojekts verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Den konkreten Installationsweg prüfen und sicherstellen, dass er ausschließlich den ausdrücklich gewählten App-Slot beschreibt.
- Nutzer bleibt auf DBFM v4.2.2; kein Update auf neuere Versionen.
- Vor einem Flash muss die Wiederherstellbarkeit des vollständigen Backups nachgewiesen werden.

## Freigabesperren vor jedem Flash-Versuch

- [ ] Das vorhandene vollständige Backup ist extern vorhanden und lesbar; Dateiname, Größe und SHA-256 sind protokolliert.
- [ ] Am Gerät ist DBFM v4.2.2 (DES) bestätigt.
- [ ] Das tatsächlich aktive Partitionslayout und die DES-Tabellenauswahl sind am Gerät oder durch einen zuverlässigen Dump bestätigt.
- [ ] OSPI/PSRAM-Hardwareprofil des Geräts ist bestätigt.
- [ ] Das Buildprofil ist mit N16R8 / 16 MB Flash / OPI-PSRAM konsistent.
- [ ] Das App-Image ist aus dem richtigen Commit/Profil gebaut; Dateiformat und Größe sind geprüft und passen in den Zielslot.
- [ ] SHA-256, Git-Commit, Buildprofil und Dateiname des geprüften Images sind protokolliert.
- [ ] Kompatibilität der App mit DBFM-v4.2.2-DES (insbesondere verwendete Partitionen/NVS/Filesystem) ist geprüft.
- [ ] Der ausgewählte Installationsweg und alle Schreibbereiche sind geprüft; keine nicht freigegebenen Bereiche werden verändert.
- [ ] Rückfallweg über das vollständige Backup ist verfügbar.
- [ ] Der Nutzer erteilt eine separate, ausdrückliche Freigabe für genau diesen Flash-Schritt.

Solange ein Punkt offen ist: **nicht flashen**.

## Nächste sichere Schritte

1. Den manuellen Compile-Workflow und die OSPI-Konfiguration so überarbeiten, dass das tatsächliche N16R8-Zielprofil klar und überprüfbar ist; die 8-MiB-Referenzpartitionstabelle darf nicht als Flash-Ziel übernommen werden.
2. Vor einer erneuten Kompilierung das aktuelle Firmware-Scaffold und die gewünschten Funktionen/Hardwareinitialisierung prüfen.
3. Erst nach expliziter Zustimmung des Nutzers den manuellen Compile-Workflow starten. Der Workflow wird nicht automatisch ausgelöst.
4. Artefakt danach nur lesend prüfen: Image-Header, Größe, SHA-256, Build-Commit, Profil und erzeugte Dateien. Noch keine Flash-Freigabe daraus ableiten.
5. Eine konkrete Flash-Entscheidungsvorlage mit Zielslot, erwarteten Schreibbereichen und Rückfallweg erstellen.

## Explizit ausgeschlossen

- Kein Upgrade von DBFM v4.2.2.
- Kein Bootloader-Flash.
- Keine Partitionstabelle schreiben oder reparieren.
- Keine Änderung an `otadata`.
- Kein Merged-/Full-Flash-Image als App-Slot-Image verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Kein automatischer Build-/GitHub-Actions-Start ohne ausdrückliche Zustimmung.
- Kein Flash und keine Änderung am laufenden Gerät.
