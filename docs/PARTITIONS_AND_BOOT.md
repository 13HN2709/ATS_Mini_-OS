# Partitionen, Bootpfad und Kompatibilität

**Status:** Referenzlayout aus `ats-mini-dev` und Build #56 wurden quellenbasiert geprüft. Der Nutzer bestätigt den erfolgreichen Betrieb von `ats-mini-dev` in beiden DBFM-Slots einschließlich LittleFS-Dateien. Das aktuelle ATS-Mini-OS-Scaffold besitzt diese Storage-Funktionen noch nicht vollständig. Keine Flash-Freigabe.

## Aktueller Referenzstand

- Gerät: AMNVOLT ATS Mini V4, N16R8, laut Nutzer 16 MB Flash / 8 MB OPI-PSRAM.
- Bootmanager: GNBD DBFM v4.2.2 (DES); Version beibehalten, nicht aktualisieren.
- Release-Referenz: https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot
- DBFM-Release-Dokumentation nennt APP0 bei `0x010000`, APP1 bei `0x390000`, je 3,5 MiB, Recovery bei `0x860000`, LittleFS bei `0xA00000` sowie ENV0/ENV1 bei `0xF9D000` / `0xFBD000`.
- Diese Werte sind dokumentierte Referenzwerte, kein direkter Auslesebeweis der tatsächlichen DES-Auswahl oder des Gerätezustands.

## Was der Praxistest mit ats-mini-dev belegt

Der Nutzer bestätigt, dass `ats-mini-dev` in beiden Slots lief und LittleFS-Dateien einschließlich EiBi-Schedules und `memories.json` nutzte. Die Quellcodeprüfung von `13HN2709/ats-mini-dev` (main) bestätigt:

- `ats-mini/partitions.csv`: App0 `0x10000 / 0x300000`, App1 `0x310000 / 0x300000`, LittleFS `0x610000 / 0x1d0000`, Settings `0x7e0000 / 0x10000`, Coredump `0x7f0000 / 0x10000`.
- `ats-mini/Storage.h`: `STORAGE_PARTITION "settings"`.
- `ats-mini/Storage.cpp`: Preferences-Speicherung über die Partition `settings`.
- `ats-mini/EIBI.cpp`: EiBi-Datei `/schedules.bin` und temporäre Datei `/schedules.tmp` auf LittleFS.

Die ATS-Mini-OS-Partitionstabelle ist aktuell identisch mit der Referenzdatei aus `ats-mini-dev`. Die Abweichung von der dokumentierten DBFM-Tabelle ist **allein kein Beweis für Inkompatibilität**: Die vom Nutzer bestätigte Praxis zeigt, dass die Referenzfirmware in diesem Setup funktionierte. Das bedeutet nicht automatisch, dass das neue OS bereits gleichwertige Storage-Zugriffe oder denselben Runtime-Pfad besitzt.

## ATS-Mini-OS-Scaffold und Build #56

- Das aktuelle Scaffold verwendet `FlashSize=8M` für OSPI/QSPI und enthält die Referenz-Partitionstabelle aus `ats-mini-dev`.
- Das OS hat `src/settings/SettingsStore.h/.cpp` für die letzte FM-Frequenz. Es verwendet `Preferences.begin("atsmini_os", false)` ohne explizite Partitionsbezeichnung, also Standard-NVS. Die Referenz `ats-mini-dev` nutzt dagegen `STORAGE_PARTITION "settings"`.
- Eine LittleFS-Implementierung für EiBi-Dateien, `memories.json` oder `/schedules.bin` ist im OS noch nicht übernommen.
- Build #56: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41476309903
- App-Binärdateien: OSPI 956.624 Bytes; QSPI 954.368 Bytes. Beide passen größenmäßig in einen 3,5-MiB-Slot und besitzen ESP-Image-Magic `E9`.
- Beide Deskriptoren zeigen `arduino-lib-builder`; der Name ist kein bestätigter Blocker. `ATSM` bei Offset 288 fehlt, aber ist nicht als allgemeine DBFM-v4.2.2-Installationsvoraussetzung bestätigt.
- Build-Erfolg und Dateigröße sind kein Nachweis vollständiger Radio-Funktion oder Slot-Kompatibilität.

## Schutzanforderungen

1. DBFM v4.2.2 nicht aktualisieren.
2. Bootloader, tatsächliche Partitionstabelle, `otadata`, Recovery und DES-Tabellen nicht verändern.
3. Kein Merged-/Full-Flash-Image und keine Standard-Partitionstabelle auf das Gerät schreiben.
4. Nicht den aktiven Slot überschreiben und keine Slot-Umschaltung auf Verdacht durchführen.
5. Backup und Wiederherstellungsweg vor einem späteren riskanten Schritt verifizieren.
6. Erst die Storage- und Runtime-Abhängigkeiten des neuen OS mit der funktionierenden Referenz vergleichen.
7. Ein erfolgreicher Compile-Test ist keine Flash-Freigabe.

## Nachweisprotokoll

| Prüfpunkt | Nachweis / Quelle | Status |
|---|---|---|
| DBFM-Version | Nutzerangabe v4.2.2; Release-README | Dokumentierte Referenz |
| DBFM-Soll-Slots | Release-README | Dokumentiert, Gerät nicht ausgelesen |
| ats-mini-dev in App0 und App1 | Nutzererfahrung | Praktisch bestätigt |
| LittleFS mit EiBi / memories.json | Nutzererfahrung; EIBI.cpp bestätigt `/schedules.bin` | Praktisch bestätigt |
| Referenz-Partitionstabelle im OS | CSV-Dateien verglichen | Identisch mit ats-mini-dev |
| OS-Storage-Schicht | aktueller Scaffold geprüft | Noch nicht implementiert/übernommen |
| Build #56 App-Größe | erzeugte Artefakte | Geprüft, beide unter 3,5 MiB |
| Tatsächlicher aktiver Slot / DES-Tabelle | kein Gerätereport/Dump | Offen |
| Vollständige Radiofunktion | Scaffold-Hinweis im Quellcode | Nicht nachgewiesen |
| Flash-Freigabe | kein Hardwaretest | Nicht freigegeben |

## Nächster sicherer Ablauf

1. `ats-mini-dev` Storage-Initialisierung, Preferences-Partition und LittleFS-Init vollständig prüfen.
2. Die tatsächlich verfügbare NVS-/LittleFS-Partitionierung des DBFM-Setups klären, bevor die OS-Preferences-Partition geändert wird.
3. Die benötigte Storage-Schnittstelle für ATS Mini OS definieren und modular implementieren; vorhandene Dateien dürfen nicht automatisch formatiert oder gelöscht werden.
4. Runtime-Partitionzugriffe und Fehlerverhalten statisch prüfen.
5. Nutzer startet anschließend den manuellen Compile-Workflow; keine automatische Action aus dieser Dokumentationsänderung.
6. Artefakt lesend prüfen und separat einen Installationsplan mit Zielslot, exakter Datei, Adressen und Wiederherstellungsweg vorlegen.

## Explizit ausgeschlossen

- Kein DBFM-Upgrade.
- Kein Bootloader-Flash.
- Keine Änderung an der Geräte-Partitionstabelle oder `otadata`.
- Kein Merged-/Full-Flash-Image als App-Slot-Image.
- Kein automatischer Build-/GitHub-Actions-Start.
- Kein Flash und keine Änderung am laufenden Gerät.
