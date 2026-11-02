# DBFM v4.2.2 (DES) – Kompatibilitätsabgleich für ATS Mini OS

**Stand:** 2026-11-02  
**Zielgerät:** AMNVOLT ATS Mini V4, N16R8 (16 MB Flash, 8 MB OPI-PSRAM)  
**Bootmanager:** GNBD DBFM v4.2.2 (DES)  
**Arbeitszweig:** `dev/first-build-preparation`

## Kurzentscheidung

**Die bekannte `FlashSize=8M`-Einstellung des ATS-Mini-Referenzfirmwares ist kein Fehler und muss nicht allein wegen des 16-MB-Chips auf 16M geändert werden.** Das bekannte `ats-mini-dev`-Profil baut absichtlich mit 8M und `PSRAM=opi`; der Nutzer bestätigt, dass dieses Image läuft.

DBFM v4.2.2 selbst ist dagegen für N16R8 dokumentiert und wird laut Release-Dokumentation mit `FlashSize=16M` gebaut. Das ist kein Widerspruch: Der Bootmanager und die jeweilige App sind unterschiedliche Images mit unterschiedlichen Build-Einstellungen.

## Dokumentierte DBFM-Slots

Laut README des Releases [GNBD/ats-mini-dualboot v4.2.2](https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot):

| Bereich | Adresse | Größe |
|---|---:|---:|
| App0 | `0x010000` | 3.5 MiB |
| App1 | `0x390000` | 3.5 MiB |
| Recovery | `0x860000` | 1.625 MiB |
| LittleFS | `0xA00000` | 5.58 MiB |
| Settings ENV0 / ENV1 | `0xF9D000` / `0xFBD000` | je 128 KiB |

DBFM verlangt für App-Slots ein **ESP32-S3-OTA-App-Image**, kein zusammengeführtes Full-Flash-/Merged-Image. Die Release-Dokumentation warnt, dass Full-Flash-/Merged-Images die DES-Tabellen beschädigen können.

## Vergleich mit dem bekannten ATS-Mini-Build

Das bekannte Profil in `13HN2709/ats-mini-dev` verwendet:
- `FlashSize=8M`
- OSPI: `PSRAM=opi`
- QSPI: `PSRAM=enabled`
- eine 8-MB-Referenz-Partitionstabelle mit App-Slots ab `0x10000` und `0x310000`
- eine Workflow-Prüfung auf das vorhandene `ATSM`-Metadatum an Image-Offset 288

Unser ATS-Mini-OS-Scaffold verwendet ebenfalls `FlashSize=8M`, hat aber eine eigene Referenz-`partitions.csv` und gibt aktuell noch kein verifiziertes DBFM-OTA-Paket aus. Die beiden Projekte sind deshalb **nicht automatisch austauschbar**, auch wenn beide mit 8M gebaut werden.

## Wichtige technische Unterscheidung

Bei einem normalen App-Image, das in einen DBFM-Slot installiert wird, wird nicht die in unserem Repository liegende `partitions.csv` als neue physische Partitionstabelle geschrieben. Diese Datei beeinflusst den Build und die erzeugten Flash-/Merged-Artefakte; sie darf nicht mit der bereits installierten DBFM-Tabelle verwechselt werden. Ein App-Image kann trotzdem zur Laufzeit Partitionen anhand der tatsächlich installierten Tabelle suchen. Deshalb müssen alle verwendeten Partitionen und OTA-/Storage-Aufrufe mit der DBFM-Tabelle zusammenpassen.

Die 8M-Buildoption erklärt also, warum das bisherige Image auf einem 16-MB-Chip laufen kann. Sie beweist **nicht**, dass ein beliebiges neu gebautes Image automatisch DBFM-kompatibel ist.

## Blocker vor einer ersten Installation von ATS Mini OS

- [ ] Den tatsächlich erzeugten App-Binärdatei-Typ und die Image-Größe nach einem vom Nutzer gestarteten Build prüfen; die Datei muss in den 3.5-MiB-Slot passen.
- [ ] Klären, ob und welche DBFM-Firmware-Identitäts-/ATSM-Metadaten das neue OS benötigt. Die bekannte Referenz-Workflow-Prüfung erwartet `ATSM` an Offset 288; unser Scaffold erzeugt diese Metadaten aktuell nicht nachweislich.
- [ ] Runtime-Partitionzugriffe mit der DBFM-Tabelle prüfen: `nvs`, `otadata`, `app0`, `app1`, `littlefs`, `settings` und alle OTA-Aufrufe.
- [ ] Sicherstellen, dass nur das reine App-Image in App0 oder App1 geladen wird. Keine `partitions.bin`, kein Bootloader, kein `boot_app0.bin`, kein Merged-/Full-Flash-Image im App-Slot.
- [ ] Zuerst Build- und Binärdatei-Prüfungen dokumentieren. Erst danach einen separaten, ausdrücklich freigegebenen Installationsplan erstellen.

## Sicherheitsgrenzen

- DBFM bleibt bei v4.2.2; kein Bootloader-Upgrade.
- Die vorhandene Partitionstabelle, DES-Tabellen, `otadata`, Recovery und der bekannte funktionierende Slot werden nicht geändert.
- Diese Dokumentation hat weder einen Build noch eine GitHub Action oder einen Flash-/Geräteschreibvorgang ausgelöst.
- Die derzeitige ATS-Mini-OS-`partitions.csv` ist eine Build-Referenz und **nicht** als Tabelle zum Flashen auf das Gerät freigegeben.

## Quellen

- [DBFM v4.2.2 Release-Dokumentation](https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot)
- [Bekanntes ATS-Mini-Referenzprofil](https://github.com/13HN2709/ats-mini-dev)
- [Arduino-ESP32: Partition Table](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html)
