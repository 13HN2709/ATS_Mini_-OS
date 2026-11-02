# DBFM v4.2.2 (DES) – Kompatibilitätsabgleich für ATS Mini OS

**Stand:** 2026-11-02  
**Zielgerät:** AMNVOLT ATS Mini V4, N16R8 (16 MB Flash, 8 MB OPI-PSRAM)  
**Bootmanager:** GNBD DBFM v4.2.2 (DES)  
**Arbeitszweig:** `dev/first-build-preparation`

## Kurzentscheidung

**Die abweichende 8-MiB-Partitionstabelle ist nicht allein ein Beweis für Inkompatibilität.** Der Nutzer bestätigt aus praktischer Erfahrung, dass `ats-mini-dev` mit dieser Konfiguration in beiden DBFM-Slots lief und Daten auf LittleFS (u. a. EiBi-Schedules; außerdem `memories.json` in seiner verwendeten Version) speichern bzw. lesen konnte. Diese Beobachtung ist wichtiger Praxisnachweis als ein reiner Tabellenvergleich.

Daraus folgt aber nicht automatisch, dass das aktuelle ATS-Mini-OS-Scaffold kompatibel ist: Es besitzt derzeit noch nicht dieselbe vollständige Storage-Implementierung wie `ats-mini-dev`. Die Referenz muss deshalb bei Bedarf in kleinen, gezielten Schritten übernommen und geprüft werden. Die funktionierende Partitionstabelle wird **nicht auf Verdacht geändert**.

## Dokumentierte DBFM-Slots

Laut README des Releases [GNBD/ats-mini-dualboot v4.2.2](https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot):

| Bereich | Adresse | Größe |
|---|---:|---:|
| App0 | `0x010000` | 3.5 MiB |
| App1 | `0x390000` | 3.5 MiB |
| Recovery | `0x860000` | 1.625 MiB |
| LittleFS | `0xA00000` | 5.58 MiB |
| Settings ENV0 / ENV1 | `0xF9D000` / `0xFBD000` | je 128 KiB |

DBFM verlangt für App-Slots ein ESP32-S3-OTA-App-Image, kein zusammengeführtes Full-Flash-/Merged-Image. Die Release-Dokumentation warnt, dass Full-Flash-/Merged-Images die DES-Tabellen beschädigen können.

## Vergleich mit `ats-mini-dev`

Quellcode in `13HN2709/ats-mini-dev` (Branch `main`) bestätigt:

- `ats-mini/partitions.csv` nutzt `FlashSize=8M` als Referenzlayout mit App0 `0x10000 / 0x300000`, App1 `0x310000 / 0x300000`, LittleFS `0x610000 / 0x1d0000`, Settings/NVS `0x7e0000 / 0x10000` und Coredump `0x7f0000 / 0x10000`.
- `ats-mini/Storage.h` setzt `STORAGE_PARTITION "settings"`.
- `ats-mini/Storage.cpp` nutzt Arduino `Preferences` mit dieser Partition für Einstellungen, Bänder und Memories.
- `ats-mini/EIBI.cpp` verwendet `LittleFS` und die Datei `/schedules.bin` (mit temporärer Datei `/schedules.tmp`).
- Der Nutzer bestätigt, dass seine laufende `ats-mini-dev`-Version in beiden Slots funktionierte und LittleFS-Dateien einschließlich EiBi-Schedules und `memories.json` nutzte.

Die `partitions.csv` des ATS-Mini-OS-Scaffolds ist aktuell identisch mit der Referenzdatei aus `ats-mini-dev`. **Nicht** identisch ist sie mit der dokumentierten DBFM-4.2.2-Partitionsübersicht. Die praktische Erfahrung zeigt, dass dieser Unterschied allein kein hinreichender Grund ist, das Referenzlayout zu ändern; wir dürfen daraus aber auch keine automatische Kompatibilitätsgarantie für andere Firmware ableiten.

## Vergleich mit dem aktuellen ATS-Mini-OS-Scaffold und Build #56

