# Erstflash-Vorbereitung – ATS Mini OS (nur Review, kein Flash)

**Arbeitszweig:** `dev/first-build-preparation`  
**Stand:** 2026-11-02  
**Status:** Vorbereitungsprotokoll; keine Flash-Freigabe  
**Ausführung:** Nur Quellcode-/Konfigurationsprüfung. Kein Build, kein GitHub-Actions-Start, kein Gerätezugriff.

## Ziel und tatsächlicher Ausgangspunkt

Der Nutzer bestätigt, dass auf seinem ATS Mini V4 der **GNBD ATS Mini Dualboot / DBFM in Version 4.2.2 (DES)** läuft. Der Nutzer bleibt vorerst bewusst bei **v4.2.2**; neuere Versionen werden nicht automatisch installiert. Der Link zur verwendeten Version ist: https://github.com/GNBD/ats-mini-dualboot/releases/tag/v4.2.2-dualboot

Das README am Tag `v4.2.2-dualboot` beschreibt das Zielgerät als ESP32-S3 **N16R8** (16 MB Flash + 8 MB PSRAM) und nennt ein 16-MB-Layout mit zwei App-Slots sowie einer Recovery-Partition. Der Default-Branch des Repositorys hat inzwischen eine neuere Version; für die hier geplante Arbeit ist aber der vom Nutzer bestätigte Stand v4.2.2 maßgeblich. Keine Bootloader-Aktualisierung ohne ausdrückliche Entscheidung des Nutzers.

## Nachgewiesene DBFM-v4.2.2-Angaben (aus README am Release-Tag)

Der README-Stand `v4.2.2-dualboot` dokumentiert:
- Bootmanager/Recovery: `0x860000`
- APP0: `0x010000`
- APP1: `0x390000`
- App-Slot-Größe: jeweils 3,5 MiB
- ESP32-S3 N16R8, 16 MB Flash und 8 MB PSRAM; FQBN nennt `FlashSize=16M`, `PSRAM=opi`, `CPUFreq=80`, `FlashMode=qio`
- ESP32-S3-OTA-App-Image für einen Slot verwenden; kein Merged-/Full-Flash-Image in einen App-Slot schreiben.
- Das Projekt warnt, dass externe Full-Flash-/Merged-Images die DES-Partitionstabellen beschädigen können.

Die dokumentierte 16-MB-Partitionstabelle (Table A / ENV0) lautet:

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

Table B unterscheidet sich laut README beim `settings`-Offset. DBFM verwendet eine DES-Konfiguration mit zwei Partitionstabellen und unterschiedlichen Settings-Regionen für die beiden Umgebungen. Die obigen Werte sind die vom Release-README dokumentierten Sollwerte, **noch kein direkter Auslesebeweis für die Partitionstabelle des konkreten Geräts**.

## Was aus dem ATS Mini OS Repository bekannt ist

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

Diese Tabelle reicht bis `0x800000` (8 MiB) und ist **nicht mit dem dokumentierten DBFM-v4.2.2-DES-Layout kompatibel**. Sie darf weder auf das Gerät geschrieben noch als Grundlage für Slot-Offsets, Flash-Image-Erstellung oder einen Flash-Befehl verwendet werden.

Das bisherige OSPI-Buildprofil im OS-Projekt nennt teils `FlashSize=8M`; das passt nicht zum im DBFM-v4.2.2-README angegebenen N16R8-Profil mit `FlashSize=16M`. Vor dem Erstflash muss das konkrete Buildprofil kontrolliert und für die reale Hardware nachvollziehbar festgelegt werden. Werte dürfen nicht nur zum Beseitigen eines Buildproblems geändert werden.

## Sicherheitsregeln für den Erstflash

- Nur ein **reines, passendes ESP32-S3-OTA-App-Image** darf als Kandidat für APP0/APP1 betrachtet werden.
- Das Image muss nachweislich in den ausgewählten 3,5-MiB-Slot passen.
- Bootloader (`0x0`), Partitionstabellen (`0x8000` und DBFM-DES-Tabelle), `otadata`, Recovery (`0x860000`) und Settings-Bereiche werden bei diesem App-Test nicht verändert.
- Keine Merged-/Full-Flash-Datei und keine Standard-Partitionstabelle des normalen ATS-Mini-Firmwareprojekts verwenden.
- Kein Slot-Wechsel und keine Änderung an der Bootauswahl auf Verdacht.
- Der konkrete Installationsweg muss geprüft werden, damit er wirklich nur den freigegebenen App-Slot schreibt. Die Dokumentation des DBFM nennt interne OTA/Web-UI und externe esptool-Installation als unterschiedliche Wege; deren tatsächliche Schreibbereiche müssen für unseren konkreten Kandidaten klar sein.
- Nutzer bleibt auf v4.2.2; kein Update auf den neueren Default-Branch.

## Freigabesperren vor jedem Flash-Versuch

- [ ] Das vorhandene vollständige Backup ist extern vorhanden und lesbar; Dateiname, Größe und SHA-256 sind protokolliert.
- [ ] Das Gerät meldet bzw. zeigt DBFM v4.2.2 (DES); keine ungeprüfte Annahme über einen anderen Bootmanager.
- [ ] Das tatsächliche aktive Partitionslayout und die DBFM-DES-Tabellenauswahl sind am Gerät oder durch einen zuverlässigen Dump bestätigt.
- [ ] OSPI/PSRAM-Hardwareprofil des Geräts ist bestätigt.
- [ ] Das Buildprofil ist mit N16R8 / 16 MB Flash / OPI-PSRAM konsistent; Abweichungen sind erklärt.
- [ ] Das exportierte App-Image ist eindeutig identifiziert; Dateiformat und Größe sind geprüft und passen in den Zielslot.
- [ ] SHA-256, Git-Commit, Buildprofil und Dateiname des geprüften Images sind festgehalten.
- [ ] Der ausgewählte Installationsweg und alle Schreibbereiche sind geprüft; keine nicht freigegebenen Bereiche werden verändert.
- [ ] Rückfallweg über das vollständige Backup ist verfügbar.
- [ ] Der Nutzer erteilt eine **separate, ausdrückliche Freigabe** für genau diesen Flash-Schritt.

Solange ein Punkt offen ist: **nicht flashen**.

## Sicherer nächster Schritt

Die DBFM-v4.2.2-Dokumentation ist jetzt die Referenz für Slot-Adressen und DES-Layout. Als nächstes nur lesend abgleichen: (1) die aktuelle ATS-Mini-OS-Buildkonfiguration, (2) die tatsächliche Imagegröße und (3) ob sich ein reines App-Image sicher in einen freien/ausgewählten Slot installieren lässt, ohne Bootmanager, Partitionen, Recovery oder `otadata` zu überschreiben. Vor einem Schreibschritt wird eine kurze Entscheidungsvorlage vorgelegt.

## Explizit ausgeschlossen

- Kein Upgrade von DBFM v4.2.2.
- Kein Bootloader-Flash.
- Keine Partitionstabelle schreiben oder reparieren.
- Keine Änderung von `otadata`.
- Kein Merged-/Full-Flash-Image als App-Slot-Image verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Kein neuer Build und kein Start von GitHub Actions im Rahmen dieses Reviews.
- Kein Flash und keine Änderung am laufenden Gerät.
