# RadioDNS-Schnittstellen und Diagnose-Testlayout

**Stand:** 2026-11-01  
**Status:** Architekturentwurf; keine ausführbaren Interfaces oder Firmwaredateien.  
**Zielgerät:** ATS Mini V4 / ESP32-S3; konkrete Display- und Tuner-APIs sind noch nicht verbindlich.

## 1. RadioDNS-Protokoll: geprüfte Eckpunkte

Die Umsetzung richtet sich nach den aktuellen veröffentlichten Standards:
- RadioDNS Core Lookup: ETSI TS 103 270 v1.4.1.
- Service and Programme Information (SPI/SI): ETSI TS 102 818 v3.5.1.
- RadioDNS Project Logo technische Anleitung.

### FM/RDS-Identität

Die standardisierte FM-Identität verwendet:
- `frequency`: genau fünf Zeichen, Einheiten von 100 kHz, Frequenzen unter 100 MHz mit führender Null.
- `pi`: vier hexadezimale Zeichen des vollständigen 16-Bit-PI.
- `gcc`: drei hexadezimale Zeichen des Global Country Code.

Beispiel nach ETSI: 103,9 MHz, PI `D1E0`, GCC `DE0` → FQDN `10390.d1e0.de0.fm.radiodns.org`; ServiceIdentifier `fm/de0/d1e0/10390`; FM-Bearer `fm:de0.d1e0.10390`.

Die FQDN- und Bearer-Bausteine müssen aus normalisierten Daten erzeugt werden. Die Beispielwerte sind nur zur Formatkontrolle gedacht, nicht als feste Senderdaten.

### ECC und Länder-Fallback

RadioDNS verlangt im aktuellen Core-Lookup den GCC; die frühere Alternative mit ISO-Ländercodes ist veraltet. ECC wird optional im RDS ausgestrahlt. Wenn er fehlt, ist der von RadioDNS empfohlene GCC-Fallback mit PI-/Länderkennung und Geolocation zu prüfen. Eine feste UI-Region „EU“ oder das Heimatland des Nutzers darf nicht als Sender-GCC ausgegeben werden.

### SI-Datei entdecken

Der übliche Project-Logo-Weg ist:
1. FM-FQDN nach Core Lookup auflösen und den autoritativen FQDN erhalten.
2. Den Metadatenserver über den SRV-Eintrag `_radioepg._tcp.<authoritative-fqdn>` ermitteln.
3. SI-Datei unter `/radiodns/spi/3.1/SI.xml` abrufen.
4. Den Service anhand des passenden FM-Bearers `fm:<gcc>.<pi>.<frequency>` identifizieren.
5. Die Logo-Variante aus `mediaDescription/multimedia` auswählen.

**Transporthinweis:** Die offizielle Project-Logo-Anleitung beschreibt den SI-Endpunkt über HTTP. Deshalb darf der Client nicht pauschal voraussetzen, dass jeder Sender HTTPS anbietet. Redirects und Transportregeln müssen explizit begrenzt und sicher behandelt werden. HTTPS kann verwendet werden, wenn der standardkonforme Serverpfad es unterstützt; eine nicht vorhandene HTTPS-Variante darf nicht stillschweigend als Fehler des RadioDNS-Eintrags interpretiert werden.

Die SI-Datei muss hinsichtlich Schema, Größe und Inhalt begrenzt verarbeitet werden. DNS ist der autoritative Entdeckungsweg; bei anderweitig bezogenen SI-Daten sind Bearer und FQDN gemäß RadioDNS-Hinweisen gegen DNS zu validieren. Keine beliebige Logo-URL allein aufgrund ihres Auftretens in XML laden.

## 2. Vorgeschlagene Modulgrenzen

Die folgenden Verträge sind fachliche Entwürfe. Sie verwenden absichtlich sprachneutrale Typnamen und sind **keine fertigen C++-Deklarationen**.

### A. Tuner-/RDS-Adapter

**Verantwortung:** Genau ein Leser verarbeitet die RDS-FIFO-Gruppe. Er veröffentlicht einen unveränderlichen Snapshot für andere Module.