Der aktuelle Scaffold enthält noch keine gleichwertige Storage-Schicht: In `ats-mini-os.ino` werden aktuell weder LittleFS noch Preferences initialisiert oder verwendet. Damit sind `memories.json`, EiBi-Schedules und die vorhandene Speicherlogik noch nicht in das neue OS übernommen.

Build #56: [GitHub Actions Run 41476309903](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41476309903)

| Prüfung | OSPI | QSPI |
|---|---:|---:|
| Build | erfolgreich | erfolgreich |
| App-Binärdatei | 956.624 Bytes | 954.368 Bytes |
| ESP-Image-Magic | `E9` | `E9` |
| App-Größe unter 3,5 MiB | ja | ja |
| Descriptor-Projekt | `arduino-lib-builder` | `arduino-lib-builder` |
| `ATSM` bei Offset 288 | nicht vorhanden | nicht vorhanden |

Der fehlende `ATSM`-Marker wird dokumentiert, ist anhand der DBFM-v4.2.2-README aber nicht als allgemeine Installationsvoraussetzung bestätigt. Der Descriptor-Name ist kein nachgewiesener Blocker. Die erfolgreiche Kompilierung und passende Größe beweisen nicht, dass das Scaffold bereits eine vollständige Radio-Firmware ist.

## Wichtige technische Unterscheidung

Bei einem normalen App-Image, das in einen DBFM-Slot installiert wird, wird nicht die in unserem Repository liegende `partitions.csv` als neue physische Partitionstabelle geschrieben. Diese Datei beeinflusst den Build und erzeugte Flash-/Merged-Artefakte; sie darf nicht mit der bereits installierten DBFM-Tabelle verwechselt werden. Runtime-Zugriffe hängen davon ab, welche Partitionen die Firmware sucht und welche Labels/Adressen das Framework verwendet.

## Nächste Schritte

- [x] Referenz-Partitionstabelle von `ats-mini-dev` mit der ATS-Mini-OS-Datei vergleichen: identisch.
- [x] Referenz-Speichermechanismen lesen: Preferences über `settings`, EiBi über LittleFS `/schedules.bin`.
- [x] Build #56 anhand der erzeugten OSPI-/QSPI-Artefakte auswerten.
- [ ] Die benötigten Storage-Funktionen von `ats-mini-dev` mit der Architektur des OS abgleichen, bevor Code übernommen wird.
- [ ] LittleFS-Initialisierung und Fehlerverhalten für das Ziel-OS festlegen; bestehende Dateien nicht formatieren oder löschen.
- [ ] Preferences-/Settings-Zugriffe so planen, dass keine DBFM-ENV-/DES-Regionen versehentlich überschrieben werden.
- [ ] Nur das reine App-Image als möglichen späteren Slot-Test betrachten; kein Merged-/Full-Flash-Image und keine Partitionstabelle flashen.
- [ ] Erst nach separater Prüfung und ausdrücklicher Freigabe einen Installationsplan erstellen.

## Sicherheitsgrenzen

- DBFM bleibt bei v4.2.2; kein Bootloader-Upgrade.
- Keine Änderungen an der laufenden Gerätekonfiguration, DES-Tabellen, Recovery oder dem bekannten funktionierenden Slot.
- Das 8-MiB-Merged-Image aus Build #56 ist nur ein Review-Artefakt und darf nicht als DBFM-App-Image geflasht werden.
- Das derzeitige ATS-Mini-OS ist ein Scaffold; es ist noch keine voll funktionsfähige Radio-Firmware.
- Die Analyse und Dokumentationsänderung haben keinen Build und keinen Flash-/Geräteschreibvorgang ausgelöst.

## Quellen

- [DBFM v4.2.2 Release-Dokumentation](https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot)
- [ATS-Mini-Referenzprojekt](https://github.com/13HN2709/ats-mini-dev)
- [Build #56](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41476309903)
- [Arduino-ESP32: Partition Table](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html)
