# WLAN-Datenquellen: Wetter, RDS-Senderlogos und spätere Online-Funktionen

Stand: 2026-11-01

## Zweck und Status

Diese Datei dokumentiert eine erste Web-/GitHub-Recherche für das eigenständige ATS Mini OS. Sie ist eine technische Vorauswahl, keine Implementierung und keine verbindliche Anbieterfreigabe. Vor der Implementierung sind Lizenz/Terms, Datenschutz, HTTPS, Antwortgrößen, Aktualität und Verhalten bei Ausfällen nochmals zu prüfen.

## 1. Wetterdaten

### Kandidat A: Open-Meteo – bevorzugter Startkandidat

Links:
- Wetter-API: https://open-meteo.com/en/docs
- Geocoding-API: https://open-meteo.com/en/docs/geocoding-api
- Lizenz/Terms: https://open-meteo.com/
- GitHub-Serverprojekt: https://github.com/open-meteo/open-meteo

Begründung:
- Die Geocoding-API akzeptiert Ortsnamen oder Postleitzahlen und unterstützt Länderfilter; für Deutschland sollte eine PLZ-Suche mit countryCode=DE durchgeführt und das Ergebnis zur Bestätigung angezeigt werden, weil Postleitzahlen/Ortsnamen nicht immer eineindeutig sind.
- Forecast API liefert aktuelle Bedingungen und Prognosen, unter anderem Temperatur, relative Feuchte, Wettercode, Bewölkung, Niederschlagswahrscheinlichkeit und Wind.
- Für nicht kommerzielle Nutzung ist die API grundsätzlich ohne API-Schlüssel und ohne Anmeldung nutzbar. Daten stehen unter CC BY 4.0; passende Quellenangabe im UI/Dokumentation ist erforderlich. Nutzungslimits und Terms vor einer Veröffentlichung nochmals prüfen.
- Open-Meteo integriert mehrere Wettermodelle, darunter DWD-Modelle für Mitteleuropa. Prognosen sind modellbasiert und nicht als minutengenaue Beobachtung zu behandeln.

Vorgeschlagener Datenfluss:
1. Nutzer gibt PLZ ein; System sucht Ort in Deutschland und zeigt Treffer zur Bestätigung.
2. Koordinaten und Zeitzone werden lokal gespeichert.
3. Bei aktiver Internetverbindung Wetterdaten über HTTPS abrufen; nur erforderliche Felder anfragen.
4. Daten auf Plausibilität prüfen, Wettercode in lokale Icons übersetzen und Cache mit Abruf-/Gültigkeitszeitpunkt aktualisieren.
5. UI zeigt Quelle und Zeitpunkt der letzten erfolgreichen Aktualisierung. Bei Fehlern bleiben letzter Cache und lokales Radio verfügbar.
6. Aktualisierung in moderatem Intervall (Startvorschlag 15–30 Minuten, später konfigurierbar); keine wiederholten Anfragen in kurzen Schleifen.

### Alternative: Deutscher Wetterdienst (DWD)

DWD ist eine relevante Quelle für Deutschland, insbesondere bei offiziellen Wetterwarnungen und Radarprodukten. Zu prüfen sind konkrete offene Daten-Endpunkte, Nutzungsbedingungen, Dateiformate und der zusätzliche Implementierungsaufwand. Für einen kleinen ersten Wetterbildschirm ist Open-Meteo voraussichtlich einfacher; DWD-Warnungen/Radar können später als eigenes Modul ergänzt werden.

## 2. RDS-Sendernamen und Logos

### Kandidat A: RadioDNS Project Logo / Service Information

Links:
- RadioDNS technische Dokumentation: https://radiodns.org/technical/documentation/
- Project Logo – technischer Ablauf: https://radiodns.org/campaigns/project-logo/walkthrough-finding-radio-station-logos/
- Project Logo technische Details: https://radiodns.org/campaigns/project-logo/technical-details/

RadioDNS beschreibt einen standardisierten Weg, aus terrestrischen FM-Informationen (Frequenz, PI-Code und Ländercode/ECC) per DNS einen Sender-Metadatenserver zu finden und dessen Service-Information (SI) auszulesen. Diese Metadaten können Sendernamen und Logo-Referenzen enthalten. Das ist der fachlich passendste Standardkandidat, sofern Sender den Dienst korrekt eingerichtet haben.

Offene Punkte:
- Nicht jeder Sender veröffentlicht RadioDNS-Metadaten.
- Für Deutschland müssen PI-/ECC-Umsetzung, Frequenzkonvertierung und DNS-/HTTP(S)-Pfad korrekt umgesetzt und getestet werden.
- SI-Dateien können größer/komplexer als für den ATS nötig sein; Parser sollte nur erforderliche Felder verarbeiten.
- Bild-URL, Format, Auflösung, Cache- und Lizenzbedingungen der jeweiligen Senderressource prüfen.
- Fallback erforderlich, wenn kein Logo existiert oder die Verbindung scheitert.

### Kandidat B: Radio-Browser

Link: https://docs.radio-browser.info/

Die API liefert unter anderem Stationsdatensätze mit Namen, Homepage und Favicon-Feldern. Das kann als ergänzende Suche oder Fallback dienen, ist aber nicht garantiert mit dem empfangenen FM-RDS-Sender identisch. Der PI-Code ist nicht automatisch ein universell eindeutiger Schlüssel in dieser Datenbank. Zuordnungen anhand von Sendername, Land, Frequenz und Homepage können mehrdeutig sein; Treffer müssen lokal validiert werden.

### Kandidat C: kommerzielle Radiodatenbanken