Vorgeschlagene Daten:
- `frequencyHz` oder eine dokumentierte Frequenzdarstellung aus dem Radio Core; die endgültige Einheit wird einmal zentral festgelegt.
- `pi16`, `ps`, `pty`, `radioText`, `ct`.
- `ecc` mit Quelle, Gruppe/Variante und Gültigkeit.
- `tp`, `ta`, `af[]`, RT+-Tags nur, wenn deren Decoder verifiziert implementiert sind.
- `rdsSync`, `lastValidGroupAt`, `blockErrors`, `fieldValidity` und `generation` (ändert sich bei neuem Sender/Frequenzkontext).

Regeln:
- Kein RadioDNS-/UI-Modul darf `getRdsStatus()` zusätzlich aufrufen oder die FIFO direkt lesen.
- Ein ungültiges/fehlendes Feld wird explizit als ungültig bzw. unbekannt markiert; keine alten Daten unbemerkt weiterreichen.
- Bei Abstimmung auf eine neue Frequenz wird der Snapshot generation-basiert invalidiert, bis neue Daten stabil vorliegen.
- Die vorhandene PI-High-/Low-Byte-Korrektur aus dem Referenzprojekt darf bei einer späteren Übernahme nicht verloren gehen.

### B. Senderidentität / RadioDNS Identity Builder

**Eingaben:** stabiler RDS-Snapshot, aktuelle abgestimmte FM-Frequenz, optional freigegebene Standortinformation nur für den standardkonformen GCC-Fallback.

**Ausgaben:**
- `identityStatus`: `WAIT_RDS`, `INVALID_PI`, `WAIT_ECC_OR_FALLBACK`, `READY`, `STALE`.
- normalisierte fünfstellige Frequenz, vierstelliger PI und dreistelliger GCC.
- FQDN, ServiceIdentifier und FM-Bearer, sobald valide erzeugbar.
- Quelle und Vertrauensstufe des GCC (Broadcast-ECC oder normgerechter Fallback).
- Zeitstempel/Generation des Eingangssnapshots.

**Regeln:** Der Builder führt keine DNS- oder HTTP-Aufrufe aus. Er erzeugt keine Identität aus PS allein. Alle Formate werden mit Unit-Tests auf führende Null, Hex-Groß-/Kleinschreibung, Grenzfrequenzen und ungültige Werte geprüft.

### C. RadioDNS Discovery / Netzwerkdienst

**Verantwortung:** DNS-Auflösung und Ermittlung des Metadatenservers, ohne Tuner- oder Displayzugriff.

Vorgeschlagener Vertrag:
- Eingabe: vollständige, valide RadioDNS-Identität plus Abbruchtoken/Generation.
- Ausgabe: Ergebnisstatus, autoritativer FQDN, DNS-/SRV-Details und Zeit-/Fehlerdiagnose.
- Netzwerkzugriff erfolgt asynchron, mit begrenzten Timeouts, Abbruch und begrenzter Wiederholungsstrategie.
- Ergebnis einer alten Generation wird verworfen, wenn der Nutzer zwischenzeitlich die Frequenz gewechselt hat.
- DNS, HTTP und gegebenenfalls HTTPS werden als getrennte Fehlerstufen protokolliert.

### D. SI Service Parser und Bearer Matcher

**Verantwortung:** SI-XML begrenzt parsen, den tatsächlich empfangenen Service finden und nur dessen Metadaten weiterreichen.

Vorgeschlagener Ausgabe-Datensatz:
- Service-Identifier und autoritativer FQDN.
- Namen/Descriptions als Fallback-Anzeige.
- nur die `multimedia`-Einträge des Service, dessen FM-Bearer exakt zu GCC/PI/Frequenz passt.
- Logo-Metadaten: URL, `type`, `mimeValue`, Breite/Höhe sofern angegeben.
- `terms`/Nutzungsinformationen, soweit vorhanden.
- Parser-/Validierungsstatus und Quelle.

PS-/Sendernamen-Ähnlichkeit ist kein Ersatz für den Bearer-Match. Parserfehler, zu große XML-Antworten oder fehlender passender Service führen zu einem kontrollierten Fehlerstatus, nicht zu einem Absturz.

### E. Logo Selector / Image Cache

**Logo Selector:** bevorzugt eine passende kleine Variante für die tatsächliche Logo-Fläche; bei Gleichstand klare, deterministische Priorität. Die fünf typischen Project-Logo-Größen sind 32×32, 112×32, 128×128, 320×240 und 600×600 Pixel. Tatsächliche Metadaten können abweichen.

