# Treiber- und Bibliotheksplan vor dem ersten Build

**Status:** Prüfung und Planung. Keine neuen Bibliotheken als installiert oder getestet ausgeben, solange kein tatsächlicher Test erfolgt ist.

## Vorgehen

1. Vorhandene ATS-Mini-V4-Referenzquellen und Build-Konfigurationen lesen.
2. Display, Encoder/Tasten, SI4735-Tuner, WLAN, Dateisystem und Flash-Aufteilung identifizieren.
3. Bestehende, passende Treiber bevorzugen; doppelte Hardwarezugriffe vermeiden.
4. Für jede externe Bibliothek Version, Lizenz, Speicherbedarf, unterstützte Hardware und benötigte Funktionen dokumentieren.
5. Schriftunterstützung getrennt halten: Standardtext, erweiterte Unicode-Zeichen und arabische Textformung.
6. Erst wenn die Abhängigkeiten feststehen, eine isolierte Testkonfiguration anlegen.
7. Den ersten Build nur nach expliziter Rückmeldung/Abstimmung starten. Bis dahin keine Actions auslösen und nichts flashen.

## Zu prüfende Bereiche

- Display und Zeichenausgabe (LovyanGFX, passende Version zur Referenz)
- SI4735-Tuner und zentraler RDS-Datenfluss
- Encoder und Tasten
- WLAN und HTTPS
- Dateisystem und Bild-/Wettercache
- Font-Konvertierung und Unicode-Abdeckung
- Arabische Buchstabenformung und Schreibrichtung
- Partitionierung, Dual-Boot-/Recovery-Kompatibilität und Image-Format

## Sicherheitsregeln

- Slot A und DiamondRadio in Slot B bleiben unverändert.
- Referenz-Repositories werden ausschließlich lesend geprüft.
- Kein Build/Flash vor Abschluss der Abhängigkeits- und Boot-Prüfung.
- Keine neuen Treiber nur aufgrund eines Namens oder einer Webbeschreibung übernehmen; Quellcode und Lizenz prüfen.
