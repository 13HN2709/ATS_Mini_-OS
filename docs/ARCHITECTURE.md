# Architekturentwurf

**Status:** Vorschlag zur Diskussion – keine Implementierung und keine Hardwareverifikation.

## Ziele

- Modulare, wartbare Firmware mit klaren Verantwortlichkeiten.
- Radio-Kern unabhängig von Layouts und Weboberfläche halten.
- Hardwareabhängigkeiten über HAL und Treiber kapseln.
- Spätere kompatible Radio-Varianten ergänzen können, ohne den Kern neu zu schreiben.
- Bestehende Slot-A- und Slot-B-Firmware nicht verändern.
- WLAN als optionale Plattform für Wetter, Bildinhalte und weitere Netzwerkdienste nutzen, ohne den Radioempfang zu blockieren.

## Vorgeschlagene Schichten

1. **App / System Lifecycle** – Initialisierung, Zustände und kontrolliertes Herunterfahren bzw. Wechseln von Modi.
2. **Services** – Konfiguration, Logging, Zeitdienste, Netzwerkzugriff, Bildcache und Performance Manager.
3. **Radio Core** – Frequenz, Band, Modulation, Abstimmung und gemeldete Fähigkeiten.
4. **HAL** – stabile Schnittstellen zu Display, Eingabe, Tuner und Speicher.
5. **Drivers** – konkrete Implementierungen für nachgewiesene Hardware.
6. **Modules** – optionale Funktionen wie Spectrum Scanner, Dual RX, Web Control und Wetter.
7. **UI** – Layouts, Statusanzeigen, Encoder- und Menübedienung.

Die genaue Schichtenzuordnung wird nach der Inventarisierung überprüft.

## Zentrale Architekturregeln

- UI-Code darf keine Hardwarefähigkeiten erfinden oder voraussetzen.
- Module fragen Fähigkeiten über definierte Schnittstellen ab.
- Treiberbibliotheken und Versionen erst nach Prüfung des Referenzquellcodes festlegen.
- Hardwarezugriffe sollen nicht unkontrolliert über das Projekt verteilt sein.
- Fehlerzustände und nicht verfügbare Funktionen müssen klar gemeldet werden.
- Netzwerkzugriffe müssen zeitlich begrenzt, abbrechbar und vom Radio-Hauptloop entkoppelt sein.
- WLAN-Ausfall darf den lokalen Radioempfang und die Grundbedienung nicht blockieren.
- Netzwerkdaten, Bilddownloads und Cachegrößen müssen begrenzt sein; keine ungeprüften Inhalte direkt als ausführbare Daten behandeln.

## WLAN- und Inhaltsdienste

Geplant ist ein zentraler Netzwerkdienst, den optionale Module gemeinsam nutzen können. Er soll Verbindungsstatus, DNS/HTTPS, Zeitlimits, Fehler und Offline-Verhalten vereinheitlichen. Zugangsdaten und Ortsangaben werden in den Systemeinstellungen verwaltet.

### Wettermodul

- Nutzer kann eine deutsche Postleitzahl eingeben; ein Geocoding-Dienst löst diese in einen geeigneten Ort bzw. Koordinaten auf.
- Wetterdaten werden über einen konfigurierbaren Wetterdienst abgerufen. API-Anbieter, Nutzungsbedingungen, Datenschutz und eventuelle Schlüssel sind vor der Implementierung zu prüfen.
- Anzeigeoptionen: aktuelles Wetter, Temperatur, gefühlte Temperatur soweit verfügbar, Niederschlagswahrscheinlichkeit, Wind, Tageshöchst-/Tiefstwerte und kurze Vorhersage.
- Sonne, Wolken, Regen, Schnee, Gewitter und unbekannte Zustände werden über einheitliche lokale Icons dargestellt. Dadurch muss nicht für jede Wetteraktualisierung ein Bild geladen werden.
- Wetteraktualisierung erfolgt periodisch und bei Bedarf manuell, mit Cache und Offline-Anzeige des letzten erfolgreichen Abrufs samt Zeitstempel.
- Abrufe erfolgen asynchron bzw. in kleinen Arbeitsschritten und dürfen Audio, Abstimmung, Encoder, Scanner oder Dual RX nicht blockieren.

### Bildanzeige und Bildcache

- Ziel ist die Anzeige geeigneter Bilddateien aus lokalen Ressourcen und später optional aus dem Internet, z. B. Wetterkarten, Logos oder Nutzerbilder.
- Ein eigener Bilddienst soll Dateiformat, Abmessungen, Dateigröße, Download-Zeitlimit, Dekodierfehler und Speicherbedarf kontrollieren.
- Für die erste Version lokale, mitgelieferte Icons bevorzugen; anschließend kleine, optimierte Rasterbilder über HTTPS laden und im begrenzten Cache ablegen.
- JPEG/BMP/PNG-Unterstützung ist erst nach Prüfung der vorhandenen Displaybibliothek, Decoder-Lizenzen und RAM-/PSRAM-Bilanz festzulegen. Animierte Bilder und große Satellitenkarten sind nicht vorausgesetzt.
- Bilder müssen auf die Displayfläche skaliert oder beschnitten werden; die Darstellung darf den verfügbaren Framebuffer nicht unkontrolliert vergrößern.