**Image Cache:** getrenntes Modul mit begrenzter Dateigröße, erlaubten Formaten, Decoderprüfung, Timeout, URL-/Redirect-Regeln, Cachebudget und Einträgen pro Serviceidentität. Ein fehlgeschlagener Download darf ein bereits gültiges Logo nicht überschreiben. Der Cache darf nicht mit der UI- oder Tuner-FIFO-Lebensdauer gekoppelt sein.

Noch offen: genaue Decoderwahl, Bildpufferbedarf, Cachegröße, TLS-Zertifikatsstrategie und HTTP-Redirect-Policy; vor Implementierung anhand des gewählten Frameworks prüfen.

## 3. Diagnose-Testlayout

Das Testlayout ist ein Entwicklungswerkzeug, kein endgültiger Hauptbildschirm. Es soll die gesamte Kette vom Empfang bis zum Logo sichtbar machen, aber Radio Core und Netzwerkmodul nicht selbst steuern.

### Anzeigeaufteilung (Vorschlag)

**Kopfzeile**
- links oben: aktuelle Frequenz, groß und gut lesbar; Frequenzänderung bleibt beim vorhandenen Radio Core/Encoder.
- daneben: FM/RDS-Sync-Status und WLAN-/Netzwerkstatus.
- darunter: PS/Sendername und PI.

**RDS-Datenblock**
- PI (16 Bit), ECC (mit Gruppe/Variante/Gültigkeit), GCC (inkl. Quelle), PTY, RT-Status.
- AF, TP/TA, CT und RT+ nur dann als echte Werte zeigen, wenn die jeweilige Decoderfunktion verifiziert Daten liefert.
- fehlende Werte als `—` bzw. `NICHT EMPFANGEN` darstellen, niemals veraltete Werte stehen lassen.

**RadioDNS-Pipeline**
- normalisierte Frequenz / PI / GCC.
- erzeugter FQDN und ServiceIdentifier.
- DNS-CNAME-Ergebnis, SRV-Ziel/Port, SI-HTTP-Status und Parserstatus.
- Bearer-Match, ausgewählte Logo-Variante, MIME/Abmessungen, Downloadstatus und Cachestatus.
- Fehlerstufe und letzter Zeitstempel.

**Logo-Vorschau**
- kleines Vorschaufenster mit „kein Logo“, „Suche“, „geladen“ oder konkretem Fehlerstatus.
- Vorschau wird aus dem Logo-/Cache-Modul geliefert, nicht aus einer vom Layout selbst geladenen URL.

### Bedienung

- Der Encoder verändert die Frequenz ausschließlich über den vorhandenen Radio-Core-Vertrag.
- Frequenzwechsel erhöht die Sendergeneration und entwertet laufende alte RadioDNS-Ergebnisse.
- Kein zusätzlicher manueller RDS-Region-Wahlschalter im Testlayout. Das Layout zeigt GCC-Quelle und Empfangsdaten; die „EU“-Region ist kein GCC.
- Eine manuelle Funktion „RadioDNS erneut prüfen“ kann später als Diagnoseaktion ergänzt werden; sie darf nicht den Tuner-FIFO auslesen.
- Das Layout bleibt responsiv: Netzwerkoperationen aktualisieren Status schrittweise und blockieren weder Encoder noch Audio-/RDS-Verarbeitung.

## 4. Digitalschrift für das Testlayout

### Primärer Kandidat: Oxanium

Für die Testansicht wird **Oxanium** als bevorzugter digitaler Display-Font vorgeschlagen. Die Google-Fonts-Metadaten führen Oxanium als Display-Schrift unter SIL Open Font License (OFL) mit den Subsets `latin` und `latin-ext`. Latin Extended ist für die gewünschte Abdeckung deutscher Umlaute und erweiterter lateinischer Zeichen wesentlich.

- Quelle/Metadaten: https://github.com/google/fonts/tree/main/ofl/oxanium
- Lizenzdatei/Projektquelle: https://github.com/sevmeyer/oxanium
- Google-Fonts-Projekt wird unter OFL geführt; die konkrete verwendete Fontdatei und Lizenz müssen später zusammen im Repository dokumentiert werden.

Vor dem Einbinden muss eine echte Glyphenprüfung an der heruntergeladenen Datei erfolgen: `Ä Ö Ü ä ö ü ß`, `À È É Ì Ò Ù`, `à è é ì ò ù`, Ziffern, Satzzeichen und Hexwerte `A–F`. Nur das Vorhandensein des Subsets ersetzt diesen Test nicht.

