# RadioDNS-Senderlogo: benötigte Daten und Diagnoseplan

**Stand:** 2026-11-01  
**Status:** Spezifikation/Planung auf Basis öffentlicher RadioDNS-Dokumentation; keine Firmwareimplementierung.

## Ziel

ATS Mini OS soll bei UKW-Empfang das zum tatsächlich empfangenen Sender passende Logo finden und – sofern ein gültiger Eintrag vorhanden ist – später lokal zwischenspeichern. Vor dem Logo-Download muss der Empfänger eine belastbare Senderidentität bilden und den standardisierten RadioDNS-Dienstweg erfolgreich durchlaufen.

Die Diagnoseansicht soll deshalb nicht nur PI/PS anzeigen, sondern den kompletten Weg von der Empfangsidentität bis zum ausgewählten Logo nachvollziehbar machen. Das ist zunächst ein Entwicklungs-/Diagnoseziel; die endgültige Benutzeransicht kann später einfacher sein.

## 1. Daten vom Tuner und RDS-Decoder

| Feld | Zweck | Einordnung |
|---|---|---|
| Aktuelle UKW-Frequenz | Teil des RadioDNS-Lookups; muss der tatsächlich abgestimmten Frequenz entsprechen | Für FM-Lookup erforderlich |
| Vollständiger PI-Code (16 Bit, 4 Hex-Zeichen) | Identifiziert den RDS-Dienst zusammen mit Frequenz und GCC | Erforderlich |
| ECC (Extended Country Code) | Ergänzt den PI-Country-Code zur global eindeutigen Länderkennung | Bevorzugt, wenn korrekt empfangen |
| GCC (Global Country Code) | RadioDNS-Lookup-Parameter aus PI-Country-Code und ECC; alternativ standardkonforme GCC-Fallback-Strategie | Erforderlich |
| RDS-Synchronisation und Gültigkeit | Verhindert Lookup anhand von Zufalls-/veralteten Daten | Qualitäts-/Diagnosefeld |
| Blockfehler/BLE und Gruppenstatus | Ungültige RDS-Blöcke verwerfen und Fehlerursachen sichtbar machen | Qualitäts-/Diagnosefeld |
| PS / Sendername | Plausibilitätskontrolle und sichtbarer Fallback; allein kein sicherer Schlüssel | Ergänzend |
| PTY | Programmtyp anzeigen bzw. RDS-Datenpfad prüfen | Ergänzend |
| RT / Radiotext | Zusätzliche Senderinformation und Fallback bei fehlendem Logo | Ergänzend |
| RT+ | Optionale strukturierte Tags in RT, falls ODA-Zuordnung und Tags unterstützt werden | Optional; nicht voraussetzen |
| AF | Alternative Frequenzen und mögliche spätere Senderverfolgung | Ergänzend; kein Kernfeld des aktuellen FM-Lookups |
| TP / TA | Verkehrsprogramm-/Verkehrsdurchsage-Status | Ergänzend |
| CT | RDS-Uhrzeit; kann den Diagnose-/Zeitstatus ergänzen | Ergänzend |

**Wichtig:** Für die eigentliche FM-RadioDNS-Identität sind Frequenz, vollständiger PI und GCC zentral. PS, PTY, RT, RT+, AF, TP/TA und CT sind keine Ersatzwerte für diese Identität. Ein „EU“-RDS-Regionssetting ist keine Länderkennung für RadioDNS.

## 2. Normalisierung und Identitätsprüfung

- Frequenz aus dem tatsächlichen Tunerzustand übernehmen; nicht unabhängig aus PS oder einer Stationsliste schätzen.
- RadioDNS-Frequenz als fünfstellige Zeichenfolge in Einheiten von 100 kHz normalisieren, mit führender Null unter 100 MHz. Beispiel: 103,9 MHz → `10390`; 95,8 MHz → `09580`.
- Den vollständigen 16-Bit-PI erhalten und als vier Hex-Zeichen normalisieren. Die bereits im Referenzprojekt dokumentierte Korrektur, die PI-High- und PI-Low-Byte kombiniert, darf nicht verloren gehen.
- ECC nur übernehmen, wenn Gruppe, Variante und Blockqualität korrekt sind. ECC wird nicht von jedem Sender ausgestrahlt und kann selten kommen.
- GCC nach aktueller RadioDNS-Spezifikation bilden. Vorrang hat ein valide empfangenes ECC. Wenn es fehlt, soll später die offizielle RadioDNS-GCC-Fallback-Strategie verwendet werden; nicht blind „EU“, Deutschland oder den Standort des Geräts als Senderland einsetzen. Grenzempfang muss berücksichtigt werden.
- Senderidentität erst als stabil markieren, wenn RDS-Synchronisation und die relevanten Identitätsdaten über einen geeigneten Zeitraum bzw. mehrere gültige Gruppen plausibel sind. Konkrete Schwellenwerte sind noch festzulegen und zu testen.
- Bei Frequenzwechsel PI/ECC/GCC und zugehörige Lookup-Ergebnisse invalidieren oder neu validieren, damit keine alten Senderdaten am neuen Sender erscheinen.