## Performance Manager

Geplante zentrale Stelle für CPU-Leistungsprofile.

- Nutzerauswahl eines CPU-Profils in den Systemeinstellungen vorsehen.
- Eine Ansicht wie Waterfall darf ein höheres Leistungsprofil anfordern.
- Die Verwaltung entscheidet zentral über den tatsächlich zulässigen Modus.
- Beim Verlassen der Ansicht soll der vorherige Modus wiederhergestellt werden.
- Netzwerk- und Bilddekodieraufgaben können bei Bedarf ein Profil anfordern, dürfen aber nicht dauerhaft eigenständig den höchsten Takt erzwingen.
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
- Wetteranbieter, Datenformat, Aktualisierungsintervall und Offline-Strategie.
- Bildformate, Decoder, Cachegröße und Speicherstrategie.
- Bootloader, Partitionen, Slot- und Image-Kompatibilität.

## Nächster Schritt

Hardware- und Quellcode-Inventar vervollständigen. Erst danach Architekturentscheidungen als bestätigt markieren.

## Erstes Entwicklungsziel: UKW-Radio mit RDS und RadioDNS

Der Nutzer hat die Reihenfolge festgelegt: Das Projekt wird zuerst als möglichst kleines, stabiles UKW-Radio aufgebaut. Zusätzliche Funktionen kommen erst schrittweise hinzu.

### Zielumfang der ersten funktionsfähigen Stufe
1. **UKW-FM-Grundbetrieb:** Frequenz abstimmen und Radio-Audio über die nachgewiesene ATS-Mini-V4-Hardware ausgeben.
2. **RDS:** RDS-Daten empfangen und auswerten, einschließlich Sendername/PS und PI-Code, soweit Hardware und Treiber diese Daten zuverlässig liefern.
3. **RT (RadioText):** Radiotext darstellen, wenn vom Sender ausgestrahlt und vom Tuner verfügbar.
4. **PTY (Program Type):** Programmtyp darstellen, wenn vom Sender ausgestrahlt und vom Tuner verfügbar.
5. **RadioDNS:** nach erfolgreicher Netzwerkverbindung anhand der standardkonform ermittelten Senderidentität verfügbare Service-Informationen und gegebenenfalls Senderlogo finden.
6. **Robustes Fallback:** Fehlen RDS-/RadioDNS-Daten oder ist WLAN nicht verfügbar, bleibt das UKW-Radio bedienbar; Sendername bzw. RT/PTY werden je nach Verfügbarkeit weiter angezeigt, ein Logo ist optional.

### Entwicklungsreihenfolge und Abnahmepunkte
- **Stufe 0 – Inventarisierung:** Hardware, Pins, Tuner-Modell, vorhandene SI4735-Anbindung, Display, Eingaben, Audioausgabe, WLAN/HTTPS und Zeitdienst quellenbasiert bestätigen.
- **Stufe 1 – FM-Grundbetrieb:** nur UKW-Abstimmung und Audio; verifizieren, dass Bedienung und Empfang stabil sind.
- **Stufe 2 – RDS/PS/PI:** erst RDS-Grunddaten und stabile Senderidentität testen.
- **Stufe 3 – RT und PTY:** Daten unabhängig behandeln; fehlende Senderdaten dürfen die Anzeige nicht beschädigen.
- **Stufe 4 – RadioDNS:** zuerst mit einem bekannten deutschen UKW-Sender testen; PI/ECC/Frequenzbildung gemäß Spezifikation verifizieren, Metadaten abrufen und Logo nur nach Prüfung laden.
- **Stufe 5 – Stabilität:** WLAN-Ausfall, fehlende Metadaten, Timeouts und Cache-Verhalten testen, ohne Radioempfang oder Bedienung zu blockieren.
- Erst danach werden weitere Funktionen wie Wetter, Web Control, Scanner oder Dual RX priorisiert und einzeln geplant.

### Noch nicht festgelegt
- Exakte Menü-/Layoutgestaltung und Logo-Position.
- Konkreter Bilddecoder und Cachegröße.
- Ob die initiale Stufe bereits heruntergeladene Logos dauerhaft speichert oder zunächst nur temporär cached.
- Diese Liste definiert den Umfang und die Reihenfolge, ist aber keine Freigabe für Code, Builds, GitHub Actions oder Flashen.
