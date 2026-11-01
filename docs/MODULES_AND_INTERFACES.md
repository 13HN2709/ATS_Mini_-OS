# Modulkarte und Schnittstellenplanung

**Status:** Entwurf. Noch keine APIs oder Implementierungen verbindlich festgelegt.

## Geplante Module

| Modul | Verantwortung | Abhängigkeiten / offene Fragen |
|---|---|---|
| App | Systemstart, Lebenszyklus und Zustände | Boot-/Framework-Basis offen |
| HAL | stabile Hardwareverträge | Hardware-Inventar offen |
| Display Driver | Display initialisieren und zeichnen | Controller, Bus und Pins offen |
| Tuner Driver | Tuner steuern und Status lesen | Chip und Bibliothek offen |
| Radio Core | Frequenz, Band, Modulation und Tuning | konkrete Tuner-Fähigkeiten offen |
| UI | Layouts, Menüs, Encoder und Status | Display- und Eingabeschnittstellen offen |
| Spectrum Scanner | Scanablauf, Peaks und Ergebnisse | unterstützte Frequenzen/Modi offen |
| Dual RX | parallele Empfangsfunktion nur bei belegter Unterstützung | Hardwarebeleg zwingend |
| Web Control | Weboberfläche und Verbindungsmodi | Netzwerk- und Sicherheitskonzept offen |
| Config Service | Einstellungen speichern und migrieren | Speichermedium offen |
| Logging Service | Diagnosen und Fehlerberichte | Speicher-/Ausgabekanal offen |
| Performance Manager | CPU-Leistungsprofile zentral steuern | unterstützte Takte und Framework offen |

## Schnittstellenprinzipien

- Module erhalten Abhängigkeiten über explizite Schnittstellen statt direkter globaler Hardwarezugriffe.
- Fähigkeiten werden zur Laufzeit oder über eine geprüfte Hardwarekonfiguration bereitgestellt.
- Hardware- und I/O-Fehler werden als definierte Fehlerzustände zurückgegeben.
- UI und Web Control sollen Radio-Kernfunktionen über dieselben fachlichen Schnittstellen ansprechen.
- Module müssen initialisierbar und – wo sinnvoll – kontrolliert deaktivierbar sein.
- Keine API wird als final markiert, bevor Hardware und Referenzquellcode geprüft sind.

## Spectrum Scanner

Geplante Trennung zwischen Scan-Engine und Anzeige. Frequenzbereiche, Schrittweiten, Modulationswechsel, Abbruch und Peak-Daten werden später als Anforderungen spezifiziert.

## Dual RX

Dual RX wird nur angeboten, wenn Tuner, Hardwarepfad und Treiber die erforderliche parallele Empfangsfunktion tatsächlich unterstützen. Zwei UI-Empfänger oder zwei Softwareobjekte allein beweisen keinen gleichzeitigen Empfang.

## Web Control

Web Control wird als separates Modul geplant. WLAN-Modi, Verbindungsstatus und Anzeigezustand sollen nicht mit dem Radio-Kern vermischt werden.

## Performance Manager

Layouts melden einen Bedarf bzw. ein Leistungsprofil; sie setzen nicht eigenständig an beliebigen Stellen den CPU-Takt. Waterfall ist ein vorgesehener Anwendungsfall für maximale Leistung. Das Wiederherstellen des vorherigen Profils beim Verlassen der Ansicht ist ein Anforderungspunkt.

## Speicherzugriff

Nur die vorgesehenen OSPI-/QSPI-Pfade berücksichtigen. Treiber und Pins erst nach der Hardwareprüfung festlegen.