## 3. RadioDNS-Ablauf, den die Diagnoseansicht sichtbar machen soll

1. **RDS erfasst:** aktuelle Frequenz, PI, ECC, RDS-Sync und Blockqualität.
2. **Identität normalisiert:** formatierte Frequenz, vierstelliger PI, ECC-Quelle und berechneter/ermittelter GCC.
3. **Lookup-Name erstellt:** FM-FQDN gemäß aktueller TS 103 270. Beispiel nur als Formatmuster: `<frequency>.<pi>.<gcc>.fm.radiodns.org`.
4. **DNS-Auflösung:** Ergebnis/Status des RadioDNS-CNAME-Lookups anzeigen.
5. **Metadatenserver gefunden:** SRV-Abfrage für `_radioepg._tcp.<authoritative-fqdn>`; Host und Port nur als Diagnosewert zeigen.
6. **SI-Dokument geladen:** Service Information aus dem standardisierten Pfad `/radiodns/spi/3.1/SI.xml` abrufen. Die tatsächliche Serveradresse kommt aus DNS, nicht aus einem fest einprogrammierten Host.
7. **Dienst eindeutig zugeordnet:** in der SI-Datei den Service anhand des FM-Bearers `fm:<gcc>.<pi>.<frequency>` finden. Ein Treffer nur nach PS-Text ist nicht ausreichend.
8. **Logo ausgewählt:** passende `multimedia`-Referenz aus dem zugeordneten Service wählen.
9. **Logo geprüft und geladen:** URL, MIME-Typ, deklarierte Abmessungen, tatsächliche Antwortgröße und Downloadstatus prüfen.
10. **Cache aktualisiert:** Identität, gewählte Ressource, Cache-Zeit und Ergebnis speichern; bei Fehlern kein vorhandenes gültiges Cache-Logo zerstören.

Die Diagnose sollte pro Stufe einen klaren Status zeigen, etwa `WAIT_RDS`, `IDENTITY_INVALID`, `WAIT_DNS`, `SI_NOT_FOUND`, `SERVICE_NOT_MATCHED`, `LOGO_NOT_ADVERTISED`, `DOWNLOAD_REJECTED`, `CACHED` oder `READY`. Diese Statusnamen sind Vorschläge, keine bereits implementierten Konstanten.

## 4. Informationen aus der Service-Information (SI)

Die SI-Datei kann unter anderem Folgendes enthalten:

- `shortName`, `mediumName`, `longName` und Beschreibungen: verifizierter Name/Fallback, nicht der primäre Lookup-Schlüssel.
- `bearer`: Zuordnung zum tatsächlich empfangenen FM-Dienst. Der Bearer muss zum normalisierten GCC, PI und Frequenzwert passen.
- `mediaDescription/multimedia`: Logo-URL, Typ, MIME-Wert und – falls angegeben – Breite/Höhe.
- `radiodns`: FQDN und Service-Identifier des beschriebenen Dienstes.
- `serviceInformation`-Metadaten, Erstellungszeit und Nutzungsbedingungen/Terms, soweit vorhanden.

Die SI-Datei muss als XML/strukturierte Daten behandelt und begrenzt verarbeitet werden. Keine beliebige URL allein deshalb herunterladen, weil sie im Dokument steht. Die Zuordnung zum korrekten Service und die zulässigen Metadatenfelder sind vorher zu validieren.

## 5. Logo-Auswahl und Schutzgrenzen

