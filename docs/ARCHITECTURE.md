# Architekturentwurf

**Status:** Vorschlag zur Diskussion – keine Implementierung und keine Hardwareverifikation.

## Ziele

- Modulare, wartbare Firmware mit klaren Verantwortlichkeiten.
- Radio-Kern unabhängig von Layouts und Weboberfläche halten.
- Hardwareabhängigkeiten über HAL und Treiber kapseln.
- Spätere kompatible Radio-Varianten ergänzen können, ohne den Kern neu zu schreiben.
- Bestehende Slot-A- und Slot-B-Firmware nicht verändern.

## Vorgeschlagene Schichten

1. **App / System Lifecycle** – Initialisierung, Zustände und kontrolliertes Herunterfahren bzw. Wechseln von Modi.
2. **Services** – Konfiguration, Logging, Zeitdienste und Performance Manager.
3. **Radio Core** – Frequenz, Band, Modulation, Abstimmung und gemeldete Fähigkeiten.
4. **HAL** – stabile Schnittstellen zu Display, Eingabe, Tuner und Speicher.
5. **Drivers** – konkrete Implementierungen für nachgewiesene Hardware.
6. **Modules** – optionale Funktionen wie Spectrum Scanner, Dual RX und Web Control.
7. **UI** – Layouts, Statusanzeigen, Encoder- und Menübedienung.

Die genaue Schichtenzuordnung wird nach der Inventarisierung überprüft.

## Zentrale Architekturregeln

- UI-Code darf keine Hardwarefähigkeiten erfinden oder voraussetzen.
- Module fragen Fähigkeiten über definierte Schnittstellen ab.
- Treiberbibliotheken und Versionen erst nach Prüfung des Referenzquellcodes festlegen.
- Hardwarezugriffe sollen nicht unkontrolliert über das Projekt verteilt sein.
- Fehlerzustände und nicht verfügbare Funktionen müssen klar gemeldet werden.

## Performance Manager

Geplante zentrale Stelle für CPU-Leistungsprofile.

- Nutzerauswahl eines CPU-Profils in den Systemeinstellungen vorsehen.
- Eine Ansicht wie Waterfall darf ein höheres Leistungsprofil anfordern.
- Die Verwaltung entscheidet zentral über den tatsächlich zulässigen Modus.
- Beim Verlassen der Ansicht soll der vorherige Modus wiederhergestellt werden.
- Frequenzwerte, API und Verhalten sind offen, bis Framework und ESP32-S3-Konfiguration verifiziert wurden.
- Energieverbrauch, Wärme, Stabilität und Timing müssen später geprüft werden.

## Speicherbusse

Für den betrachteten Umfang sind ausschließlich **OSPI und QSPI** vorgesehen. Welche Controller, Geräte und Treiber tatsächlich benötigt werden, ist anhand der Zielhardware zu verifizieren.

## Offene Entscheidungen

- Arduino-ESP32 oder ESP-IDF als Buildbasis.
- Bestehende Bibliotheken übernehmen oder ersetzen.
- Konkrete HAL-Schnittstellen und Fehlerkonzept.
- Konfigurationsspeicherung und Migration.
- Modul-Lebenszyklus und Nebenläufigkeit.
- Verfügbare Tuner-Fähigkeiten und Grenzen von Dual RX.
- Display-Auflösung, Controller und Speicherbedarf.
- Bootloader, Partitionen, Slot- und Image-Kompatibilität.

## Nächster Schritt

Hardware- und Quellcode-Inventar vervollständigen. Erst danach Architekturentscheidungen als bestätigt markieren.
