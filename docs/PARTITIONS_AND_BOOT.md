# Partitionen, Bootpfad und Kompatibilität

**Status:** Kritischer Prüfpunkt – noch nicht verifiziert. Keine Build- oder Flash-Freigabe.

## Vor jedem späteren Image- oder Flash-Schritt zu klären

- Exaktes Flash-Layout und Partitions-Tabelle.
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

## Nachweisprotokoll

| Prüfpunkt | Nachweis / Quelle | Status |
|---|---|---|
| Flash-Kapazität | Noch zu ermitteln | Offen |
| Partitions-Tabelle | Noch zu ermitteln | Offen |
| Slot A Offset/Größe | Noch zu ermitteln | Offen |
| Slot B Offset/Größe | Noch zu ermitteln | Offen |
| Image-Format | Noch zu ermitteln | Offen |
| Bootmanager-Metadaten | Noch zu ermitteln | Offen |
| Recovery-Pfad | Noch zu ermitteln | Offen |
| Sicherung aktuell und lesbar | Vor späterem Risiko separat bestätigen | Offen |

## Freigabekriterium

Ein späterer Build, Image-Export oder Flash-Schritt benötigt eine separate ausdrückliche Nutzerfreigabe. Die Fertigstellung dieser Dokumentation ist selbst keine Freigabe.