- RadioDNS Project Logo nennt typische PNG-Logoabmessungen von 32×32, 112×32, 128×128, 320×240 und 600×600 Pixeln. SI-Beispiele können zusätzliche Größen und auch andere deklarierte Formate enthalten.
- Für das ATS-Display soll bevorzugt die kleinste geeignete, korrekt deklarierte Variante gewählt werden, die bei der tatsächlichen Logo-Fläche noch gut lesbar ist. Die endgültige Größe hängt vom späteren Layout ab.
- MIME-Typ, Dateisignatur, deklarierte und tatsächliche Dimensionen, Antwortgröße und Downloadzeit prüfen; SI-Angaben allein nicht blind vertrauen.
- Maximale Downloadgröße, Timeout, Redirect-Regeln, Parsergrenzen und Cachebudget müssen vor Implementierung festgelegt werden.
- Nur erlaubte Bildformate unterstützen, sobald der Decoder auf der Zielplattform und seine Lizenz geprüft sind. Bis dahin keine Annahme, dass PNG/JPEG bereits dekodierbar ist.
- Nutzungsbedingungen aus der SI beachten. RadioDNS stellt nicht selbst die Senderlogos bereit und garantiert nicht, dass jeder Sender ein Logo veröffentlicht.
- Netzwerkzugriffe nicht-blockierend halten. Radioempfang, Frequenzabstimmung, Encoder, RDS-Parser und Display dürfen bei DNS-/HTTP-Ausfällen nicht hängen.
- Bei fehlendem WLAN, fehlendem GCC, keinem DNS-Eintrag, fehlender SI, nicht passendem Bearer oder fehlendem Logo bleiben Sendername/RT und das lokale Radio bedienbar.

## 6. RDS-FIFO: ein Leser, mehrere Verbraucher

Die geprüfte PU2CLR-SI4735-2.1.8-Statusantwort enthält die RDS-Blöcke A–D samt Fehlerindikatoren im internen Statusobjekt. Der direkte öffentliche Zugriff ist nicht als allgemeiner Rohblock-Getter bestätigt. Ein zweiter unabhängiger Aufruf von `getRdsStatus()` könnte den FIFO weiterdrehen und vorhandene PS-/RT-Verarbeitung beeinträchtigen.

Daher muss später genau ein koordinierter RDS-FIFO-Leser jede gültige Gruppe erfassen und an getrennte Decoder/Verbraucher verteilen: PS, PI, PTY, RT, ECC, AF, TP/TA, CT und – falls korrekt implementiert – RT+. Der RadioDNS-Dienst liest nur den daraus gebildeten Stationsdatensatz und greift nicht direkt in den Tuner-FIFO ein.

## 7. EU-/Regions-Einstellung

Die Bedienoberfläche soll keine zusätzliche manuelle RDS-Region-Auswahl benötigen, sofern die vorhandene Systementscheidung dies bereits festlegt. Das ist jedoch eine UI-/RDS-Dekodierkonfiguration und ersetzt nicht die RadioDNS-GCC-Bildung. Der GCC wird aus gültigen Broadcast-Daten oder der standardkonformen Fallback-Strategie bestimmt. Das Gerät darf den Empfang eines Senders aus einem Nachbarland nicht allein wegen einer voreingestellten EU-/Deutschland-Region falsch zuordnen.

## 8. Quellen

- RadioDNS Project Logo – technische Details und SI-Elemente: https://radiodns.org/campaigns/project-logo/technical-details/
- RadioDNS – Senderlogo Schritt für Schritt, FM-FQDN, CNAME, SRV, SI-Datei und Bearer: https://radiodns.org/campaigns/project-logo/walkthrough-finding-radio-station-logos/
- RadioDNS GCC Library – GCC und Fallback bei fehlendem ECC: https://radiodns.org/technical/libraries/gcc-library/
- RadioDNS technische Dokumentation / aktuelle Standards: https://radiodns.org/technical/documentation/
- RadioDNS Project Logo FAQ / Nutzungsbedingungen: https://radiodns.org/campaigns/project-logo/frequently-asked-questions/
- PU2CLR SI4735, Tag 2.1.8: https://github.com/pu2clr/SI4735/tree/2.1.8
- Silicon Labs AN332: https://strawberry-linux.com/pub/Si4735%20Programming%20Guide.pdf

## 9. Noch offen vor Implementierung

- Vollständige Prüfung der aktuellen TS 103 270 und SPI/TS 102 818 auf konkrete Protokoll- und XML-Validierungsdetails.
- Sauberer, nicht konkurrierender RDS-Rohgruppen-Zugriff aus dem späteren ATS-Mini-OS-Tuneradapter.
- Nachweis der ECC-/AF-/TP-/TA-/RT+-Dekodierung anhand gültiger Gruppen und Fehlerindikatoren.
- Entscheidung zu DNS-/HTTP(S)-Bibliothek, Zertifikatsprüfung, Redirects und Timeout-Verhalten.
- Decoderformat(e), RAM/PSRAM-Bedarf, maximale Dateigröße, Cachebudget und Ablagepfad.
- Test mit mindestens einem deutschen UKW-Sender mit bestätigter RadioDNS-Registrierung und anschließend Tests für fehlende ECC, fehlendes Logo, WLAN-Ausfall und Senderwechsel.

**Arbeitsgrenze:** Dies ist eine Dokumentations-/Planungsdatei. Keine ausführbare Firmware, Builds, GitHub Actions oder Flashvorgänge wurden damit freigegeben.
