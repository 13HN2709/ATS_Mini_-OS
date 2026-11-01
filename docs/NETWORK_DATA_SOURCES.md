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


## 7. Entscheidung aus der weiteren Planung: RadioDNS bevorzugt prüfen

Der Nutzer hat zugestimmt, RadioDNS als bevorzugten Kandidaten für die automatische Senderlogo-Suche in die Planung aufzunehmen. Das ist eine Architektur-/Rechercheentscheidung, noch keine Implementierungsfreigabe.

Geplanter Ablauf:
1. RDS-Informationen des tatsächlich empfangenen Senders auslesen (insbesondere PI-Code, Frequenz/Band und Länderkennung/ECC; PS-Name nur ergänzend).
2. Die Senderidentität nach der RadioDNS-Spezifikation auflösen und verfügbare Service-Informationen abrufen.
3. Falls ein Logo referenziert wird, URL/Antwort, erlaubtes Format, Lizenzbedingungen und Bildgröße prüfen.
4. Logo in einer begrenzten, displaygerechten Größe herunterladen und lokal cachen; bei erneutem Empfang aus dem Cache anzeigen.
5. Bei fehlenden RadioDNS-Daten, ungültiger Zuordnung, fehlendem WLAN oder Fehlern auf Sendername/RDS-Text zurückfallen.

Wichtige Einschränkungen:
- Noch nicht bestätigt, dass die vom Nutzer verwendete TEF6686-App tatsächlich RadioDNS nutzt; das ist eine plausible Vermutung, kein verifizierter Fakt.
- RadioDNS ist ein technischer Standard und keine Garantie, dass jeder Sender Logos veröffentlicht oder dass jeder Logo-Endpunkt kostenlos bzw. unter denselben Bedingungen nutzbar ist.
- Zuerst mit einem bekannten deutschen Sender testen und die Zuordnung über PI/Frequenz verifizieren. Radio-Browser bleibt optionaler Fallback, falls später sinnvoll.
- Vor Implementierung Spezifikation, Lizenz-/Nutzungsbedingungen, HTTPS und Decoder-/Speicherbedarf erneut prüfen.
- Weiterhin keine Builds, GitHub Actions, Flashvorgänge oder Änderungen an bestehenden Firmware-Repositories ohne ausdrückliche Freigabe.



## 8. RDS-Daten, die wir für RadioDNS erfassen müssen

Der Nutzer hat ausdrücklich festgelegt: Bevor RadioDNS gesucht wird, sollen alle vom Tuner/RDS-Decoder verfügbaren Informationen erfasst werden, die für eine korrekte Senderauflösung benötigt werden. Das ist zunächst eine Anforderung für Inventarisierung und Datenmodell; noch keine Implementierung.

### Unbedingt prüfen / erfassen
- **PI-Code (Programme Identification):** zentrale Kennung aus RDS; Rohwert unverändert speichern und erst nach Prüfung des Standards interpretieren.
- **Empfangsfrequenz:** aktuelle UKW-Frequenz in einer eindeutig festgelegten Einheit intern speichern; die RadioDNS-Abfrage muss die im Standard vorgeschriebene Frequenzdarstellung verwenden.
- **ECC / Länderkennung:** Extended Country Code, sofern der Decoder ihn liefert. Nicht stillschweigend aus dem Standort des Nutzers ableiten; Verfügbarkeit und korrekte Erfassung im vorhandenen Tuner-Treiber prüfen.
- **Band und Empfangsart:** für den ersten Schritt UKW/FM festhalten, damit keine nicht-terrestrischen oder anderen Banddaten in die UKW-RadioDNS-Auflösung geraten.
- **RDS-PS / Sendername:** zur Anzeige und Plausibilitätsprüfung erfassen, aber nicht als alleinigen eindeutigen Schlüssel verwenden.

### Weitere RDS-Daten – erfassen, wenn verfügbar
- **PTY** (Program Type) und **TP/TA** (Traffic Programme/Announcement), falls vom Decoder bereitgestellt.
- **AF** (Alternative Frequencies), sofern unterstützt; nützlich als Zusatzinformation, aber nicht ohne Prüfung als Ersatz für die aktuell empfangene Frequenz verwenden.
- **RT** (RadioText) und **RT+** (strukturierte Tags, z. B. Titel/Interpret), für die Anzeige im Radio-UI. Diese sind nicht automatisch Voraussetzung für die grundlegende RadioDNS-Senderauflösung.
- **RDS-Signal-/Decoderstatus** (z. B. gültiger PI, Synchronisation oder Empfangsqualität), sofern verfügbar, damit keine unvollständigen oder flackernden Daten unnötige Netzwerkabfragen auslösen.

