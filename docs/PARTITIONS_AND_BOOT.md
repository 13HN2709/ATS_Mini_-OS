# Partitionen, Bootpfad und Kompatibilität

**Status:** DBFM v4.2.2 (DES) ist anhand des zugehörigen Release-README als dokumentierte Referenz identifiziert. Das tatsächliche Layout des Geräts und die Kompatibilität der aktuellen ATS-Mini-OS-App sind noch nicht am Gerät verifiziert. Keine Flash-Freigabe.

## Aktueller Referenzstand

- Der Nutzer bestätigt, dass sein ATS Mini V4 mit **GNBD ATS Mini Dualboot / DBFM v4.2.2 (DES)** läuft und vorerst auf dieser Version bleiben soll.
- Release-Dokumentation: https://github.com/GNBD/ats-mini-dualboot/tree/v4.2.2-dualboot
- DBFM v4.2.2 dokumentiert N16R8 (16 MB Flash + 8 MB OPI-PSRAM), `FlashSize=16M`, APP0 bei `0x010000`, APP1 bei `0x390000`, je 3,5 MiB, Recovery bei `0x860000` sowie ein DES-Layout mit zwei Partitionstabellen.
- Die Werte aus dem Release-README sind Sollwerte, kein direkter Auslesebeweis der tatsächlichen Partitionstabelle oder der aktiven DES-Umgebung am Gerät.

## Kritischer Konfigurationsunterschied

- `firmware/ats-mini-os/partitions.csv` entspricht weiterhin der Referenz aus `ats-mini-dev`, die bis `0x800000` (8 MiB) reicht und APP0/APP1 an anderen Offsets anlegt.
- `.github/workflows/ats-mini-os-first-compile.yml` baut derzeit explizit mit `FlashSize=8M` und prüft die erzeugten `flash_args` auf `8MB`.
- Diese 8-MiB-Referenz ist **nicht kompatibel mit dem dokumentierten DBFM-v4.2.2-DES-Partitionslayout** und darf nicht als Partitionstabelle oder Installationsanleitung für das Gerät verwendet werden.
- Ein reines App-Image enthält zwar nicht die Partitionstabelle als installierte Gerätekonfiguration; trotzdem müssen sein Header/Buildprofil, die verwendeten Partition-Labels und die Größe gegen das tatsächliche Ziel geprüft werden. Aus dem bloßen Vorhandensein einer `.ino.bin` folgt keine Flash-Freigabe.
- Das aktuelle OS-README beschreibt einen Scaffold, in dem Display-/Encoder-Ansteuerung und vollständige Radio-Firmware noch fehlen. Ein Compile-Erfolg ist noch kein erfolgreicher Radio-Betrieb.

## Vor einem späteren App-Image- oder Flash-Schritt zu prüfen

- Tatsächliches Flash-/PSRAM-Profil des Geräts.
- Aktive DBFM-Version, DES-Tabellenauswahl und tatsächliche Partitionstabelle.
- Exakter Slot und sein Zustand (frei, aktiv, Standby); den aktiven Slot nicht überschreiben.
- Größe, Offset und erwartetes Format des App-Images.
- Ob die Firmware benötigte Labels/Partitionen (`nvs`, `littlefs`, `settings` etc.) im DBFM-v4.2.2-Layout korrekt findet und verwendet.
- OTA-/Update-Verfahren und mögliche automatische Slot-Auswahl.
- Backup vollständig, lesbar und Wiederherstellungsweg bekannt.
- Verhalten bei fehlgeschlagenem Start und Rückkehr zum bisherigen Slot.
- Signatur-, Header-, Verschlüsselungs- oder Prüfsummenanforderungen, falls vorhanden.

## Schutzanforderungen

1. DBFM v4.2.2 nicht aktualisieren.
2. Bootloader, Partitionstabellen, `otadata`, Recovery und Settings-Regionen nicht verändern.
3. Kein Merged-/Full-Flash-Image und keine 8-MiB-Standard-Partitionstabelle auf das Gerät schreiben.
4. Keine automatische Slot-Umschaltung und kein Überschreiben des aktiven Slots.
5. Eine frühere Erwähnung von `ATSMiniFULLBackup.nbin` (16 MB) ist kein Nachweis, dass eine aktuelle, vollständige und lesbare Sicherung verfügbar ist. Vor einem riskanten Schritt muss das überprüft werden.
6. Ein erfolgreicher Compile-Test ist keine Flash-Freigabe.

## Nachweisprotokoll

| Prüfpunkt | Nachweis / Quelle | Status |
|---|---|---|
| DBFM-Version | Nutzerangabe: v4.2.2; Release-README geprüft | Dokumentierter Referenzstand |
| DBFM-Soll-Slots | Release-README: APP0 `0x010000`, APP1 `0x390000`, je 3,5 MiB | Dokumentiert, Gerät noch nicht ausgelesen |
| DBFM-Soll-Recovery | Release-README: `0x860000` | Dokumentiert |
| ATS-Mini-OS-Partitionstabelle | CSV bis 8 MiB, andere Offsets | Für DBFM v4.2.2 nicht verwenden |
| ATS-Mini-OS Buildprofil | Workflow verlangt `FlashSize=8M` | Blocker für flashbereites Image |
| Vollständige Firmware-Funktionalität | README kennzeichnet Scaffold; Display/Encoder fehlen | Nicht flashbereit für normalen Radioeinsatz |
| Imageformat/-größe/-Hash | Kein neues geprüftes Artefakt in dieser Prüfung | Offen |
| Aktiver Slot / DES-Tabellenauswahl | Kein Gerätereport/Dump vorliegend | Offen |
| Backup aktuell und lesbar | Noch nicht verifiziert | Offen |

## Nächster sicherer Ablauf

1. Buildkonfiguration und App-Abhängigkeiten für das DBFM-v4.2.2-N16R8-Ziel vorbereiten; keine Partitionstabelle des DBFM überschreiben.
2. Den aktuellen Scaffold und den geplanten Testumfang prüfen; klar festlegen, ob ein Diagnose-App-Test oder eine vollständige Radio-Firmware das Ziel ist.
3. Erst nach ausdrücklicher Zustimmung des Nutzers den manuellen Compile-Workflow starten.
4. Artefakt lesend prüfen: Dateiname, Commit, Profil, Header, Größe und SHA-256. Noch keine Flash-Freigabe allein aus einem erfolgreichen Build ableiten.
5. Vor einem Flash eine genaue Vorlage mit Zielslot, Schreibbereichen, Backup-/Rückfallweg und verbleibenden Risiken vorlegen.

## Explizit ausgeschlossen

- Kein Upgrade von DBFM v4.2.2.
- Kein Bootloader-Flash.
- Keine Partitionstabelle schreiben oder reparieren.
- Keine Änderung an `otadata`.
- Kein Merged-/Full-Flash-Image als App-Slot-Image verwenden.
- Keine Slot-Umschaltung auf Verdacht.
- Kein automatischer Build-/GitHub-Actions-Start ohne ausdrückliche Zustimmung.
- Kein Flash und keine Änderung am laufenden Gerät.
