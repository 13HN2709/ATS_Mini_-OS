# Partitionen, Bootpfad und Kompatibilität

**Status:** Die ATS-Mini-DEV-Referenzkonfiguration ist im Quellcode abgeglichen. Die tatsächliche Kompatibilität mit Bootmanagern und Single-Flash-Verfahren ist weiterhin nicht verifiziert. Keine Flash-Freigabe.

## Bereits abgeglichen

- `ats-mini-dev/ats-mini/partitions.csv` und `firmware/ats-mini-os/partitions.csv` enthalten dieselben Partitionseinträge und Offsets.
- Die Build-Profile in `.github/workflows/build.yml` von `ats-mini-dev` und `.github/workflows/ats-mini-os-first-compile.yml` von ATS Mini OS verwenden dieselben ESP32-S3-Optionen für QSPI und OSPI: `FlashSize=8M`, `PSRAM=enabled/opi`, `CPUFreq=80`, `FlashMode=qio`, `PartitionScheme=custom` sowie dieselben USB-/CDC-Optionen.
- Die Tabelle reicht bis Offset `0x800000` (8 MiB). Das beschreibt die aktuell verwendete 8-MiB-Partitionierung, nicht die gesamte physische Flash-Kapazität des 16-MB-Geräts.
- Dieser Abgleich bestätigt gleiche Quellkonfigurationen; er beweist **nicht**, dass ein daraus erzeugtes Image bereits mit jedem Dual-Boot-Manager oder jedem Single-Flash-Verfahren kompatibel ist.

## Vor jedem späteren Image- oder Flash-Schritt zu klären

- Exaktes Flash-Layout des Geräts und Partitions-Tabelle, die der jeweilige Bootmanager erwartet.
- Bootloader- und Bootmanager-Version sowie deren Metadaten.
- Bedeutung und Lage von Slot A und Slot B.
- Größe, Offset und erwartetes Format jedes App-Images.
- OTA-/Update-Verfahren und Auswahl des aktiven Slots.
- Recovery-Verfahren und Wiederherstellung aus einer vollständigen Sicherung.
- Signatur-, Header-, Verschlüsselungs- oder Prüfsummenanforderungen.
- Verhalten bei fehlgeschlagenem Start oder ungültigem Image.

## Schutzanforderungen

1. Die bestehende Firmware in Slot A darf nicht überschrieben werden.
2. DiamondRadio in Slot B darf nicht verändert werden.
3. Keine Aktionen ausführen, die automatisch flashen oder den aktiven Slot wechseln.
4. Eine frühere Erwähnung von `ATSMiniFULLBackup.nbin` (16 MB) ist kein Nachweis, dass eine aktuelle, gültige Sicherung verfügbar ist. Vor späteren riskanten Arbeiten muss der Sicherungsstatus bestätigt werden.
5. Keine Imagegröße, Partitionsoffsets oder Boot-Metadaten raten.
6. Build-Artefakte aus dem Compile-Test nicht flashen, bevor Imageformat und Flash-/Bootpfad ausdrücklich geprüft und freigegeben wurden.

## Nachweisprotokoll

| Prüfpunkt | Nachweis / Quelle | Status |
|---|---|---|
| DEV- und OS-Partitionsdatei | Einträge und Offsets verglichen | Identisch |
| QSPI-/OSPI-Buildoptionen | Beide Workflows verglichen | Identisch |
| Physische Flash-Kapazität des konkreten Geräts | Gerät ist als 16 MB angegeben; tatsächlicher Chip noch nicht ausgelesen | Teilweise |
| Vom Bootmanager erwartetes Layout | Bootmanager-/Gerätemetadaten noch nicht untersucht | Offen |
| Slot A Offset/Größe | Bootmanager-Konfiguration noch nicht verifiziert | Offen |
| Slot B Offset/Größe | Bootmanager-Konfiguration noch nicht verifiziert | Offen |
| Image-Format | Artefakt-/Flashing-Anforderungen noch nicht geprüft | Offen |
| Recovery-Pfad | Backup und Wiederherstellungsverfahren vor einem Flash-Schritt bestätigen | Offen |
| Sicherung aktuell und lesbar | Vor späterem Risiko separat bestätigen | Offen |

## Freigabekriterium

Ein späterer Image-Export oder Flash-Schritt benötigt eine separate ausdrückliche Nutzerfreigabe. Ein erfolgreicher Compile-Test und identische DEV-Referenzwerte sind selbst keine Flash-Freigabe.