### Geplanter Ablauf
1. RDS-Decoder-Datenmodell und tatsächliche Rückgabefunktionen der gewählten Tuner-Bibliothek inventarisieren.
2. PI, Frequenz und ECC/Länderkennung als zusammengehörigen Senderidentitätsdatensatz führen; PS als Anzeige-/Plausibilitätsfeld ergänzen.
3. RadioDNS erst dann abfragen, wenn die erforderlichen Werte valide und stabil genug sind. Senderwechsel oder wechselnde/ungültige PI-Werte müssen laufende Abfragen abbrechen oder deren Ergebnisse verwerfen.
4. Die RadioDNS-Auflösung gemäß offizieller Spezifikation durchführen; DNS-/Metadatenfehler begrenzen und Ergebnisse anhand der Senderidentität plausibilisieren.
5. Logo-Download und Bilddecodierung getrennt vom Empfangs-/Tuningpfad ausführen; Ergebnis cachen und bei Fehlschlag normal mit PS/RT weiterarbeiten.

### Offene technische Prüfungen
- Welche Felder liefert der konkrete ATS-Mini-Tuner und welche sind in der vorhandenen Bibliothek/API zugänglich?
- Wie werden PI, ECC und Frequenz in der verwendeten Bibliothek repräsentiert und validiert?
- Ist ECC im jeweiligen Empfangsfall verfügbar, oder braucht es einen definierten Fallback nach RadioDNS-Spezifikation?
- Welche RadioDNS-DNS-Namensbildung, Antworttypen und SI-Versionen sind für den unterstützten FM-Dienst erforderlich?
- Welche maximalen Größen, Formate und URLs akzeptiert der Logo-Cache?

Die offizielle RadioDNS-Spezifikation ist maßgeblich; keine DNS-Namen oder Feldkodierungen aus Vermutung zusammensetzen.


## 9. Ergebnis der ersten Tuner-/RDS-Inventarisierung (read-only, 2026-11-01)

Die folgenden Aussagen stammen aus der Sichtung der Dateien `ats-mini/SI4735-fixed.h`, `ats-mini/Station.cpp`, `ats-mini/Common.h` und `ats-mini/ats-mini.ino` im Referenz-Repository `13HN2709/ats-mini-dev` (main). Das Referenz-Repository blieb unverändert.

### Im Anwendungscode nachgewiesen
- **PI:** `SI4735_fixed::getRdsPI()` setzt BLOCKAH und BLOCKAL zu einem 16-Bit-Wert zusammen. Der Wrapper dokumentiert dies als Korrektur eines Fehlers in der Basisbibliothek.
- **PS:** `getRdsStationName()` verwendet Gruppe 0A; vollständiger Sendername wird aus vier geordneten 2-Zeichen-Segmenten gebildet.
- **PTY:** `getRdsProgramTypeX()` liest die fünf PTY-Bits aus Block B; Anzeige nutzt RDS- bzw. optional RBDS-Tabellen.
- **RT:** Gruppe 2A/2B wird über die SI4735-API verarbeitet und als RadioText bereitgestellt.
- **CT:** `getRdsUTCEpoch()` dekodiert Gruppe 4A als RDS-Uhrzeit.
- **RDS-Abruf/Status:** `checkRds()` prüft die FIFO-Belegung und verarbeitet begrenzt Gruppen pro Schleifendurchlauf. Die RDS-Konfiguration wird im FM-Pfad aktiviert.

### Nicht nachgewiesen / offene Prüfung
- **RT+:** kein RT+-Parser oder RT+-Tag-Handling in den geprüften Anwendungspfaden gefunden.
- **ECC:** keine eindeutige ECC-Extraktion/Getter-Funktion in den geprüften Anwendungspfaden gefunden.
- **AF:** keine explizite AF-Dekodierung/-Sammlung in den geprüften Anwendungspfaden gefunden.
- **TP/TA:** keine eigene Auswertung/Anzeige in den geprüften Anwendungspfaden gefunden; der Wrapper kommentiert explizit, dass Gruppe 0 Bit 4 TA ist und nicht das RadioText-A/B-Flag.
- **RadioDNS-Validierung:** es gibt noch keinen vollständigen Datensatz aus PI + Frequenz + ECC mit Stabilitäts-/Validitätsprüfung und keine RadioDNS-Abfrage.