### Arabische Schrift: Noto Sans Arabic

Für spätere arabische UI-Texte wird **Noto Sans Arabic** als separate Schriftfamilie vorgemerkt. Die Google-Fonts-Metadaten führen Arabisch sowie Latin/Latin Extended als Subsets und die SIL Open Font License (OFL).

- Quelle/Metadaten: https://github.com/google/fonts/tree/main/ofl/notosansarabic
- Upstream: https://github.com/notofonts/arabic

Wichtige technische Grenze: arabische Schrift benötigt kontextabhängige Zeichenformen und bidirektionale Textverarbeitung. Das bloße Vorhandensein von Arabisch-Glyphen in einer Schriftdatei garantiert keine korrekte Darstellung durch die aktuelle Displaybibliothek. Im ersten Testlayout wird deshalb Oxanium für lateinische/digitale Diagnosetexte verwendet; Noto Sans Arabic bleibt ein eigener späterer Rendering-Test und wird nicht ohne geprüfte Shaping-/RTL-Unterstützung als funktionsfähig behauptet.

### Font-Renderer und Speicher

- Das ATS-Display verwendet in bisherigen Referenzständen LovyanGFX, aber die konkrete ATS Mini OS Render-Integration ist noch nicht implementiert.
- Vor dem Festlegen des Fontformats müssen LovyanGFX-API, FreeFont/TTF-Unterstützung, Glyphenabdeckung, RAM-/Flashkosten und eine mögliche Konvertierung zu kompakten Bitmap-Fonts geprüft werden.
- Keine Schriftdatei wird unbesehen als „bereits integriert“ behandelt. Quelle, Version, Lizenz und tatsächlich verwendete Zeichenabdeckung werden in einem Font-Inventar dokumentiert.
- Der Test muss mindestens die deutsche Zeichenfolge **„ÄÖÜ äöü ß – Empfang: 103,9 MHz – Grüße aus Münster“** ohne Ersatzzeichen, abgeschnittene Akzente oder Layoutüberlauf darstellen.

## 5. Abnahmekriterien für spätere Implementierung

1. RDS-Daten kommen aus genau einem koordinierten FIFO-Leser.
2. PI, ECC, GCC und Frequenz sind validiert und generation-konsistent.
3. FQDN, ServiceIdentifier und FM-Bearer stimmen mit TS 103 270 überein.
4. SI wird über den DNS-entdeckten Metadatenserver abgerufen; kein fest verdrahteter Host.
5. Nur ein exakt passender FM-Bearer darf Logo-Metadaten liefern.
6. Fehler in DNS, HTTP, SI oder Logo-Download beeinträchtigen Radioempfang und Encoder nicht.
7. Testlayout zeigt alle Zwischenzustände und stale-data-Vermeidung.
8. Oxanium zeigt deutsche Umlaute/ß korrekt; Glyphen werden an der tatsächlichen Fontdatei getestet.
9. Arabisch wird erst nach bestätigtem RTL-/Shaping-Pfad als unterstützt gekennzeichnet.
10. Cache-/Bildlimits und Nutzungsbedingungen sind dokumentiert und getestet.

## Quellen

- RadioDNS aktuelle Standards: https://radiodns.org/technical/documentation/
- RadioDNS Project Logo technical details: https://radiodns.org/campaigns/project-logo/technical-details/
- RadioDNS Logo discovery walkthrough: https://radiodns.org/campaigns/project-logo/walkthrough-finding-radio-station-logos/
- RadioDNS SI validation guidance: https://radiodns.org/technical/
- ETSI TS 103 270 v1.4.1 (Core Lookup): https://standards.iteh.ai/catalog/standards/etsi/f0706e64-7216-419e-9156-4d09d9021bd3/etsi-ts-103-270-v1-4-1-2022-05
- Google Fonts Oxanium metadata: https://github.com/google/fonts/tree/main/ofl/oxanium
- Google Fonts Noto Sans Arabic metadata: https://github.com/google/fonts/tree/main/ofl/notosansarabic

**Arbeitsgrenze:** Diese Datei definiert Anforderungen und vorgeschlagene Verträge. Sie implementiert keine Firmware, triggert keine Builds/Actions und ändert keine vorhandene Firmware.
