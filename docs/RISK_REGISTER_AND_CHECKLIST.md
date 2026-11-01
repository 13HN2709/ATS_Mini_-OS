# Risikoregister und Checkliste

## Risikoregister

| Risiko | Auswirkung | Gegenmaßnahme | Status |
|---|---|---|---|
| Slot A überschrieben | Verlust funktionierender Firmware | Keine Flash-Aktionen; Layout zuerst nachweisen | Offen |
| DiamondRadio in Slot B beeinträchtigt | Verlust vorhandener Firmware | Slot B nicht verändern; Bootpfad verifizieren | Offen |
| Image passt nicht in App-Slot | Build-/Startfehler | Slotgröße und Imageformat nachweisen | Offen |
| Falsche Pinbelegung | Hardware kann ausfallen oder nicht starten | Pinout aus verlässlicher Quelle bestätigen | Offen |
| Tuner unterstützt Dual RX nicht | Funktion nicht realisierbar | Fähigkeiten und Signalweg prüfen | Offen |
| Bibliotheken inkompatibel | Buildfehler oder Laufzeitprobleme | Framework und Versionen aus Quellen ermitteln | Offen |
| CPU-Maximalmodus instabil/zu heiß | Abstürze, Timingprobleme oder erhöhter Verbrauch | Taktgrenzen, Temperatur und Last später testen | Offen |
| Fremdcode/Lizenz unklar | rechtliches oder Wartungsrisiko | Herkunft und Lizenz vor Übernahme prüfen | Offen |
| Übergabedokumentation veraltet | Fehlentscheidungen in Folgesitzungen | Handover nach relevanten Änderungen aktualisieren | Laufend |

## Inventarisierungs-Checkliste

- [ ] Repository- und Branch-Stand verifizieren
- [ ] Referenzprojektzugriff und Umfang klären
- [ ] Hardwarevariante und Pinout belegen
- [ ] Display-Controller und Displaybus belegen
- [ ] Tuner-Variante und unterstützte Modi belegen
- [ ] Buildsystem, Framework und Toolchain identifizieren
- [ ] Bibliotheken und Lizenzen erfassen
- [ ] Partitionen, Slots, Bootmetadaten und Recovery dokumentieren
- [ ] Modulgrenzen und APIs abstimmen
- [ ] Speicherbedarf und Leistungsprofile erst nach Quellenprüfung planen

## Arbeitsgrenzen

- Kein Build und keine GitHub Actions auslösen.
- Nicht flashen.
- Slot A und Slot B nicht verändern.
- Keine bestehenden Firmware-Repositories verändern.
- Keine offene Frage durch eine unbelegte Annahme ersetzen.