### Technische Einschränkung
Dies belegt nur, was in den geprüften Anwendungspfaden sichtbar ist. Es beweist nicht, dass die SI4735-Hardware oder die PU2CLR-Bibliothek die übrigen Felder grundsätzlich nicht liefern kann. Vor Implementierung sind die exakt verwendete PU2CLR-Version 2.1.8, der SI4735-Datenpfad und die RadioDNS-Spezifikation zu prüfen. Fehlende API-Funktionen dürfen nicht geraten werden.

### Konsequenz für die Planung
1. Vorhandene Basis: PS, PI, PTY, RT und CT weiterverwenden bzw. in ATS Mini OS bewusst neu kapseln.
2. RT+, ECC, AF und TP/TA als Recherche-/Implementierungsfragen offen halten, bis die zugrunde liegenden RDS-Gruppen und API-Zugriffe verifiziert sind.
3. RadioDNS erst planen, wenn Identitätsdaten valide/stabil erfasst werden und die normgerechte Frequenz-/Länderkennung geklärt ist.
4. Diese Bestandsaufnahme ist read-only und keine Freigabe für ausführbaren Code, Builds, GitHub Actions oder Flashvorgänge.


## 10. PU2CLR SI4735 2.1.8 API-Prüfung und RadioDNS-GCC

Geprüfte externe Quellen (read-only): offizielles Repository `pu2clr/SI4735`, Tag `2.1.8`, Dateien `src/SI4735.h` und `src/SI4735.cpp`; Release-Übersicht; offizielle RadioDNS-Dokumentation, Core Lookup TS 103 270 und GCC-Library-Seite.

### Bibliotheks-API bestätigt
- Status/FIFO: `getRdsStatus(...)`, `getRdsReceived()`, `getRdsSync()`, `getRdsSyncFound()`, `getRdsSyncLost()`, `getGroupLost()`, `getNumRdsFifoUsed()`.
- Dekodierte RDS-Felder: `getRdsPI()`, `getRdsGroupType()`, `getRdsVersionCode()`, `getRdsProgramType()`, `getRdsText0A()` (PS), `getRdsText2A()` / `getRdsText2B()` (RT), `getRdsTime()` (CT).
- Kein fertiger ECC-, AF-, TP/TA- oder RT+-Getter/Parser in den geprüften öffentlichen API-Deklarationen gefunden. Das bedeutet nicht, dass die Rohgruppen nicht auswertbar sind; der konkrete FIFO-Datenzugriff ist der nächste Prüfpunkt.

### RadioDNS-GCC: wichtiger Präzisionspunkt
- Aktuelle RadioDNS-Dokumentation sagt, dass der FM-Core-Lookup Frequenz, PI und GCC benötigt. GCC wird aus dem Country Code (erstes Nibble des RDS-PI) plus ausgestrahltem ECC gebildet.
- ECC ist optional im ausgestrahlten RDS. RadioDNS empfiehlt daher eine standardkonforme Ersatzstrategie über die GCC-Library bzw. Geolocation, statt ECC ungeprüft aus dem Empfangsort abzuleiten. Grenzempfang ist zu berücksichtigen.
- Bei Implementierung ist die aktuelle Core Lookup Spezifikation TS 103 270 maßgeblich. Historische RDNS01-Dokumente sind nicht als alleinige Implementierungsgrundlage zu verwenden.
- Quellen: https://radiodns.org/technical/documentation/ ; https://radiodns.org/technical/libraries/gcc-library/ ; https://radiodns.org/technical/documentation/howto/ .

### Noch offen
- In der SI4735-2.1.8-Implementierung untersuchen, wie FIFO-Gruppen ausgelesen und welche Blockdaten exponiert werden.
- Danach prüfen, ob Gruppe 1A (ECC), Gruppe 0A/0B (TP/TA/AF) und Gruppe 3A (RT+) anhand der gelieferten Daten zuverlässig dekodiert werden können.
- Noch keine Implementierung, kein Build, keine GitHub Actions und kein Flashen.