Radio Data Center (https://www.radiodatacenter.com/radio-datenbank/) bewirbt eine umfangreiche weltweite Radiodatenbank einschließlich RDS-PI, PS-Namen und Logos. Das wirkt für hohe Abdeckung interessant, erfordert aber eine Anfrage zur Lizenz/Verfügbarkeit/Kosten und ist daher kein automatischer Standard für ein Hobbyprojekt.

### Nicht als alleinige Quelle empfohlen

- Ungeprüfte Google-Bildersuche oder Scraping von Logo-Webseiten: rechtlich, technisch und hinsichtlich Stabilität problematisch.
- Beliebige Logo-URLs direkt bei jedem Senderwechsel abrufen: langsam, fehleranfällig und kann die Radiofunktion beeinträchtigen.
- Logo nur anhand des kurzen RDS-PS-Textes bestimmen: mehrdeutig, dynamischer PS möglich, ähnlich benannte Regionalprogramme.

### Empfohlene Logo-Strategie

1. Zuerst vorhandene RDS-Daten (PS/PI/Frequenz/Band) auslesen und als Stationsidentität normalisieren.
2. RadioDNS standardkonform als erste Logo-/Metadatenquelle prüfen.
3. Radio-Browser als optionalen Fallback testen, mit strenger Trefferbewertung.
4. Logos in kleinen, vorverarbeiteten Bildgrößen speichern; lokal gecachtes Logo beim erneuten Empfang sofort anzeigen.
5. Bei fehlendem Treffer weiterhin Sendername/RDS-Text anzeigen; Logo ist ein optionales Extra, kein Fehlerzustand.
6. Vorübergehend keine großen Originalbilder oder Animationen herunterladen.

## 3. Bildformate und Display

- Das vorhandene ATS-Referenzprojekt verwendet LovyanGFX; dessen Repository nennt unter anderem TJpgDec und Pngle als verwendete/unterstützte Bildbibliotheken. Das ist ein Hinweis auf mögliche Wege, aber noch kein Beleg, dass diese Decoder in der ATS-Mini-Referenzfirmware bereits eingebunden oder für deren Displaykonfiguration eingerichtet sind.
- Espressifs LVGL-Decoder-Beispiel für ESP32-S3 demonstriert JPEG, PNG, QOI sowie speichersparende zeilenweise Split-Formate. Das ist eine technische Referenz, aber die Übernahme würde eine Prüfung des tatsächlichen Frameworks und der Integration erfordern.
- Für den ersten Schritt bevorzugt: lokal gezeichnete Wetter-Icons und kleine vorverarbeitete Senderlogos. JPG/PNG erst nach einem isolierten Prototyp mit messbarem RAM-, Flash- und Zeitbudget auswählen.
- Jede externe Datei braucht maximale Downloadgröße, Timeout, Typprüfung, Dimensionenbegrenzung, Cache-Limit und sauberen Abbruch.

Referenz:
- LovyanGFX: https://github.com/lovyan03/LovyanGFX
- Espressif LVGL Image Decoder Beispiel: https://github.com/espressif/esp-iot-solution/tree/master/examples/display/gui/lvgl_decode_image

## 4. Netzwerkarchitektur-Anforderungen

- Bestehende WLAN-Verbindungsverwaltung wiederverwenden, nicht pro Modul einen eigenen Wi-Fi-Manager aufbauen.
- AP Only ist für lokale Websteuerung geeignet, stellt allein aber keine Internetverbindung für Wetter/Logos bereit. Für externe Daten muss STA-Internet verfügbar sein (z. B. im AP+Connect-Modus oder STA).
- Zentrale Netzwerkdienste: Verbindungsstatus, DNS/HTTPS, Zeitstatus, begrenzte Requests, Fehlerstatus, optional gemeinsamer Download-/Cache-Dienst.
- Netzwerkoperationen dürfen nicht im zeitkritischen Radio-/Display-/Encoderpfad blockierend laufen. Klare Zeitlimits, begrenzte Antwortgrößen und Abbruch-/Retry-Backoff vorsehen.
- Wetter und Logos müssen bei WLAN-Ausfall graceful degraden: Cache/Platzhalter nutzen, Radioempfang weiterbetreiben.
- Keine Passwörter oder personenbezogenen Zugangsdaten in Logs/URLs speichern.
- HTTPS-Zertifikatsprüfung nicht deaktivieren, um Verbindungsfehler zu umgehen.
- Zunächst keine allgemeinen Webbrowser-Funktionen; nur explizit erlaubte und begrenzte Datentypen/Hosts.

## 5. Empfohlene Reihenfolge

1. Netzwerk-/HTTPS-Fähigkeiten und Zeitdienst in der bestehenden Referenz abschließend inventarisieren.
2. Wetter zuerst: PLZ-Geocoding, aktuelle Bedingungen, Wettercodes, kleiner Cache, Quelle/Zeitstempel.
3. Logo-Recherche: RadioDNS-Machbarkeit mit einem bekannten deutschen Sender und PI/Frequenz verifizieren; Radio-Browser als ergänzende Quelle bewerten.
4. Bilddecoder und Speicherverbrauch isoliert prüfen, bevor Downloads auf dem ATS stattfinden.
5. Erst nach Nutzerfreigabe Implementierung und Test planen.

## 6. Sicherheits- und Arbeitsgrenzen

- `13HN2709/ats-mini-dev` bleibt read-only.
- Keine Änderungen an bestehenden Radio-Firmware-Repositories.
- Keine Builds, GitHub Actions oder Flash-Vorgänge ohne ausdrückliche Freigabe.
- Slot A und DiamondRadio in Slot B bleiben unangetastet.
- Die 8-MB-Konfiguration in der Referenz ist kein Beweis für die physische Flash-Größe des Geräts.
