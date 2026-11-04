# Übergabeprotokoll – ATS Mini OS

> **Zweck:** Dieses Dokument ist die zentrale Übergabe für neue Chat-Sitzungen. Vor jeder Fortsetzung zuerst diese Datei lesen und anschließend den tatsächlichen Repository-Stand prüfen.
>
> **Stand:** 2026-11-03  
> **Repository:** https://github.com/13HN2709/ATS_Mini_-OS  
> **Status:** ATS Mini V4-Hardwaretest vom Nutzer bestätigt: Radio startet mit Audio auf 95,400 MHz, RDS/PS/RT bleiben stabil ohne Flackern, Encoder reagiert korrekt. RadioDNS-Logoabruf trifft bei Radioplayer auf HTTP 401; deshalb wurde auf `dev/first-build-preparation` ein zusätzlicher RAM-only-Testpfad über den FM-DX-Webserver-Logoserver für den deutschen Testfall PI D399 ergänzt. Noch nicht kompiliert und nicht auf dem Gerät geprüft. Kein Build durch den Assistenten gestartet; kein Flash-/Bootloader-/Partitionsschritt durchgeführt.

## 1. Aktueller Auftrag

Ein eigenständiges, modular aufgebautes Betriebssystem-/Firmwareprojekt für den **ATS Mini V4** planen und später schrittweise entwickeln. Die Architektur soll so gestaltet sein, dass weitere ESP32-basierte Radios später ergänzt werden können, ohne den Kern neu schreiben zu müssen.

**Wichtig: Im Moment nur planen und inventarisieren.** Keine Builds, keine GitHub-Actions starten, keine Firmware flashen und keine Änderungen an bestehenden Radio-Firmware-Repositories vornehmen, solange der Nutzer das nicht ausdrücklich freigibt.

## 2. Schutz bestehender Firmware

- Die aktuell gut funktionierende Firmware in **Slot A** muss unangetastet bleiben.
- **DiamondRadio in Slot B** muss ebenfalls unangetastet bleiben.
- Vor späteren Build- oder Flash-Schritten müssen Partitionslayout, Slot-Größen, Image-Format, Bootloader-/Bootmanager-Anforderungen und Recovery-Verfahren anhand belastbarer Quellen geprüft werden.
- Keine vorhandenen Projekte, Branches, Releases oder Workflow-Dateien verändern, um dieses neue OS aufzubauen.
- Der Nutzer startet GitHub Actions selbst. Keine Workflows auslösen.
- Eine vollständige Sicherung namens `ATSMiniFULLBackup.nbin` mit 16 MB wurde in früherem Kontext erwähnt; vor riskanten Arbeiten trotzdem den aktuellen Sicherungsstatus bestätigen.

## 3. Repository-Status

Dieses Repository wurde neu erstellt und war bei der ersten Prüfung **leer**. Inzwischen wurde ein nicht ausführbares Dokumentations- und Ordnergerüst auf `main` angelegt:
- Repository: `13HN2709/ATS_Mini_-OS`
- Standardbranch: `main`
- Sichtbarkeit: privat
- Branch: `main`
- Enthalten sind `README.md`, dieses Übergabeprotokoll, Architektur-/Hardware-/Boot-/Risiko-Dokumentation sowie README-Platzhalter für geplante Module.
- Es gibt weiterhin keinen ausführbaren Firmware-Code, keine Build-Konfiguration und keine Partitionsdatei zur technischen Prüfung.

Alle Aussagen zur tatsächlichen Quellcode-Architektur müssen erst nach dem Hinzufügen und Prüfen von Quellcode bestätigt werden.

## 4. Bestätigte Anforderungen und Grenzen

### Zielplattform und Ausgangslage
- Hauptziel: ATS Mini V4 auf ESP32-S3.
- Für das Gerät wurden zuvor 16 MB Flash genannt. Die genaue Speicheraufteilung muss anhand der tatsächlich verwendeten Hardware-/Bootmanager-Konfiguration bestätigt werden.
- In bisherigen Firmwareprojekten kamen unter anderem PU2CLR SI4735 `2.1.8` und LovyanGFX `1.2.32` vor. Ob und wie diese übernommen werden, ist noch offen und muss anhand der Lizenz, Hardware-Anbindung und des tatsächlichen Quellcodes entschieden werden.
- Ein früheres Entwicklungsprojekt existiert unter `13HN2709/ats-mini-dev`; es ist **nicht** dieses neue OS-Repository. Es darf nur lesend als mögliche Referenz untersucht werden, sofern der Nutzer das wünscht.

### Architekturprinzipien
- Modularer Aufbau mit klaren Schnittstellen.
- Hardwareabhängigkeiten in einer HAL (Hardware Abstraction Layer) bzw. in Treibern kapseln.
- Radio-/Tuner-Implementierungen von UI, Weboberfläche und allgemeinen Systemdiensten trennen.
- Funktionen nur als verfügbar ausweisen, wenn die konkrete Hardware sie unterstützt.
- Bestehende Firmwarefunktionen nicht ungeprüft kopieren; erst Quellcode und Lizenzbedingungen prüfen.
- Neue Features möglichst isoliert hinzufügen, um funktionierende Bereiche nicht zu beschädigen.

### Speicherbusse für spätere Radio-Varianten
Der Nutzer hat festgelegt, dass bei der Betrachtung weiterer Radio-Varianten **nur OSPI und QSPI** berücksichtigt werden sollen; andere Speicherbus-Optionen sollen nicht Teil dieses Umfangs sein. Diese Vorgabe betrifft die entsprechende Speicher-/Hardware-Abstraktion und ist nicht mit einem Tuner- oder Funkstandard gleichzusetzen. Vor einer Implementierung klären, welche konkrete Hardware OSPI/QSPI tatsächlich nutzt und welche Treiber/Controller verfügbar sind.

### CPU-Takt und Performance-Steuerung

Vom Nutzer gewünschte Funktion für die Architekturplanung:

- CPU-Takt soll in den Systemeinstellungen **auswählbar** sein.
- Für rechenintensive Layouts bzw. Ansichten, insbesondere **Waterfall**, soll optional **Maximalleistung („Vollgas“) ** angefordert werden können.
- Das soll möglichst über einen zentralen **Performance Manager** bzw. ein klar definiertes Leistungsprofil umgesetzt werden, nicht über verstreute direkte CPU-Takt-Aufrufe in einzelnen Layouts.
- Zu prüfen sind die tatsächlich unterstützten CPU-Frequenzen für die gewählte ESP32-S3-/Framework-Konfiguration, Energieverbrauch, Wärmeentwicklung, Stabilität und mögliche Auswirkungen auf Funk-/Display-Timing.
- Geplante Profile könnten später beispielsweise „Energiesparen“, „Normal“ und „Maximum“ heißen; konkrete Frequenzwerte und Profile sind noch **nicht festgelegt** und müssen anhand der Plattform geprüft werden.
- Ein Layout darf einen Performance-Wunsch melden (z. B. Waterfall benötigt Maximum); die zentrale Verwaltung entscheidet, ob und wie der Takt sicher angepasst wird. Beim Verlassen des Layouts muss der vorherige Modus wiederhergestellt werden.
- Diese Funktion ist nur als Anforderung dokumentiert; noch keine Implementierung oder Messung vorgenommen.

## 5. Repository-Struktur – Dokumentationsgerüst angelegt

Die folgenden Dateien und Platzhalter sind angelegt. Sie dokumentieren die geplante Struktur, enthalten aber noch keine ausführbare Firmware:

```text
ATS_Mini_-OS/
├── docs/
├── HANDOVER.md
│   ├── ARCHITECTURE.md
│   ├── HARDWARE_INVENTORY.md
│   └── PARTITIONS_AND_BOOT.md
├── app/                 # Start, Systemzustand, Hauptschleife
├── hal/                 # stabile Hardware-Schnittstellen
├── drivers/
│   ├── display/
│   ├── storage/         # ausschließlich benötigte OSPI-/QSPI-Pfade
│   └── tuner/           # konkrete Tuner-Treiber nach Inventarisierung
├── radio/               # gemeinsamer Radio-Kern und Fähigkeiten
├── modules/
│   ├── spectrum/
│   ├── dual_rx/
│   └── web_control/
├── ui/
│   └── layouts/          # Layout-Platzhalter
├── services/
│   ├── config/           # Konfigurationsdienst
│   └── performance/      # CPU-Leistungsprofile
├── tests/
└── tools/
```

Weitere Unterordner und Dokumente können während der Inventarisierung ergänzt werden. Ausführbare Implementierungen bleiben bis zur Architekturentscheidung und separaten Freigabe zurückgestellt.

## 6. Geplante Funktionsmodule

Noch keine Implementierung zugesagt oder begonnen:

- **Radio Core:** Band, Frequenz, Modulation, Tuning und Fähigkeitsabfrage.
- **Display/UI:** Displaytreiber, Layouts, Texte, Eingaben und Encoder.
- **Spectrum Scanner:** getrennte Scan-Engine und Darstellung.
- **Dual RX:** nur entsprechend nachgewiesener Tuner-/Hardwarefähigkeiten; kein Versprechen von gleichzeitigem Empfang ohne technische Bestätigung.
- **Web Control:** Weboberfläche und Verbindungsmodi getrennt vom Radio-Kern.
- **Hardware-Abstraktion:** ermöglicht später weitere kompatible ESP32-Radios.
- **OSPI/QSPI:** ausschließlich die vom Nutzer gewünschten Speicherbus-Optionen berücksichtigen.

Diese Module sind Zielideen. Verfügbarkeit und Aufwand werden erst nach der Inventarisierung festgelegt.

## 7. Inventarisierung – nächste Arbeitsschritte

1. Repository und Branch-Stand prüfen; dieses Protokoll und die bereits angelegte Dokumentation als Ausgangspunkt verwenden.
2. Mit Zustimmung des Nutzers relevante Referenzquellen **nur lesend** untersuchen, insbesondere `13HN2709/ats-mini-dev`.
3. Hardware und Pinbelegung dokumentieren: ESP32-S3-Variante, Display/Controller, Tuner, Encoder, Speicher, Netzwerk und sonstige Peripherie.
4. Buildsystem und Versionen ermitteln: Arduino-ESP32 oder ESP-IDF, Compiler, Framework, Bibliotheken und Toolchain.
5. Partitionstabelle, App-Slotgrößen, Bootmanager-Metadaten, Image-Format, OTA-/Recovery-Pfade und Bootreihenfolge verifizieren.
6. Vorhandene Funktionen und Treiber mit Quellcodepfaden sowie Status (bestätigt / vermutet / offen) dokumentieren.
7. Architektur und Modulgrenzen schriftlich weiter ausarbeiten; vor ausführbarem Quellcode die relevanten Entscheidungen festlegen.
8. Erst nach ausdrücklicher Freigabe einen minimalen Kern planen bzw. implementieren. Builds und Hardwaretests sind ein separater, später freizugebender Schritt.

## 9. Risikoregister

| Risiko | Gegenmaßnahme |
|---|---|
| Bestehende Slot-A-Firmware wird überschrieben | Keine Flash-Aktion; Partitions- und Bootpfade zuerst verifizieren |
| DiamondRadio in Slot B wird beeinträchtigt | Slot B nicht verändern; Image-/Slot-Kompatibilität dokumentieren |
| Firmware-Image passt nicht in App-Slot | Slotgröße und Imagegröße vor jedem späteren Build/Flash abgleichen |
| Tuner kann Dual RX nicht wirklich parallel | Hardwarefähigkeiten und Treiber prüfen; Funktion nur entsprechend ausweisen |
| Bibliotheks-/Framework-Versionen inkompatibel | Versionen aus Quellcode und Buildkonfiguration ermitteln, nicht raten |
| Übernahme von Fremdcode oder Layouts verletzt Lizenz-/Wartungsgrenzen | Herkunft und Lizenz prüfen, Module isolieren |
| Übergabestand driftet zwischen Chats | Dieses Dokument bei wichtigen Entscheidungen, abgeschlossenen Inventarabschnitten und Statusänderungen aktualisieren |

## 10. Arbeitsregeln für neue Chats

Zu Beginn jeder neuen Sitzung:

1. `HANDOVER.md` bzw. dieses Übergabeprotokoll lesen.
2. Den tatsächlichen GitHub-Stand prüfen; das Dokument ist eine Übergabehilfe und kann hinter dem Repository zurückliegen.
3. Zwischen **bestätigten Fakten**, **Vorschlägen** und **offenen Fragen** unterscheiden.
4. Keine Änderungen, Builds, Actions oder Flash-Vorgänge ohne ausdrückliche Freigabe.
5. Nach wichtigen neuen Entscheidungen den Übergabetext aktualisieren und den Commit-Link bzw. die Commit-SHA im Chat nennen.
6. Den Nutzer informieren, wenn eine Aktualisierung nicht möglich war; niemals behaupten, das Dokument aktualisiert zu haben, ohne den GitHub-Schreibvorgang bestätigt bekommen zu haben.

## 22. Digitalschrift für das RadioDNS-Testlayout

- Für den ersten Test wird **DSEGstended 14-Segment Regular** als Kandidat bevorzugt, statt Oxanium.
- Quelle: https://github.com/ZeusOfTheCrows/DSegstended
- Das Projekt nennt die SIL Open Font License 1.1 und zusätzliche lateinische Akzentzeichen, griechische/kyrillische Großbuchstaben sowie `ß`, `Æ/æ`, `Œ/œ` und `Ø/ø`.
- Vor dem Einbau sind die konkrete Fontdatei, Lizenz, tatsächliche Glyphen und Lesbarkeit in mehreren Größen zu prüfen: große Frequenz, mittlere Sender-/Statuszeilen und kleine Diagnosewerte.
- Pflichtzeichen: `Ä Ö Ü ä ö ü ß`, italienische Akzente, Ziffern, Satzzeichen und Hexwerte.
- Noto Sans Arabic bleibt für einen separaten RTL-/Shaping-Test vorgemerkt.
- **Status: nur Auswahl und Dokumentation; Fontdatei nicht integriert, nicht auf dem Display getestet, keine Builds/Actions gestartet.**

## 11. Änderungsverlauf

- **2026-11-01:** Erstes Übergabeprotokoll erstellt. Neues Repository geprüft und als leer bestätigt. Planungsvorgaben, Schutz bestehender Slots, OSPI/QSPI-Einschränkung und nächste Inventarisierungsschritte dokumentiert.\n- **2026-11-01:** CPU-Takt-Auswahl und layoutabhängige Maximalleistung (insbesondere Waterfall) als Anforderungen ergänzt; Umsetzung bleibt bis zur Plattformprüfung offen.
- **2026-11-01:** Nicht ausführbares Projektgerüst angelegt: Root-README, Architektur-, Hardware-, Boot-/Partitions-, Modul-/Schnittstellen- und Risiko-Dokumentation sowie README-Platzhalter unter `app/`, `hal/`, `drivers/`, `radio/`, `modules/`, `ui/`, `services/`, `tests/` und `tools/`. Keine Builds, Workflows oder Flashvorgänge ausgeführt.

- **2026-11-01:** WLAN-Erweiterung als Planungsanforderung ergänzt: Wetterbericht per Postleitzahl, lokale Wetter-Icons, Offline-Cache/Status, späterer Bilddownload/-decoder mit begrenzten Ressourcen. Neue README-Platzhalter unter modules/weather/ und services/image-cache/; keine Firmware implementiert und keine Builds/Actions/Flashvorgänge ausgeführt.

## 12. Verbindlicher 3-Punkteplan für die Bestandsaufnahme

Der Nutzer hat ausdrücklich darum gebeten, diesen Plan dauerhaft festzuhalten und bei der Fortsetzung wiederzuverwenden:

1. **WLAN / Netzwerk / HTTPS (nur lesend):** vorhandene Verbindungsmodi, Internetzugang, HTTP(S)-Client, Zertifikatsprüfung, Zeitdienst und nicht blockierende Netzwerkabläufe untersuchen.
2. **Display / Bilddarstellung:** vorhandene Zeichen- und Sprite-Funktionen prüfen, feststellen, ob ein Bilddecoder bereits vorhanden ist, und mögliche Formate erst nach Quellcode-/Bibliotheksprüfung festlegen.
3. **Flash / Partitionen / Speicher:** Build-Konfiguration und Partitionstabelle mit Vorsicht auswerten; App-Slots, Dateisystem, NVS und Grenzen dokumentieren. Eine konfigurierte Flash-Größe ist kein Beweis für die tatsächlich bestückte Flash-Hardware.

### Erste Ergebnisse aus der rein lesenden Prüfung von `13HN2709/ats-mini-dev`

**Punkt 1 – WLAN/HTTPS**
- `ats-mini/Network.cpp` verwendet `WiFi.h`, `WiFiMulti.h`, `WiFiUdp.h`, `AsyncTCP.h`, `ESPAsyncWebServer.h`, `NTPClient.h` und `ESPmDNS.h`.
- Vorhandene Modi umfassen laut `netInit()` unter anderem AP Only, AP+STA und STA. **AP Only allein stellt keinen Internetzugang bereit**; für Wetterdaten muss eine funktionierende Verbindung zu einem Router/Internet vorhanden sein.
- Ein NTP-Zeitdienst und ein asynchroner Webserver auf Port 80 sind vorhanden.
- `ats-mini/Ota.cpp` nutzt `HTTPClient` und `NetworkClientSecure` mit dem im ESP32-Core enthaltenen CA-Zertifikatsbundle. Das belegt eine vorhandene HTTPS-Technik für OTA; es ist noch keine fertige Wetter-API-Integration.
- Netzwerkzugriffe für neue Module müssen begrenzt und so entworfen werden, dass Radio, Tuning, Encoder, Scanner und Dual RX nicht durch lange Wartezeiten blockiert werden.

**Punkt 2 – Display/Bilder**
- `ats-mini/Display.h` definiert den LovyanGFX-basierten Displaytreiber. In `ats-mini/ats-mini.ino` wird ein 320×170-Sprite mit 16-Bit-Farbtiefe und PSRAM-Nutzung angelegt.
- `ats-mini/Draw.cpp` zeigt derzeit vor allem text- und primitivebasierte Darstellung. In den bisher geprüften Dateien wurde noch kein allgemeiner JPEG-/PNG-/BMP-Decoder bestätigt.
- `Network.cpp` enthält einen Web-Upload-Pfad für eine Splash-Datei mit einer Grenze von 512 KiB. Das allein beweist **nicht**, welche Bildformate decodiert oder auf dem Display angezeigt werden können. Decoder, Dateiformat, Speicherbedarf und Lizenz müssen separat geprüft werden.
- Für den ersten Wetter-Prototyp bleiben lokale Wetter-Icons die ressourcenschonende Ausgangsidee; heruntergeladene Bilder sind ein späterer, separat zu prüfender Schritt.

**Punkt 3 – Flash/Partitionen**
- `ats-mini/sketch.yaml` setzt sowohl im Profil `esp32s3-ospi` als auch `esp32s3-qspi` die Build-Option `FlashSize=8M`. Das ist eine Build-Konfiguration und bestätigt nicht die physische Flash-Größe des konkreten Geräts.
- `ats-mini/partitions.csv` definiert zwei OTA-App-Partitionen `app0` und `app1`, jeweils `0x300000` (3 MiB), sowie `littlefs` mit `0x1d0000` (1.8125 MiB), dazu NVS-/Settings- und Core-Dump-Partitionen. Die Tabelle reicht bis Offset `0x800000` (8 MiB).
- Das muss vor jeder späteren Speicher-/Bootplanung mit dem tatsächlichen Gerät, dem Bootmanager und der aktuellen Sicherung abgeglichen werden. Keine Partitionsdatei ändern und nichts flashen.

**Geprüfte Referenzpfade**
- `ats-mini/sketch.yaml`
- `ats-mini/partitions.csv`
- `ats-mini/Network.cpp`
- `ats-mini/Ota.cpp`
- `ats-mini/Storage.cpp`
- `ats-mini/Display.h`
- `ats-mini/Draw.cpp`
- `ats-mini/ats-mini.ino`
- `docs/source/flash.md`
- `docs/source/development.md`

**Weiterhin strikt ausgeschlossen:** Builds, Auslösen von GitHub Actions, Flashen sowie Änderungen an `13HN2709/ats-mini-dev` oder anderen bestehenden Firmware-Repositories. Die bisherige Untersuchung war ausschließlich lesend. Die drei Punkte sind Bestandsaufnahme, keine Freigabe für Implementierung.

## 13. Erste Web-/GitHub-Recherche zu Wetterdaten und RDS-Logos

Die erste Recherche ist in `docs/NETWORK_DATA_SOURCES.md` dokumentiert:
https://github.com/13HN2709/ATS_Mini_-OS/blob/main/docs/NETWORK_DATA_SOURCES.md

Vorläufige Auswahl:
- **Wetter:** Open-Meteo ist der bevorzugte erste Kandidat, weil Geocoding Ortsnamen/Postleitzahlen unterstützt und die Wetter-API aktuelle Bedingungen, Wettercodes, Bewölkung, Niederschlagswahrscheinlichkeit und Wind liefert. Nutzung für nicht kommerzielle Zwecke grundsätzlich ohne API-Schlüssel; Daten-Attribution und Terms beachten.
- **RDS-Logos:** RadioDNS Project Logo ist der fachlich passende Standardkandidat, weil Sender über Frequenz, PI-Code und Länderkennung auf Service-Informationen mit Metadaten/Logo-Referenzen auflösen können. Die Verfügbarkeit ist abhängig davon, ob der jeweilige Sender RadioDNS korrekt veröffentlicht.
- **Fallback-Kandidat:** Radio-Browser kann Sendername, Homepage und Favicon liefern, ist aber nicht garantiert eindeutig mit dem terrestrischen RDS-Sender verknüpft. Treffer müssen bewertet werden.
- **Bilddarstellung:** LovyanGFX nennt TJpgDec und Pngle als mögliche technische Referenzen; Espressif zeigt Decoderbeispiele für ESP32-S3. Es ist noch nicht bestätigt, dass ein allgemeiner Bilddecoder in der bestehenden ATS-Referenzfirmware bereits eingebunden ist.
- **Netzwerkarchitektur:** gemeinsame Netzwerkdienste, HTTPS-Zertifikatsprüfung, begrenzte Downloads, Cache und nicht blockierende Abläufe vorsehen. AP Only allein hat keinen Internetzugang; für externe Wetter-/Logo-Daten muss STA-Internet verfügbar sein.

Die Ergebnisse sind Recherche-Hinweise und noch keine Implementierung oder endgültige Anbieterfreigabe. Alle Schutzgrenzen aus diesem Dokument bleiben bestehen: Referenz-Repositories read-only, keine Builds/Actions/Flashvorgänge und keine Änderungen an Slot A oder Slot B.


## 14. Entscheidung: RadioDNS als bevorzugten Logo-Kandidaten aufnehmen

Der Nutzer hat zugestimmt, RadioDNS für die geplante automatische RDS-Senderlogo-Funktion weiterzuverfolgen. Die Details und Quellen stehen in `docs/NETWORK_DATA_SOURCES.md`, Abschnitt 7.

- RadioDNS ist der bevorzugte erste Kandidat für die Recherche/Erprobung der Senderlogo-Suche.
- Als Identität sollen PI-Code, Frequenz/Band und Länderkennung/ECC nach Spezifikation verwendet werden; der PS-Sendername ist nur ergänzend.
- Logo-Downloads müssen begrenzt, geprüft und lokal gecacht werden. Ohne Treffer oder WLAN bleibt Sendername/RDS-Text sichtbar.
- Es ist nicht verifiziert, dass die TEF6686-App des Nutzers RadioDNS tatsächlich nutzt; dies bleibt eine Vermutung.
- RadioDNS ist keine Garantie für Logos bei allen Sendern und ersetzt nicht die Prüfung von Lizenzbedingungen, HTTPS, Bildformaten und Speicherbedarf.
- Dies ist eine Planungsentscheidung, keine Freigabe für ausführbaren Code, Builds, Actions oder Flashvorgänge. Bestehende Firmware-Repositories bleiben unangetastet.


## 15. Erstes Entwicklungsziel und schrittweiser Aufbau

Der Nutzer hat festgelegt, dass wir das ATS Mini OS zuerst als einfaches **UKW-Radio mit RDS, RT, PTY und RadioDNS** aufbauen und anschließend Schritt für Schritt erweitern. Der Umfang und die Stufen sind auch in `docs/ARCHITECTURE.md` dokumentiert.

### Geplante Reihenfolge
1. **Inventarisierung:** Hardware, Tuner, Pins, Audioausgabe, Display, Eingabe, WLAN/HTTPS und Zeitdienst anhand von Quellen bestätigen.
2. **UKW-Grundbetrieb:** FM-Abstimmung und Audio stabil zum Laufen bringen.
3. **RDS:** Sendername/PS und PI-Code auslesen und auf Stabilität prüfen.
4. **RT und PTY:** Radiotext und Programmtyp darstellen, sofern der Sender sie ausstrahlt und die Hardware sie liefert.
5. **RadioDNS:** Senderidentität nach Spezifikation auflösen und verfügbare Metadaten/Logos abrufen; zuerst mit einem bekannten deutschen Sender testen.
6. **Stabilität:** WLAN-Ausfall, fehlende Daten, Timeouts und Cache prüfen. Netzwerkfehler dürfen Radio und Bedienung nicht blockieren.
7. **Erst danach** weitere Funktionen wie Wetter, Web Control, Spectrum Scanner oder Dual RX planen/ergänzen.

### Regeln
- RDS/RT/PTY/Logo sind abhängig von Senderdaten und Hardwareunterstützung; fehlende Daten dürfen kein Fehler für das Radio sein.
- RadioDNS bleibt der bevorzugte erste Logo-Kandidat, ist aber keine Garantie, dass jeder Sender ein Logo veröffentlicht.
- Die Architektur-/Reihenfolgeentscheidung ist noch keine Freigabe für ausführbaren Code, Builds, Actions oder Flashvorgänge.
- Bestehende Slot-A- und Slot-B-Firmware und Referenz-Repositories bleiben unangetastet. Der aktuelle Status bleibt Planung/Inventarisierung.


## 15. RadioDNS: vollständige Erfassung der nötigen RDS-Daten

Der Nutzer hat festgelegt, dass wir für den ersten UKW-Radio-Ausbau alle für RadioDNS relevanten verfügbaren RDS-Informationen auslesen und erfassen sollen.

- Primäre Identitätsdaten zur Prüfung: **PI-Code, aktuelle UKW-Frequenz und ECC/Länderkennung** nach offizieller RadioDNS-Spezifikation.
- **PS/Sendername** ergänzend anzeigen und zur Plausibilitätsprüfung nutzen, nicht als alleinigen Schlüssel.
- Weitere verfügbare Felder erfassen: PTY, TP/TA, AF sowie RT und RT+; RT/RT+ dienen primär der Anzeige und sind nicht automatisch Voraussetzung für die RadioDNS-Auflösung.
- Zusätzlich Decoder-/Validitätsstatus prüfen, damit bei ungültigem oder instabilem PI keine unnötigen Abfragen gestartet werden.
- Als nächster Schritt zunächst den tatsächlichen Funktionsumfang der verwendeten Tuner-Bibliothek und den Zugriff auf diese Felder inventarisieren. DNS-Namen und Feldkodierungen nicht raten, sondern gemäß RadioDNS-Spezifikation umsetzen.
- Logoabfragen, Downloads und Bilddecodierung müssen außerhalb zeitkritischer Tuning-/Empfangspfade laufen; lokale Cache- und Fallback-Anzeige vorsehen.
- Details sind in `docs/NETWORK_DATA_SOURCES.md`, Abschnitt 8 dokumentiert.
- Dies bleibt Planung/Inventarisierung: keine ausführbare Implementierung, keine Builds, GitHub Actions oder Flashvorgänge ohne gesonderte Freigabe.


## 16. Aktueller Stand: 16-MB-Flash, LittleFS und Dual-Boot-Strategie

**Stand: 1. November 2026.** Der Nutzer hat den aktuellen Speicher- und Entwicklungsplan bestätigt.

### Flash-/Speicherplanung
- Der Nutzer hat anhand der Recovery-Anzeige bestätigt, dass der ATS Mini V4 **16 MB physischen Flash** hat.
- Der gezeigte Screenshot weist folgende Partitionen aus: `app0` 3,5 MB, `app1` 3,5 MB, `recovery` 1,6 MB, `littlefs` 5,6 MB und `ffat` 1,3 MB; weitere Systempartitionen belegen den übrigen Bereich. Werte aus der Recovery-Anzeige sind maßgeblich für diese Referenz, müssen aber vor einer späteren Neupartitionierung erneut am Gerät geprüft werden.
- LittleFS zeigt im Screenshot rund **2,5 MB frei**. Der Nutzer erklärt, dass dort zusätzlich ungefähr **2 MB `.ino`-Backup-Dateien** liegen. Wenn diese Dateien tatsächlich aus LittleFS gelöscht werden, wird mit ungefähr **4,5 MB freiem LittleFS-Speicher** gerechnet. Das ist eine Prognose nach dem Löschen, kein bereits bestätigter Messwert.
- Das Entfernen von Dateien aus LittleFS vergrößert keine App-Partitionen und ändert nicht die physische Flash-Kapazität.

### Dual-Boot- und spätere Vollspeicher-Strategie
1. Zunächst **Dual-Boot-Unterstützung beibehalten** und die neue Firmware für den vorgesehenen Dateinamen/Installationsweg `ats.mini-ino.bin` vorbereiten. Den exakten Dateinamen und das benötigte Binärformat vor einem späteren Build/Flash anhand des realen Update-Verfahrens verifizieren.
2. Slot A mit der funktionierenden Firmware und Slot B mit DiamondRadio bleiben unangetastet. Der Nutzer wechselt später selbst wieder auf das andere Dual-Boot-System, um die dortigen Speicherwerte bei Bedarf erneut zu prüfen.
3. Erst wenn ATS Mini OS stabil fertiggestellt und getestet ist, soll optional auf eine Einzel-Firmware ohne Dual Boot umgestellt und die Partitionierung für möglichst viel nutzbaren Speicher neu geplant werden.
4. Auch im späteren Einzel-Firmware-Betrieb benötigen Bootloader, Partitionstabelle, System-/Recovery-Bereiche und Dateisystem weiterhin geeigneten Platz; „gesamte 16 MB nutzen“ bedeutet, die Aufteilung sinnvoll zu optimieren, nicht jeden Byte ausschließlich für App-Code oder Nutzdateien zu reservieren.

### Projektziel und nächster Arbeitsschritt
- Langfristig soll ATS Mini OS die Funktionen selbst bereitstellen und den verfügbaren Speicher sinnvoll für Firmware, RadioDNS-Senderlogos, Wetterdaten/-grafiken, Cache und Konfigurationen nutzen. Vorgesehene Verzeichnisstruktur: `/radio/logos/`, `/radio/cache/`, `/weather/` und `/config/` (Planung, noch nicht implementiert).
- Erstes Funktionsziel bleibt ein UKW-Radio mit RDS, RT, PTY, RT+ und RadioDNS.
- **Aktueller Arbeitsstand: nur Planung und read-only Inventarisierung; es wurde noch keine ausführbare Firmware implementiert.**
- **Nächster konkreter Schritt:** die Tuner-Bibliothek und ihre tatsächlich verwendeten Aufrufpfade im Referenz-Repository `13HN2709/ats-mini-dev` nur lesend untersuchen und dokumentieren, welche RDS-Felder wirklich verfügbar sind (PI, PS, PTY, RT, AF, TP/TA, ECC/Länderkennung und RT+, soweit unterstützt). Verifizierte Fakten, Annahmen und offene Fragen getrennt halten.
- Keine Builds, keine GitHub Actions auslösen, nichts flashen und keine Änderungen an bestehenden Firmware-Repositories ohne ausdrückliche Freigabe. Das Referenz-Repository `13HN2709/ats-mini-dev` bleibt read-only.


## 17. Fortschritt: erste read-only Inventarisierung der RDS-Funktionen

Am 1. November 2026 wurde die erste Tuner-/RDS-Codeprüfung im Referenz-Repository `13HN2709/ats-mini-dev` (main) abgeschlossen. Es wurden nur Dateien gelesen; das Referenz-Repository blieb unverändert.

### Im vorhandenen Anwendungscode nachgewiesen
- PS/Sendername über Gruppe 0A und `getRdsStationName()`.
- PI über `SI4735_fixed::getRdsPI()`, das BLOCKAH + BLOCKAL zu einem 16-Bit-PI zusammensetzt (Wrapper-Korrektur für dokumentierten Basisbibliotheksfehler).
- PTY über `getRdsProgramTypeX()` (fünf Bits) mit RDS-/RBDS-Anzeigetabellen.
- RT über die vorhandenen Gruppe-2A/2B-Funktionen.
- RDS Clock Time über Gruppe 4A / `getRdsUTCEpoch()`.
- RDS-FIFO wird in `checkRds()` begrenzt ausgelesen; RDS wird im FM-Pfad konfiguriert.

### Noch nicht nachgewiesen
- RT+ Parser/Tags, ECC-Extraktion, AF-Sammlung sowie TP/TA-Auswertung sind in den geprüften Anwendungspfaden nicht vorhanden bzw. nicht nachgewiesen.
- Das ist kein Beweis, dass Hardware oder Bibliothek diese Felder unmöglich liefern; die konkrete PU2CLR-SI4735-Version 2.1.8, RDS-Gruppen, Rohblockzugriff und RadioDNS-Spezifikation müssen als Nächstes geprüft werden.
- Ein vollständiger, stabilitätsgeprüfter RadioDNS-Identitätsdatensatz aus PI + Frequenz + ECC ist noch nicht implementiert.

### Dokumentation aktualisiert
- `docs/ARCHITECTURE.md` – Abschnitt „RDS/Tuner-Bestandsaufnahme“; Commit `434e1fbd79da269c3d3f360393fbfd819497789e`.
- `docs/NETWORK_DATA_SOURCES.md` – Abschnitt 9 „Ergebnis der ersten Tuner-/RDS-Inventarisierung“; Commit `ecbc5216976fe2a172b639e027236cff890cd8d7`.

### Nächster Schritt
Die exakt verwendete PU2CLR-SI4735-Version 2.1.8 und die RDS-Rohblock-/FIFO-Semantik prüfen, um festzustellen, ob ECC, AF, TP/TA und RT+ über Bibliotheks- oder Rohdatenzugriff erschlossen werden können. Anschließend RadioDNS-Identitätsbildung gemäß offizieller Spezifikation prüfen. Weiterhin: keine Builds, keine GitHub Actions, kein Flashen und keine Änderungen an bestehenden Firmware-Repositories ohne ausdrückliche Freigabe.


## 18. Recherchefreigabe des Nutzers: externe Quellen selbständig nutzen

Der Nutzer hat ausdrücklich freigegeben, dass für ATS Mini OS alle erforderlichen Informationen selbständig recherchiert werden dürfen: öffentliche GitHub-Repositories, Quellcodes, Releases/Tags, Wikis, Dokumentationen, Standards, Hersteller-Datenblätter, Webseiten und andere geeignete öffentliche Informationsquellen. Die Recherche soll nicht auf das Referenz-Repository beschränkt sein.

Diese Freigabe gilt für Informationsbeschaffung und read-only Prüfung. Die bestehenden Projektgrenzen bleiben bestehen: `13HN2709/ats-mini-dev` und alle vorhandenen Firmware-Repositories bleiben unverändert; keine Builds starten, keine GitHub Actions auslösen, nichts flashen und keine ausführbare Firmware implementieren, solange der Nutzer das nicht ausdrücklich freigibt. Dokumentationsänderungen im neuen Repository `13HN2709/ATS_Mini_-OS` sind im bisherigen Planungsumfang zulässig.

## 19. Ergebnis: PU2CLR SI4735 2.1.8 und RadioDNS-Standards geprüft

Öffentliche Quellcodeprüfung am Tag `2.1.8` des offiziellen `pu2clr/SI4735`-Repositorys: `src/SI4735.h` und `src/SI4735.cpp` wurden gelesen. Die Release-Seite bestätigt, dass 2.1.8 eine veröffentlichte Bibliotheksversion ist. Es wurde nichts aus diesem Repository geändert.

### API-Funktionen in Version 2.1.8
- RDS-Status und FIFO: `getRdsStatus(...)`, `getRdsReceived()`, `getRdsSync()`, `getRdsSyncFound()`, `getRdsSyncLost()`, `getGroupLost()`, `getNumRdsFifoUsed()`.
- Dekodierte Felder: `getRdsPI()`, `getRdsGroupType()`, `getRdsVersionCode()`, `getRdsProgramType()`, `getRdsText0A()` (PS), `getRdsText2A()` und `getRdsText2B()` (RT), `getRdsTime()`.
- In den geprüften öffentlichen API-Deklarationen wurde kein fertiger Getter/Parser für ECC, AF, TP/TA oder RT+ gefunden. Die API stellt Status-/Gruppeninformationen bereit, sodass zusätzliche Rohgruppen-Auswertung grundsätzlich untersucht werden kann; die genaue Verfügbarkeit der nötigen Rohdaten im FIFO und die Decoderlogik sind noch nicht vollständig verifiziert.
- Daher RT+ nicht als fertige Bibliotheksfunktion annehmen. ECC, AF, TP/TA nicht als unmöglich einstufen; zuerst prüfen, ob die Rohgruppen mit den verfügbaren Status-/FIFO-Mechanismen korrekt entnommen werden können und welche Datenfelder der Chip tatsächlich liefert.

### RadioDNS-Kernanforderungen aus offizieller Dokumentation
- Offizielle Dokumentation: https://radiodns.org/technical/documentation/ und Core Lookup TS 103 270.
- FM-Lookup braucht Frequenz, PI und einen Global Country Code (GCC) bzw. die standardkonforme Länder-Fallback-Variante. GCC wird aus dem Country Code (erstes Nibble des RDS-PI) und ECC gebildet. ECC wird nicht zwingend von jeder Station ausgestrahlt; RadioDNS empfiehlt eine normgerechte GCC-Fallback-Strategie, z. B. über die offizielle GCC-Library/Geolocation, statt ECC blind zu erraten.
- Frequenzformat und DNS-Namensbildung müssen exakt der aktuellen TS 103 270 folgen. Alte RDNS01-Dokumente sind nur historischer Kontext; bei der Implementierung ist die aktuelle Spezifikation maßgeblich.
- Offizielle Referenzen: https://radiodns.org/technical/libraries/gcc-library/ ; https://radiodns.org/technical/documentation/ ; https://radiodns.org/technical/documentation/howto/ .

### Nächster Schritt
Die SI4735-2.1.8-RDS-FIFO-Implementierung und den offiziellen AN332-Datenpfad gezielt prüfen, um festzustellen, welche Blockdaten aus dem FIFO zugänglich sind und ob Gruppe 1A (ECC), Gruppe 0A/0B (TP/TA/AF) und Gruppe 3A (RT+) sauber verarbeitet werden können. Danach eine getrennte, quellenbelegte Datenfeldmatrix in `docs/NETWORK_DATA_SOURCES.md` ergänzen. Weiterhin nur Recherche/Dokumentation; keine Implementierung, Builds, Actions oder Flashvorgänge.


## 20. Ergebnis: RDS-Rohblockzugriff in PU2CLR SI4735 2.1.8 genauer geprüft

Am 1. November 2026 wurden die offizielle Bibliotheksversion pu2clr/SI4735 Tag 2.1.8 und Silicon Labs AN332 read-only geprüft. Details und Datenfeldmatrix: [docs/NETWORK_DATA_SOURCES.md, Abschnitt 11](docs/NETWORK_DATA_SOURCES.md#11-vertiefte-prüfung-si4735-rds-fifo-und-rohblöcke-2026-11-01).

### Gesicherte Ergebnisse
- Die SI4735-Statusantwort hat 13 Bytes und enthält Status/FIFO-Zähler, die 16-Bit-RDS-Blöcke A, B, C und D sowie Fehlerindikatoren BLEA–BLED.
- getRdsStatus(...) kopiert die vollständige Antwort in currentRdsStatus.raw[]. Das Rohmaterial existiert also im internen Statusobjekt.
- Das Objekt ist nicht als öffentlicher Rohblock-Getter verfügbar. Für ATS Mini OS muss ein sauberer Zugriffspfad vorgesehen werden; externe Module dürfen nicht einfach als zugriffsberechtigt angenommen werden.
- STATUSONLY=0 liefert den ältesten FIFO-Gruppeneintrag, wenn die FIFO nicht leer ist; STATUSONLY=1 dient dem letzten gültigen Status für A/B und ist nicht für vollständige Gruppenauswertung A–D geeignet.
- Ein zusätzlicher unabhängiger Aufruf von getRdsStatus() könnte die FIFO leeren/weiterdrehen und den vorhandenen RDS-Parser beeinflussen. Bei späterer Implementierung muss genau ein koordinierter FIFO-Leser die Daten an PS/RT/PTY/ECC/AF/TP/TA/RT+ verteilen.
- Die Standardbibliotheksfunktion getRdsPI() liefert in Version 2.1.8 laut Quellcode nur BLOCKAL; im ATS-Referenzprojekt existiert ein Wrapper, der High- und Low-Byte kombiniert. Diese Korrektur nicht verlieren.

### Folgerung
Die SI4735-Antwort stellt Rohfelder bereit, um die Dekodierung von ECC, AF, TP/TA und RT+ weiterzuverfolgen. Das bedeutet noch nicht, dass diese Felder im bestehenden ATS-Anwendungscode verfügbar oder korrekt dekodiert sind. Gruppenbitbelegung, Fehlerprüfung und ODA-Zuordnung für RT+ müssen vor einer Implementierung normativ geprüft werden.

### Arbeitsgrenzen bleiben unverändert
Nur öffentliche Quellen und Dokumentation. 13HN2709/ats-mini-dev und bestehende Firmware-Repositories bleiben unverändert; keine Firmwareimplementierung, Builds, GitHub Actions oder Flashvorgänge. Slot A und DiamondRadio in Slot B bleiben unangetastet.


## 21. RadioDNS-Logo-Datenfluss und Diagnoseanforderungen dokumentiert

Am 1. November 2026 wurde `docs/RADIODNS_LOGO_DATA_REQUIREMENTS.md` neu angelegt. Grundlage sind die aktuelle RadioDNS-Dokumentation zu Project Logo, Core Lookup/GCC sowie die vorherige read-only Prüfung von PU2CLR SI4735 2.1.8. Die Datei beschreibt die notwendigen Empfangsdaten, Identitätsnormalisierung, DNS-/SI-Auflösung, Logo-Auswahl, Cache-/Sicherheitsgrenzen und Diagnosezustände.

### Wesentliche Festlegungen
- Für den FM-Lookup sind die tatsächliche Frequenz, der vollständige 16-Bit-PI und der GCC entscheidend. PS/PTY/RT/RT+/AF/TP/TA/CT bleiben nützliche Zusatz- und Diagnosefelder, ersetzen aber die Identität nicht.
- Eine feste EU-RDS-Region ist keine RadioDNS-Länderkennung. ECC hat Vorrang, sofern valide empfangen; andernfalls muss die RadioDNS-GCC-Fallback-Strategie genutzt werden, ohne das Senderland blind aus Gerätestandort oder Region abzuleiten.
- Der Diagnoseablauf soll RDS-Gültigkeit, PI/ECC/GCC, normalisierte Frequenz, FQDN, DNS-CNAME, SRV, SI-Datei, Bearer-Match, Logo-Metadaten, Downloadprüfung und Cache-Status getrennt sichtbar machen.
- SI-Dateien enthalten Service-/Bearer-Zuordnung und Logo-Metadaten. Das Logo wird nur für den exakt passenden FM-Bearer ausgewählt; URLs, MIME-Typ, Abmessungen, Dateigröße, Nutzungsbedingungen und Cache-Regeln müssen geprüft werden.
- Wegen der SI4735-RDS-FIFO-Semantik bleibt ein einziger koordinierter FIFO-Leser erforderlich; das RadioDNS-Modul darf den Tuner-FIFO nicht separat konsumieren.

### GitHub-Änderung
- Neue Datei: [docs/RADIODNS_LOGO_DATA_REQUIREMENTS.md](docs/RADIODNS_LOGO_DATA_REQUIREMENTS.md)
- Dokumentationscommit: `b9b8f42353b104df8b31e69c89a50f80de4392eb`
- Keine Firmwareimplementierung, Builds, GitHub Actions oder Flashvorgänge ausgeführt.


## 22. RadioDNS-Schnittstellen und Font für das Testlayout dokumentiert

Am 1. November 2026 wurde `docs/RADIODNS_INTERFACES_AND_TEST_LAYOUT.md` ergänzt. Es enthält den erneuten Abgleich der aktuellen RadioDNS-Standards, vorgeschlagene fachliche Verträge, Zuständigkeiten, Statusübergänge und das Layoutkonzept. Keine dieser Schnittstellen ist bereits als C++-API implementiert.

### Protokollpunkte
- Core Lookup: ETSI TS 103 270 v1.4.1; SI/SPI: TS 102 818 v3.5.1.
- FM-Identität: fünfstellige Frequenz, vollständiger vierstelliger PI und dreistelliger GCC; Beispielwerte, ServiceIdentifier und Bearer sind in der neuen Spezifikation dokumentiert.
- ECC bleibt optional im Broadcast; fehlendes ECC erfordert den RadioDNS-konformen GCC-Fallback, nicht die pauschale Annahme „EU“ oder Deutschland.
- SI-Discovery folgt DNS-CNAME → SRV `_radioepg._tcp` → `/radiodns/spi/3.1/SI.xml` → exakter FM-Bearer → Logo-Metadaten. Die offizielle Project-Logo-Anleitung beschreibt den SI-Endpunkt über HTTP; HTTPS darf nicht als von jedem Sender garantiert vorausgesetzt werden.
- Die RDS-FIFO bleibt bei einem koordinierten Leser. RadioDNS und UI konsumieren nur den Snapshot; Sendergeneration verhindert, dass Ergebnisse eines alten Senders übernommen werden.

### Testlayout und Schrift
- Frequenz groß oben links; Encoder-Tuning läuft ausschließlich über den Radio Core.
- Diagnosefelder für PI/ECC/GCC, FQDN, DNS/SRV, SI, Bearer-Match, Logo-URL/Format/Abmessungen und Cache-/Fehlerstatus.
- **Oxanium** als digitale Schrift für lateinische Diagnosetexte vorgemerkt. Google-Fonts-Metadaten weisen `latin` und `latin-ext` sowie OFL aus; deutsche Umlaute/ß und italienische Akzente müssen später an der konkreten Datei geprüft werden.
- **Noto Sans Arabic** für einen separaten späteren Test vorgemerkt. Korrekte arabische Darstellung erfordert zusätzlich geprüfte RTL-/Shaping-Unterstützung; eine Fontdatei allein genügt nicht.

### GitHub
- Neue Datei: [docs/RADIODNS_INTERFACES_AND_TEST_LAYOUT.md](docs/RADIODNS_INTERFACES_AND_TEST_LAYOUT.md)
- Commit der neuen Datei: `332f47c9a65eece2b32619512dc61059503e9567`
- Schnittstellen, Fonts und Testlayout bleiben Planung. Keine Firmwareimplementierung, Builds, Actions oder Flashvorgänge.


## 23. Vorbereitung des ersten Builds – Arbeitszweig

- Nutzer hat freigegeben, die notwendigen Vorbereitungen bis unmittelbar vor den ersten Build auszuführen.
- Arbeitszweig angelegt: `dev/first-build-preparation` (aus `main`).
- Das Referenzrepository `13HN2709/ats-mini-dev` wird ausschließlich lesend untersucht. Aus `Common.h` und `sketch.yaml` wurden vorläufige Hinweise zu Displaygröße (320×170), SI473x, Pinbelegung und Arduino-ESP32-/Bibliotheksversionen gesammelt.
- Diese Werte sind noch nicht als verifizierte Hardwarefakten oder finaler OS-Buildvertrag zu behandeln.
- Schriftkandidat für das Testlayout: DSEGstended 14-Segment Regular; Quelle und SIL OFL 1.1 sind dokumentiert. Fontdatei und tatsächliche Glyphen-/Speicherprüfung stehen noch aus.
- Nächster Weg: Referenzdisplay-/Tuner-Initialisierung und Lizenz/Pin-Konfiguration vollständig nachvollziehen; dann minimale, klar getrennte Radio-Core- und UI-Grundlage auf dem Arbeitszweig anlegen und statisch prüfen.
- **Noch nicht durchgeführt:** Build, GitHub Actions, Flashen oder Änderungen an bestehenden Firmware-Repositories. Slot A und DiamondRadio Slot B bleiben geschützt.


- Ergänzung: `docs/FIRST_BUILD_READINESS.md` bündelt die Quellcode-Erkenntnisse, offene Build-Blocker und den sicheren Ablauf bis zum ersten Compile.


## 24. Erstes Firmware-Scaffold (2026-11-01)

Branch `dev/first-build-preparation` enthält jetzt:
- `firmware/ats-mini-os/ats-mini-os.ino`: Arduino-ESP32 bring-up sketch mit serieller Statusausgabe.
- `firmware/ats-mini-os/system/PerformanceManager.h/.cpp`: vier Profile (Auto, FixedLow, FixedMedium, FixedHigh), temporäre Leistungs-Leases und Timeout-Entlastung. Noch keine tatsächlichen CPU-Taktänderungen; Frequenzpunkte/API müssen vor Aktivierung verifiziert werden.
- `firmware/ats-mini-os/sketch.yaml`: OSPI-/QSPI-Profile als Ausgangspunkt aus `ats-mini-dev`.
- `firmware/ats-mini-os/partitions.csv`: Kopie des 8-MB-Referenzlayouts aus `ats-mini-dev`, ausdrücklich nicht als endgültige Flash-/Dual-Boot-Tabelle freigegeben.
- `firmware/ats-mini-os/README.md`: Grenzen und noch fehlende Komponenten.

Dies ist nur ein Compile-/Bring-up-Scaffold. Es implementiert noch keinen nutzbaren UKW-Empfänger, kein Display, keine SI4735-Initialisierung, kein RDS/RadioDNS und keine Menüauswahl für CPU-Profile. Der tatsächliche CPU-Takt wird nicht umgestellt.

**Build-Status:** Kein lokaler Build ausgeführt, kein GitHub Actions Workflow gestartet, nichts geflasht. Vor dem ersten Build muss die Arduino-CLI-Profilauflösung und die Behandlung von `PartitionScheme=custom` mit der bereitgestellten `partitions.csv` abgesichert werden. Ein erfolgreicher Scaffold-Compile wäre nur ein Toolchain-/Profiltest, kein Nachweis der finalen Firmware- oder Dual-Boot-Funktion.


## 25. Dauerhafte Regel des Nutzers: strikt modulare Entwicklung

Der Nutzer hat ausdrücklich festgelegt, dass diese Regel **immer** gelten muss und in Übergaben bei vollen Chats festgehalten werden soll:

- Jede fachlich eigenständige Funktion erhält eigene `.h`- und `.cpp`-Dateien.
- `.ino` bleibt möglichst klein und enthält nur Einstiegspunkt sowie Setup-/Loop-Verdrahtung.
- Eine Änderung an einer Funktion darf unabhängige Funktionen nicht kaputtmachen. Keine Sammeldateien, keine direkten Eingriffe in fremde Modul-Interna und keine versteckten Seiteneffekte.
- Module interagieren über klar definierte Schnittstellen; gemeinsame Ressourcen haben genau einen Besitzer.
- Vor Änderungen Abhängigkeiten/Aufrufer prüfen, betroffene Module gezielt anpassen und Prüfungen dokumentieren.
- Bei jeder Übergabe Branch, Commits, Fortschritt, offene Punkte und Build-/Flash-Status ergänzen.

Diese Regel ist verbindlich für alle weiteren Arbeiten am ATS Mini OS.

## 26. Vorbereitung des ersten Compile-Workflows

- Manuell startbarer Workflow angelegt: `.github/workflows/ats-mini-os-first-compile.yml`.
- Workflow hat nur `workflow_dispatch`; als zusätzliche Schutzmaßnahme muss der Eingabewert `COMPILE` sein. Er baut getrennte Matrix-Jobs für `esp32s3-ospi` und `esp32s3-qspi`.
- Der Workflow führt nur Compile-/Report-Schritte aus; kein Flash-Schritt und kein Upload auf ein Gerät.
- **Workflow wurde nicht gestartet.** Es gab keinen Build, keinen Flashvorgang und keine Änderung an installierten Boot-Slots.
- Firmwaremodule liegen unter `firmware/ats-mini-os/src/`; die `.ino`-Datei ist nur Einstiegspunkt. Module bekommen eigene `.h`-/`.cpp`-Dateien.
- Vor dem Start sind im Review insbesondere Arduino-CLI-Profilauflösung, `sketch.yaml`, `PartitionScheme=custom` und der nur vorläufige 8-MB-Partitions-Referenzstand zu beachten. Ein Compile-Test darf nicht als Freigabe zum Flashen verstanden werden.



CPU-Profilbezeichnungen im Code wurden an die bestätigte Menüvorgabe angepasst: `Auto`, `Taktstufe 1`, `Taktstufe 2`, `Taktstufe 3`. Die tatsächlichen MHz-Werte und die dynamische Taktumschaltung bleiben bis zur technischen Prüfung deaktiviert.


## 28. CPU-Taktvorgabe anhand DiamondRadio-Foto bestätigt (2026-11-01)

Der Nutzer hat ein Foto des DiamondRadio-Menüs `CPU CLOCK` bereitgestellt. Sichtbar sind die drei Stufen **80 MHz / 160 MHz / 240 MHz**. Damit ist die feste Taktstufen-Zuordnung für ATS Mini OS bestätigt:
- `Auto`: zentral verwaltete dynamische Leistungsregelung.
- `Taktstufe 1`: 80 MHz.
- `Taktstufe 2`: 160 MHz.
- `Taktstufe 3`: 240 MHz.

Espressif dokumentiert für den ESP32-S3 dieselben CPU-Frequenzpunkte für Dynamic Frequency Scaling: https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32s3/api-reference/system/power_management.html. Die Arduino-ESP32-CPU-HAL stellt `setCpuFrequencyMhz()`, `getSupportedCpuFrequencyMhz()` und `getCpuFrequencyMhz()` bereit: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-cpu.h. Die konkrete Laufzeitumschaltung bleibt deaktiviert, bis Verhalten und Timing mit der tatsächlich eingesetzten Arduino-ESP32-Version sowie OSPI/QSPI geprüft wurden.

Im Modus `Auto` dürfen Waterfall, RadioDNS/WLAN und Bilddekodierung zeitlich begrenzte Anforderungen über den Performance Manager stellen. Parallele Aufgaben dürfen kein vorzeitiges Heruntertakten verursachen. Feste Stufen werden niemals von solchen Anforderungen überschrieben. Der alte Hinweis „Encoderaktivität schaltet auf 240 MHz, nach zehn Sekunden Inaktivität auf 80 MHz“ ist noch nicht am DiamondRadio-Originalquellcode verifiziert.

Außerdem wurde in `PerformanceManager.cpp` ein irreführender Kommentar korrigiert: Anfragen in festen Modi werden akzeptiert, erzeugen aber absichtlich keine Lease und verändern den festen Takt nicht. Änderung: Commit `2ac60fc4f203a837ca45bf4d9ba3abcba3e8a6aa`. CPU-Recherche in Readiness-Dokument: Commit `93e02023a1f90d53b95a8d9a1008b48f13c8ed82`.


## 29. Erster Compile-Lauf fehlgeschlagen – Toolchain-Fehler

GitHub Actions Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41263944157 (2026-11-01). Beide Jobs (`Compile-only / esp32s3-ospi` und `Compile-only / esp32s3-qspi`) endeten mit `failure`. Die Logs zeigen Download/Installation von ESP32-3.3.12-Paketbestandteilen und danach `Error during build: Platform 'arduino:avr' not found: platform not installed`. Dies ist zunächst ein Fehler bei Plattformauflösung/Toolchain-Setup, kein bestätigter C++-Fehler im OS-Scaffold. Die Workflow-/CLI-Konfiguration muss vor dem nächsten Lauf korrigiert werden. Es wurde nichts auf das ATS Mini geflasht und kein Boot-Slot verändert.


## 30. Korrektur des Compile-Workflows nach Run 41263944157 (2026-11-01)

Fehlerursache aus den Logs: `arduino/compile-sketches@v1` scheiterte beim Core-Setup mit `Platform 'arduino:avr' not found: platform not installed`, obwohl ESP32-Pakete heruntergeladen wurden. Im Branch `dev/first-build-preparation` wurde der Workflow auf einen expliziten Arduino-CLI-Ablauf umgestellt: CLI installieren, Espressif-Paketindex registrieren, `esp32:esp32@3.3.12` installieren, ESP32-S3-Board prüfen, danach getrennt OSPI (`PSRAM=opi`) und QSPI (`PSRAM=enabled`) kompilieren. Der Shell-Aufruf wurde auf korrekte Zeilenfortsetzungen geprüft. Korrektur-Commits: `47376eacdfaa3608e66673f5e8ea5aec23175ec5`, `5d0f90e1f0f1259ce72e3fb0b2b78e92bf6007d3`, `7c2db0cd7e5c7d914643e6e10b41c5af8119dd9e`. Noch kein neuer Lauf gestartet; der neue Workflow ist nicht praktisch validiert. Nächster Schritt: Nutzer startet manuell mit Eingabe `COMPILE`. Keine Flash-Aktion.


## 31. Zweiter Compile-Lauf: Runner-Speicher erschöpft (Run 41264295954, 2026-11-01)

Die explizite Arduino-CLI-Konfiguration funktioniert nun: Installation von CLI und ESP32-Core-Index sowie `arduino-cli board details -b esp32:esp32:esp32s3` waren erfolgreich. Beide Compile-Jobs scheiterten danach während der Core-Installation, weil der Runner-Speicher voll wurde: `no space left on device`; das Log meldete nur 47 MB frei. Dadurch fehlten u. a. ESP32-S3-Bootloader-Dateien. Das ist ein Runner-Speicherproblem, kein bestätigter Quellcodefehler. Im Workflow wurde ein Schritt ergänzt, der vor der Core-Installation nicht benötigte vorinstallierte SDKs (Android, .NET, GHC, Boost, Swift) auf dem kurzlebigen GitHub-Runner entfernt und den freien Speicher protokolliert. Änderung Commit `4a46f8032b8ca553725c3a3ef697fd1a6421c8ba`. Noch nicht erneut ausgeführt; Nutzer muss den manuellen Compile-Workflow erneut starten. Kein Flashen.


## 32. IST-Zustand ATS Mini V4 – Partitionen und Backup bestätigt (2026-11-01)

**Diese Angaben sind der verbindliche IST-Referenzstand.** Sie stammen aus den vom Nutzer bereitgestellten Fotos der Partition-Ansicht des aktuell laufenden Firmware-/Recovery-Systems. Es handelt sich ausdrücklich **nicht** um eine Partitionstabelle des Diamond-Bootloaders.

### Hardware und Sicherheitszustand
- Gerät: **AMNVOLT ATS Mini V4**, Flashgröße **16 MB**.
- Der Nutzer hat sein zuvor erstelltes Backup über die YdiamondRadio-Webseite zurückgespielt; der Rückweg verlief erfolgreich.
- Zuvor wurde ein vollständiges Backup `ATSMiniFULLBackup.nbin` mit 16,00 MB erwähnt.
- DBFM v4.2.2 Recovery-only-Update war zuvor auf Adresse `0x860000` erfolgreich, wobei App-Slots erhalten blieben.
- Der Nutzer möchte die bestehende Boot-/Recovery-Konfiguration bewahren. Keine Partitionierung ändern und nichts flashen, solange es nicht ausdrücklich freigegeben ist.

### Partitionstabelle aus den aktuellen Fotos

| Partition | Startadresse | Angezeigte Größe |
|---|---:|---:|
| `nvs` | `0x009000` | 20 KB |
| `otadata` | `0x00E000` | 8 KB |
| `app0` | `0x010000` | 3584 KB |
| `app1` | `0x390000` | 3584 KB |
| `ffat` | `0x710000` | 1344 KB |
| `recovery` | `0x860000` | 1664 KB |
| `littlefs` | `0xA00000` | 5716 KB |
| `rec_settings` | `0xF95000` | 32 KB |
| `settings` | `0xF9D000` | 128 KB |
| `coredump` | `0xFDF000` | 128 KB |

### Angezeigte Nutzung (aus dem zuvor dokumentierten Partition-Info-Bild)
- `app0`: ca. 1,8 MB belegt / 1,7 MB frei (52 %).
- `app1`: ca. 735 KB belegt / 2,8 MB frei (20 %).
- `recovery`: ca. 1,2 MB belegt / 416 KB frei.
- `littlefs`: ca. 3 MB belegt / 2,5 MB frei.

**Wichtige Interpretation:** `app0` und `app1` sind jeweils ungefähr 3,5 MiB groß; Recovery und LittleFS liegen in fest belegten Bereichen. Die bisher im neuen OS verwendete 8-MB-Partition-CSV ist nur eine Compile-Referenz und ist **keine** freigegebene Partitionstabelle für dieses 16-MB-Gerät. Sie darf nicht auf das Gerät geflasht werden. Bei künftigen Arbeiten zuerst diese IST-Daten und die tatsächliche Boot-/Recovery-Logik prüfen.


## 33. Erster erfolgreicher Compile-Lauf – OSPI und QSPI (2026-11-01)

Der Nutzer bestätigte mit Screenshot den erfolgreichen manuellen Workflow-Lauf **ATS Mini OS - First Compile (manual only) #5** auf Branch `dev/first-build-preparation`, Commit `f8d80de`. GitHub Actions zeigt Status `Success`, Gesamtdauer 4m 45s und zwei Artefakte:
- `ats-mini-os-compile-report-esp32s3-ospi` (6.78 KB)
- `ats-mini-os-compile-report-esp32s3-qspi` (6.78 KB)

Beide Matrix-Jobs `Compile-only / esp32s3-ospi` und `Compile-only / esp32s3-qspi` sind erfolgreich abgeschlossen. GitHub zeigt pro Job eine Node.js 20 Deprecation-Warnung für eine Action; das sind Warnungen, keine Build-Fehler. Status ist **Compile erfolgreich**, aber es handelt sich weiterhin um den aktuellen modularen Scaffold und nicht um eine vollständige Radio-Firmware. Es wurde nichts geflasht; Partitionen, Boot-Slots, Recovery und bestehende Firmware sind unangetastet. Der nächste Schritt ist, die Artefakte/Build-Ausgabe zu prüfen und anschließend die tatsächliche Firmware modular zu implementieren, beginnend mit minimal stabilem FM-Radio. Nicht flashen ohne ausdrückliche Freigabe.


## 34. Erste modulare FM-Radioimplementierung (2026-11-01; Compile ausstehend)

Nach dem erfolgreichen Scaffold-Compile wurden die ersten Funktionsmodule angelegt:
- `src/tuner/TunerDriver.h/.cpp`: einziger Besitzer des SI4735-Tunerobjekts und direkter Tunerzugriffe.
- `src/radio/RadioCore.h/.cpp`: FM-Tuning, 100-kHz-Schritt, RadioSnapshot und periodisches Lesen von Frequenz/RSSI/SNR/Stereo.
- `.ino` bleibt dünner Einstiegspunkt: Start auf 103,9 MHz und serielle Diagnoseausgabe alle 2 Sekunden.
- Workflow installiert nun `PU2CLR SI4735` als Abhängigkeit.

Verwendete Referenz-Pins: LDO GPIO15, Reset GPIO16, SDA GPIO18, SCL GPIO17. Diese Zuordnung basiert auf dem bestehenden ATS Mini V4 Referenzprojekt und ist im neuen OS noch nicht auf Hardware getestet. Display, Encoder, Audio-/RDS-UI sind noch nicht integriert. **Compile für diesen neuen Code steht noch aus; kein Workflow gestartet, kein Flash.** Nächster Schritt: manuellen Compile-only-Workflow ausführen und die tatsächlichen Compilerlogs auswerten. Erst danach Hardwaretest planen.


## 35. FM-Testfrequenz gesetzt: Antenne Münster 95,4 MHz (2026-11-01)

Der Standardstart der ersten FM-Diagnose wurde auf **95,4 MHz (95400 kHz)** gesetzt, auf Wunsch des Nutzers als Testsender Antenne Münster. Änderung in `firmware/ats-mini-os/ats-mini-os.ino`, Commit `dfc44dd016b88a84fd340be60d6ac3353f4efcc6`. Der Compile-Workflow für diesen Stand ist noch nicht erneut gestartet. Dies ist eine Test-Startfrequenz, kein Nachweis von Empfang oder RDS auf dem Gerät. Kein Flash.


## 36. Slot-Aufteilung für Tests verbindlich bestätigt (2026-11-01)

Der Nutzer bestätigt den aktuellen Gerätezustand: Die gut funktionierende aktuelle Dev-Firmware läuft auf **APP0**; neue OS-Tests sollen für **APP1** vorgesehen werden.
- APP0: Start `0x010000`, Größe 3584 KB. Stabiler Betrieb – nicht überschreiben oder verändern.
- APP1: Start `0x390000`, Größe 3584 KB. Vorgesehener Testslot für neues ATS Mini OS, aber noch kein Flash freigegeben.
- Vollständiges Backup `ATSMiniFULLBackup.nbin` (16 MB) und Recovery erhalten.
- Die bisherige OS-Konfiguration/Partition-CSV ist nur für Compile-Tests und darf nicht als Flash-/Upload-Konfiguration verwendet werden. Vor einem ersten Flash müssen Binärformat, Flash-Offsets, Bootloader-/OTA-Mechanismus und 3.5-MiB-Slotgröße verifiziert werden. Keine Firmware auf Offset `0x0`, kein Clean Install, keine Änderung an Bootmanager/Recovery.
- Der Nutzer hat APP1 als Testplatz vorgesehen; dies ist noch keine ausdrückliche Freigabe, jetzt zu flashen. Bis zur gesonderten Freigabe nur Compile-/Artefaktprüfung.


## 37. Selbstständige Prüfung und persistente Einstellungen (2026-11-01; Compile ausstehend)

Nutzerwunsch: Firmware soll beim Start relevante Zustände selbst prüfen und Einstellungen unabhängig vom App-Slot zuverlässig speichern, statt dass Frequenzen je nach Firmware/Slot verloren gehen.

Umgesetzt als erster Schritt:
- Neues isoliertes Modul `src/settings/SettingsStore.h/.cpp` auf Basis von ESP32 NVS/Preferences mit eigener Namespace `atsmini_os`.
- Letzte FM-Frequenz wird beim Start geladen und auf den gültigen FM-Bereich 87,5–108,0 MHz geprüft; ungültige Werte fallen auf die Standardfrequenz zurück.
- Nach einer Abstimmung wartet die Firmware drei Sekunden ohne weitere Frequenzänderung, bevor sie speichert (Flash-Verschleiß reduzieren).
- Falls NVS nicht geöffnet werden kann, bleibt der Radio-Start mit Standardfrequenz möglich.
- `RadioCore` nutzt das Modul. Die erste Standardfrequenz bleibt Antenne Münster 95,4 MHz; eine gültig gespeicherte Frequenz hat beim Neustart Vorrang.

Technische Einschränkung: NVS ist persistent und vermeidet die Abhängigkeit von LittleFS, aber Slots, die dieselbe Firmware und denselben NVS-Namespace nutzen, würden dieselben Werte sehen. Unterschiedliche Firmware-Projekte müssen eigene Namespaces verwenden; echte Slot-Trennung kann bei Bedarf explizit anhand einer bestätigten Slot-ID ergänzt werden. Weitere Einstellungen wie CPU-Profil, Lautstärke und Betriebsart sind noch nicht gespeichert. **Noch kein Compile dieses neuen Codes, kein Hardwaretest, kein Flash.**


## 38. Compile-Lauf 41266238739 – SI4735-Header fehlt (2026-11-01)

Job `124729519289` (OSPI) scheiterte im Compile mit `fatal error: SI4735.h: No such file or directory`. Vorherige Schritte einschließlich Runner-Cleanup, ESP32-Core-Installation, SI4735-Library-Install-Step und Board-Erkennung liefen formal erfolgreich; das Library-Installieren per Name `arduino-cli lib install "PU2CLR SI4735"` hat jedoch nicht zu einem auffindbaren Header geführt. QSPI-Job `124729519108` war beim Auslesen noch `in_progress`, daher kein Endergebnis behaupten. Offizielle PU2CLR-Repository-Anleitung dokumentiert CLI-Installation per `arduino-cli lib install --git-url https://github.com/pu2clr/SI4735` mit `ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true`. Workflow-Commit `7f917516312a7d61d1b7cbbe2cdb9a9d5d6b9ca1` ersetzt die Bibliotheksinstallation durch diesen Upstream-Git-Weg und ergänzt einen Header-Existenzcheck, damit fehlende Abhängigkeiten vor dem Compile klar scheitern. Noch kein erneuter Lauf gestartet. Kein Flash.


## 39. Manueller Compile-Workflow ohne Texteingabe (2026-11-01)

Auf Wunsch des Nutzers wurde die Pflicht-Eingabe `COMPILE` aus `.github/workflows/ats-mini-os-first-compile.yml` entfernt (Commit `2eb7767fbacb8ddc5729a5e4e423799c4dacf8cb`). Der Workflow bleibt ausschließlich manuell über `workflow_dispatch` startbar; die bedingte Job-Abfrage auf `inputs.confirm_compile == 'COMPILE'` wurde entfernt. Beim Start muss nun kein Bestätigungswort mehr eingetragen werden. Der Workflow führt weiterhin nur die beiden Compile-Jobs für OSPI/QSPI aus und enthält keinen Flash-Schritt. APP0, APP1, Bootloader, Recovery und Partitionen wurden nicht verändert. Die neue Workflow-Version wurde noch nicht gestartet; der Nutzer startet Builds weiterhin selbst.


## 40. Compile-Lauf 41266579591 – SI4735 weiterhin nicht im Compiler-Suchpfad (2026-11-01)

Beide Jobs (OSPI `124730525213` und QSPI `124730525405`) scheiterten beim Compile weiterhin mit `fatal error: SI4735.h: No such file or directory`. Die Library-Installation und der Prüfschritt liefen erfolgreich; `arduino-cli lib list` zeigt `PU2CLR SI4735 2.1.8`. Damit ist der Fehler nicht mehr einfach eine fehlgeschlagene Installation, sondern wahrscheinlich die Bibliotheksauflösung beim Compile.

Workflow angepasst in Commit `bbdd150de4d749034b81e6f8624c433d94b88c57`:
- Header-Prüfung gibt jetzt den tatsächlich gefundenen vollständigen Pfad aus.
- Compile bekommt den Library-Ordner explizit über `--libraries "$HOME/Arduino/libraries"`.
- Workflow wurde nicht gestartet; der Nutzer startet den nächsten Compile manuell. Kein Flash, keine Hardware-/Partitionsänderung.


## 41. Compile-Lauf 41266976434 – Bibliothek explizit an Compiler übergeben (2026-11-01)

OSPI-Job `124731702901` und QSPI-Job `124731703117` schlagen weiterhin mit `fatal error: SI4735.h: No such file or directory` fehl. Logs bestätigen, dass die Datei vorhanden ist unter `$HOME/Arduino/libraries/PU2CLR_SI4735/src/SI4735.h`; auch `--libraries "$HOME/Arduino/libraries"` hat die Abhängigkeit nicht aufgelöst.

Nächste Workflow-Korrektur in Commit `63e3a85b5973ac5bb137ac11c3848756cbd6e3df`: statt der Sammlung mehrerer Libraries wird der konkrete Library-Root über `--library "$HOME/Arduino/libraries/PU2CLR_SI4735"` direkt an Arduino CLI übergeben. Grundlage ist die Arduino-CLI-Dokumentation zur expliziten Auswahl einer einzelnen Library. Noch kein neuer Workflow gestartet; der Nutzer startet manuell. Kein Flash und keine Geräteänderung.


## Build-Fehlerstand – 2026-11-01 (Run 41267328096)

- Der manuell gestartete Compile-only-Lauf ist in beiden Profilen fehlgeschlagen: [QSPI-Job](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41267328096/job/124732757205) und [OSPI-Job](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41267328096/job/124732757271).
- Beide Jobs finden die installierte Datei laut vorgeschaltetem Header-Check, scheitern aber beim Übersetzen von `TunerDriver.h` weiterhin mit `fatal error: SI4735.h: No such file or directory`.
- Der direkte `--library`-Pfad allein hat das Problem nicht behoben. Neuer Workflow-Commit `c78e8488729f3f83ce4aa6ee303312d5802cbbe6` ergänzt zusätzlich den expliziten Include-Pfad `.../PU2CLR_SI4735/src` als `compiler.cpp.extra_flags`. Das ist eine weitere Build-Konfigurationskorrektur, noch nicht verifiziert.
- **Nächster Schritt:** Nutzer startet den Workflow erneut manuell. Keine Actions selbst starten. Falls der Header-Fehler verschwindet, die dann sichtbaren Compilerfehler als nächsten Schritt bearbeiten.
- Keine Firmware wurde geflasht; keine Partitionen, Bootloader oder Geräte-Slots wurden verändert.


## Build-Fehlerstand – 2026-11-01 (Run 41267717924)

- Der Header-Include-Fehler ist behoben: Beide Profile finden `SI4735.h` und kompilieren nun weiter bis zur Tuner-Implementierung.
- Beide Jobs melden dieselben zwei API-Fehler in `src/tuner/TunerDriver.cpp`: `SI4735_FM` ist kein definierter Bezeichner und `getCurrentStereo()` existiert nicht in PU2CLR SI4735 2.1.8.
- Korrektur in Commit `c021e99bf9c73c6107fa71a5fa0593b2872f0687`: Initialisierung verwendet jetzt `FM_FUNCTION`; Stereo-/Pilotstatus wird über die in der Bibliothek vorhandene Methode `getCurrentPilot()` abgefragt. Referenz: PU2CLR-SI4735-API-Dokumentation und `src/SI4735.h`.
- **Nächster Schritt:** Nutzer startet den Compile-only-Workflow erneut manuell. Diese Korrektur ist noch nicht durch einen neuen Lauf verifiziert.
- Keine Firmware geflasht; keine Partitionen, Bootloader oder Geräteslots verändert.


## Build-Fehlerstand – 2026-11-01 (Run 41268473208)

- Beide Compile-only-Jobs sind fehlgeschlagen: OSPI Job `124736141818`, QSPI Job `124736141968`.
- Die SI4735-Bibliothek wird gefunden; der einzige bestätigte Compilerfehler in beiden Logs ist `'FM_FUNCTION' was not declared in this scope` in `src/tuner/TunerDriver.cpp`.
- Ursache: Der in den offiziellen PU2CLR-Beispielen dokumentierte Symbolname ist in der konkret installierten Bibliotheks-/Compile-Konfiguration nicht verfügbar. Die API-Dokumentation beschreibt `setup(resetPin, interruptPin, defaultFunction)`, wobei `defaultFunction = 0` FM und `1` AM bedeutet.
- Korrektur in Commit `bde7559eaab5270c84d4133e978caa10892b5e17`: Der Treiber verwendet nun den lokal dokumentierten `constexpr uint8_t kFmFunction = 0` statt des nicht aufgelösten Symbols. `getCurrentPilot()` bleibt für die Stereo-Pilot-Abfrage erhalten.
- **Nächster Schritt:** Nutzer startet den manuellen Compile-only-Workflow erneut; der Fix ist noch nicht durch einen neuen Build verifiziert.
- Kein Workflow von uns gestartet, keine Firmware geflasht und keine Partitionen, Bootloader oder Geräteslots verändert.

## Build-Fehlerstand – 2026-11-01 (Run 41270376115)

- Beide Compile-only-Jobs (OSPI und QSPI) erreichten den Linker, scheiterten dort aber mit `undefined reference` auf SI4735-Konstruktor und -Methoden (`SI4735()`, `setup`, `setFM`, `setVolume`).
- Die installierte Bibliothek PU2CLR SI4735 2.1.8 enthält `library.properties` sowie `src/SI4735.h` und `src/SI4735.cpp`; die Header-Prüfung allein hatte zuvor nicht bestätigt, dass die Implementierungsdatei Teil der Bibliothekserkennung ist.
- Workflowkorrektur in Commit `7885d2eaca4a35a1636af70ccb4dfd8bebfc6d3a`: statt einer expliziten Einzelbibliotheks-/Header-Include-Überschreibung wird jetzt das Bibliotheksverzeichnis über `--libraries "$HOME/Arduino/libraries"` eingebunden. Die Vorprüfung verlangt zusätzlich Metadaten, Header und `SI4735.cpp`.
- Die offizielle Arduino-CLI-Dokumentation beschreibt `--libraries` als Verzeichnis-Sammlung für Bibliotheken und bestätigt, dass die `src`-Quellen einer korrekt strukturierten Library kompiliert und gelinkt werden. Der Fix ist noch nicht durch einen neuen Lauf verifiziert.
- Nächster Schritt: Nutzer startet den manuellen Compile-only-Workflow. Wir starten keinen Workflow selbst.
- Kein Flashen, keine Hardwareänderungen und keine Änderungen an Bootloader, Partitionen, APP0 oder APP1.
## Build-Fehlerstand – 2026-11-01 (Run 41270987961)

- OSPI-Job `124743546130` schlug beim Kompilieren fehl; QSPI-Job `124743546262` lief bei der Prüfung noch. Die Installations- und Vorprüfschritte waren erfolgreich.
- Neuer Fehler: `fatal error: SI4735.h: No such file or directory` aus `src/tuner/TunerDriver.h`. Der reine `--libraries "$HOME/Arduino/libraries"`-Aufruf hat in diesem Setup den Header nicht aufgelöst.
- Korrektur in Commit `a2064d10359c06d8d2a7c5c62bbee161afe0dce8`: neben `--libraries` werden wieder explizit `--library "$HOME/Arduino/libraries/PU2CLR_SI4735"` und der Include-Pfad zur `src`-Ebene gesetzt. Der neue Lauf muss zeigen, ob damit sowohl Header als auch Bibliotheksimplementierung korrekt gebaut und gelinkt werden.
- Referenz zur Arduino-CLI-Abhängigkeitsauflösung: https://docs.arduino.cc/arduino-cli/sketch-build-process
- Nutzer startet Workflows selbst; kein Workflow wurde von uns gestartet. Kein Flashen und keine Änderung an Bootloader, Partitionen, APP0 oder APP1.
## Build-Fehlerstand – 2026-11-01 (Run 41271303901)

- Beide Profile (OSPI und QSPI) scheiterten im Linker erneut mit `undefined reference` auf den SI4735-Konstruktor und Methoden (`setup`, `setFM`, `setVolume`). Header, library.properties und `SI4735.cpp` wurden erfolgreich gefunden; das Problem bleibt, dass die Bibliotheksimplementierung nicht in den Link gelangt.
- Arduino CLI Dokumentation beschreibt, dass Dateien unter `src/` einer Arduino-Bibliothek normalerweise kompiliert und gelinkt werden; siehe https://docs.arduino.cc/arduino-cli/library-specification. Da der explizite `--library`-Pfad das Problem nicht beseitigt hat, wurde ein gezielter CI-Workaround ergänzt.
- Workflow-Fix in Commit `be6fbe391038c8a872918d717dbb1e611e712dd6`: Vor dem Kompilieren kopiert der Workflow die installierte Upstream-Quelldatei `SI4735.cpp` in `firmware/ats-mini-os/src/vendor/SI4735Library.cpp`. Damit wird sie als Sketch-Quelldatei kompiliert; der ursprüngliche SI4735-Header und Include-Pfad bleiben eingebunden.
- Dieser Workaround ist noch nicht durch einen neuen Lauf bestätigt. Nutzer startet den manuellen Compile-only-Workflow; wir starten keinen Workflow selbst.
- Kein Flashen und keine Änderungen an Hardware, Bootloader, Partitionen, APP0 oder APP1.

## Build-Lauf 41271303901 – SI4735-Linkerfehler weiterhin vorhanden (2026-11-01)

- Beide Jobs sind fehlgeschlagen; der Linker meldet weiterhin fehlende SI4735-Symbole für Konstruktor, `setup`, `setFM` und `setVolume`.
- Wichtig bei der Auswertung: Dieser Lauf enthält im Job-Schrittverlauf noch **nicht** den später hinzugefügten Schritt `Stage SI4735 implementation in sketch source tree`. Der Lauf wurde also mit einer älteren Workflow-Version ausgeführt und testet den Workaround aus Commit `be6fbe391038c8a872918d717dbb1e611e712dd6` nicht.
- Zusätzlich zeigten Compilerwarnungen, dass `TunerDriver.h` die Grenzen 87500 und 108000 kHz als `int16_t` definierte; beide Werte liegen außerhalb des Wertebereichs dieses Typs. In Commit `44b4681492c229142ad24000f8cfc0bb97238a27` wurden die beiden Konstanten auf `uint32_t` geändert.
- Nächster Schritt: manuellen Workflow erneut starten, damit der aktuelle Workflow samt SI4735-Staging-Schritt und der korrigierten Frequenztypen getestet wird. Wir starten keine Actions selbst.
- Weiterhin Compile-only: kein Flashen und keine Änderungen an Bootloader, Partitionen, APP0, APP1 oder Recovery.


## 42. Erster erfolgreicher Compile des modularen FM-Scaffolds – Run 41272465032 (2026-11-01)

Der Nutzer hat den manuellen Workflow-Lauf [41272465032](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41272465032) bereitgestellt. Beide Jobs sind erfolgreich abgeschlossen:
- QSPI: Job `124747871639`, Ergebnis `success`.
- OSPI: Job `124747871789`, Ergebnis `success`.

Der Job-Verlauf bestätigt, dass der Schritt `Stage SI4735 implementation in sketch source tree` tatsächlich ausgeführt wurde. Die SI4735-Implementierung wurde im kurzlebigen CI-Workspace als Sketch-Quelldatei bereitgestellt, sodass der vorherige Linkerfehler überwunden wurde. Die gestagte Vendor-Datei ist ein CI-Schritt und wird nicht als dauerhaftes Vendor-File im Repository vorausgesetzt.

Compilerberichte aus den Logs:
- QSPI: Programmcode 334.712 Byte; globale Variablen 24.224 Byte.
- OSPI: Programmcode 337.008 Byte; globale Variablen 24.624 Byte.
- Beide Builds geben eine Warnung in der Upstream-SI4735-Implementierung aus: Variable `mjd` könnte uninitialisiert verwendet werden. Das blockiert den Build nicht, sollte später geprüft werden. Node.js-/Action-Deprecation-Warnungen sind separat von Firmware-Compilerfehlern zu betrachten.

**Interpretation:** Dies ist die erste erfolgreiche Compile-Basis für den aktuellen modularen FM-Scaffold, kein Beleg für Hardwarefunktion. Die Pinbelegung, Tunerinitialisierung, Audio, Empfang, RSSI/SNR/Stereo und Persistenz sind noch nicht am ATS Mini V4 getestet. Kein Flashen; APP0, APP1, Bootloader, Recovery und Partitionen wurden nicht verändert. Die 8-MB-Partition-CSV bleibt Compile-Referenz und darf nicht auf das 16-MB-Gerät geflasht werden.

**Nächster sicherer Schritt:** Compile-Basis dokumentieren und Quellcode/Build-Konfiguration gezielt prüfen, dann einen kleinen nächsten Entwicklungsumfang festlegen. Der Nutzer startet jeden Workflow manuell; wir starten keine GitHub Actions selbst.

## 43. Menü-Konzept – Fullscreen-Hauptmenü mit Untermenüs (2026-11-01)

Der Nutzer hat das gewünschte Bedienprinzip beschrieben. Dies ist eine Planungsanforderung; noch keine Menüimplementierung.

- Echtes Fullscreen-Menü statt vieler unabhängig aufgebauter Einzelmenüs.
- Ein Hauptmenü mit einer vertikal scrollbaren Liste. Einträge öffnen passende Untermenüs.
- Gemeinsame Menü-Engine und einheitliche Encoder-Bedienung, um doppelte Darstellung und Steuerlogik zu vermeiden und möglichst Code zu sparen.
- Menü-Darstellung und Funktionalität werden getrennt: Ein Menüeintrag ruft ein zuständiges Funktionsmodul auf, das seine eigentliche Arbeit unabhängig ausführt.
- Für passende Ein-/Aus-Einstellungen wird unter dem Namen eine Statuszeile angezeigt, beispielsweise `Aktueller Status: Ein` oder `Aktueller Status: Aus`. Der Beispieltext des Nutzers (Licht Ein/Aus) war nur eine Illustration und ist kein festgelegter Menüpunkt.
- Der Status muss den tatsächlichen Zustand der jeweiligen Funktion anzeigen und nach Änderungen aktualisiert werden.
- Konkrete Haupt- und Untermenüpunkte, Layout und Farben werden später gemeinsam festgelegt.
- Menüarbeit beginnt erst nach Stabilisierung der aktuellen Firmware-Grundlage; sie darf die laufenden Build-/Radio-Schritte nicht unnötig vermischen.


## 44. Quellcode-Audit nach erfolgreichem Compile – FM-Grundgerüst (2026-11-01)

Nach Run `41272465032` wurden die aktuellen Dateien im Branch `dev/first-build-preparation` lesend geprüft. Es wurden keine Firmwaredateien geändert und kein Workflow gestartet.

### Bestätigte Beobachtungen
- `ats-mini-os.ino` startet `RadioCore` mit 95,4 MHz als Fallback und gibt Diagnosedaten seriell aus; Display und Encoder sind noch nicht integriert.
- `RadioCore` lädt die letzte Frequenz aus NVS, pollt Frequenz/RSSI/SNR/Pilotstatus alle 500 ms und verschiebt das Speichern nach Abstimmaktivität um drei Sekunden.
- `TunerDriver` kapselt direkte SI4735-Aufrufe; die dokumentierte Referenzbelegung ist SDA GPIO18, SCL GPIO17, LDO GPIO15, Reset GPIO16. Diese Pins sind weiterhin nicht am konkreten Gerät validiert.
- `TunerDriver::begin()` setzt `ready_ = true` unmittelbar nach `setup()`, `setFM()` und `setVolume()`. Aus dem aktuellen Code allein folgt nicht, dass der Chip tatsächlich geantwortet hat. Der Status `ready` bedeutet daher derzeit nur, dass die Initialisierungssequenz aufgerufen wurde, nicht dass Empfang hardwareseitig bestätigt ist.
- Der aktuelle Tuner-Treiber pollt SI4735-Abfragen über `const_cast`, weil die Bibliotheksmethoden nicht als `const` verfügbar sind. Das ist kompilierbar, aber beim nächsten Treiber-Refactoring auf eine klarere Schnittstelle zu prüfen.
- Die NVS-Frequenz wird auf 87,5–108,0 MHz und 10-kHz-Raster validiert. Die aktuellen Benutzer-Tuning-Schritte im RadioCore betragen 100 kHz.
- Die beiden Profile in `sketch.yaml` nutzen weiterhin `FlashSize=8M` und `PartitionScheme=custom`; diese Einstellungen gelten ausschließlich für Compile-Tests. Die 8-MB-Referenz-`partitions.csv` ist inkompatibel mit dem dokumentierten 16-MB-Geräte-IST-Layout und darf nicht als Flashkonfiguration verwendet werden.

### Nächster Schritt
1. Die tatsächlich installierte PU2CLR SI4735 2.1.8 API gezielt gegen die Initialisierungs-/Fehlererkennung prüfen; nicht raten, welche Methoden eine erfolgreiche Hardwareantwort bestätigen.
2. Einen minimalen, aussagekräftigen Fehlerstatus für Tunerinitialisierung definieren und die serielle Diagnose klar zwischen `init sequence issued` und `hardware response confirmed` unterscheiden.
3. Erst danach einen kleinen Codepatch vorbereiten, dann den Nutzer den manuellen Compile-only-Workflow ausführen lassen.
4. Hardwareverhalten erst separat testen, nachdem ein sicheres, ausdrücklich freigegebenes APP1-Image-/Bootverfahren anhand der realen 16-MB-Partitionierung verifiziert wurde.

**Sicherheitsstatus:** kein Flash, keine Änderungen an APP0/APP1, Bootloader, Recovery oder Partitionen; keine GitHub Actions durch den Assistenten gestartet.


## 45. SI4735 2.1.8: hardware detection and I2C pin setup

Read-only source inspection of the official PU2CLR `2.1.8` release confirmed:

- `SI4735::setup(...)` returns `void`; calling it cannot by itself prove that a tuner answered.
- `SI4735::getDeviceI2CAddress(resetPin)` pulses RESET, checks I2C addresses `0x11` and `0x63`, sets the library address when found, and returns `0` if neither address responds.
- The library's `setup()` calls `Wire.begin()` internally. Arduino-ESP32 documents `Wire.setPins(SDA, SCL)` as the way to configure custom pins before a library calls `Wire.begin()`.

### Code change

`firmware/ats-mini-os/src/tuner/TunerDriver.cpp` now configures SDA=18/SCL=17 with `Wire.setPins()`, checks its return value, then calls `getDeviceI2CAddress()` before normal tuner setup. If the bus setup fails or no SI47XX responds at either supported address, `begin()` returns false and `ready_` remains false; diagnostic messages are printed to Serial. The detected address is logged on success.

Sources:
- PU2CLR SI4735 release 2.1.8: https://github.com/pu2clr/SI4735/releases/tag/2.1.8
- SI4735 2.1.8 source: https://github.com/pu2clr/SI4735/blob/2.1.8/src/SI4735.cpp
- Arduino-ESP32 I2C pin configuration: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/i2c.html

### Verification status and caveats

- Code change is committed, but no new compile has been run; the user starts GitHub Actions manually.
- Hardware detection is now based on an I2C address acknowledgement, not a complete proof that FM audio/tuning works.
- The library's internal setup sequence still uses void-returning commands, so further runtime verification would need to be carefully designed against the exact library status/command behavior.
- No firmware was flashed. APP0, APP1, bootloader, recovery, and the device's 16 MB partition layout were not changed. The repository's 8 MB partition CSV remains compile-reference-only and must not be flashed to the device.


## 46. FM-Signalabfrage und RDS-Vorbereitung (2026-11-01)

### Aktueller Stand
- Entwicklungsbranch: `dev/first-build-preparation`.
- Der letzte zuvor gemeldete erfolgreiche Compile-only-Lauf war GitHub Actions Run [41273167361](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41273167361). Dieser Lauf enthält noch nicht die nachfolgend beschriebenen späteren Änderungen.
- In `TunerDriver.cpp` wurde danach RDS-Initialisierung ergänzt: `radio_.RdsInit()` und `radio_.setRdsConfig(1, 2, 2, 2, 2)`.
- Commit für diese RDS-Initialisierung: `0bb00c68f3e1e9848e1e5c13291bcf01b2afc6a7`.
- Bereits im Code vorhanden ist `refreshSignalQuality()`, das vor dem Lesen von RSSI/SNR/Pilot `getCurrentReceivedSignalQuality()` aufruft. `RadioCore` ruft diese Methode bei der Initialisierung und bei seinem 500-ms-Poll auf.
- **Erfolgreicher Compile-only-Nachweis:** GitHub Actions Run [41274168929](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41274168929) ist für beide Jobs erfolgreich: `esp32s3-qspi` und `esp32s3-ospi`. Damit ist die RDS-Initialisierung im aktuellen Build kompilierbar. Die RDS-Daten (PS, PI, PTY, Radiotext) werden noch nicht aktiv in den Snapshot übernommen; die RDS-Initialisierung allein bedeutet noch keine fertige RDS-Anzeige.

### Referenz aus `13HN2709/ats-mini-dev`
- Alte `Station.cpp` verarbeitet RDS-Gruppen in begrenzten Batches (Limit 25) und behandelt Sendername/PS, PI, PTY und Radiotext getrennt.
- Alte `ats-mini.ino` fragt zuerst `getCurrentReceivedSignalQuality()` ab und liest danach RSSI/SNR.
- Die alten RDS-Funktionen stammen aus einer abweichenden SI4735-Fork/API (`SI4735-fixed.h)); Funktionsaufrufe dürfen deshalb nicht ungeprüft in das neue OS kopiert werden.
- Ziel ist ein eigener, modularer RDS-Service mit stabilem Sendernamen-Puffer und getrenntem Radiotext-Puffer. Vor Implementierung sind die tatsächlichen RDS-Methoden der im neuen OS verwendeten PU2CLR-SI4735-Version zu verifizieren.

### Nächste Schritte
1. Der Compile-only-Workflow wurde vom Nutzer manuell gestartet und ist in Run [41274168929](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41274168929) für QSPI und OSPI erfolgreich durchgelaufen.
2. Als Nächstes die installierte SI4735-API verifizieren und einen separaten RDS-Service ergänzen (PS, PI, PTY, RT), ohne die alte Firmware zu verändern.
4. Kein Firmware-Flash und keine Änderung an Bootloader, APP0/APP1, Recovery oder Partitionen ohne ausdrückliche Freigabe. Die 8-MB-Partitionsdatei bleibt ausschließlich Compile-Referenz und darf nicht auf das 16-MB-Gerät geflasht werden.

**Verifikationsstatus:** RDS-Initialisierung committed und durch Compile-only-Run `41274168929` für QSPI und OSPI bestätigt; RDS-Decodierung und Hardwaretest noch offen.


## 47. PU2CLR SI4735 2.1.8 RDS-API geprüft (2026-11-01)

### Geprüfte Original-API
Die Header- und Implementierungsdateien der offiziellen PU2CLR-Version 2.1.8 wurden direkt im Tag `2.1.8` geprüft:

- Header: https://github.com/pu2clr/SI4735/blob/2.1.8/src/SI4735.h
- Implementierung: https://github.com/pu2clr/SI4735/blob/2.1.8/src/SI4735.cpp
- Verfügbare Funktionen sind unter anderem `getRdsStatus(0, 0, 0)`, `getRdsReceived()`, `getRdsNewBlockA()`, `getRdsNewBlockB()`, `getRdsPI()`, `getRdsGroupType()`, `getRdsProgramType()`, `getRdsText0A()` und `getRdsText2A()`.
- Laut Bibliotheksheader muss vor dem Auslesen der RDS-Felder zuerst `getRdsStatus(...)` aufgerufen werden. `getRdsText0A()` verarbeitet nur Gruppe 0 und gibt bei nicht passender Gruppe `nullptr` zurück; `getRdsText2A()` verarbeitet Gruppe 2A und gibt sonst `nullptr` zurück.
- Die RDS-Statusabfrage liest eine Antwort von 13 Bytes. Die Bibliotheksimplementierung wartet bei gesetztem Fehlerbit intern in einer Schleife; deshalb darf der Anwendungscode keine unbeschränkten eigenen RDS-Schleifen ergänzen. Die RDS-Abfrage soll mit einem festen Gruppen-/Zeitbudget pro Update erfolgen.
- Die APIs der alten `ats-mini-dev`-Firmware dürfen nicht ungeprüft übernommen werden. Dort wurde eine abweichende SI4735-Fork verwendet.

### Entwurf für den separaten RDS-Service
- Der Treiber bleibt alleiniger Besitzer des `SI4735`-Objekts und kapselt alle direkten Bibliotheksaufrufe.
- Der RDS-Service verarbeitet die gelieferten RDS-Daten getrennt vom Radio-Core und schreibt die Ergebnisse in den bestehenden `RadioSnapshot`.
- Sendername (PS, maximal 8 Zeichen) und Radiotext (RT, maximal 64 Zeichen) bleiben getrennte Felder; ein fehlender/ungültiger neuer Text darf den zuletzt gültigen Sendernamen nicht löschen.
- PI und PTY werden nur aktualisiert, wenn ein gültiger RDS-Block verfügbar ist. `rdsAvailable` soll tatsächlichen empfangenen RDS-Status widerspiegeln, nicht allein die erfolgreiche Initialisierung.
- Radiotext benötigt zusätzlich die Behandlung von Text-Flag A/B und Textsegmenten, damit beim Wechsel einer Meldung keine Fragmente alter und neuer Texte vermischt werden.

### Verifikationsstatus
- Compile-only-Run [41274168929](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41274168929) ist für QSPI und OSPI erfolgreich; er bestätigt die RDS-Initialisierung, aber noch keine RDS-Decodierung.
- Diese API-Prüfung ist eine Quellcodeprüfung. Es wurde kein Gerät getestet, kein Firmware-Image geflasht und keine Partition oder Bootkomponente geändert.
- Nächster Schritt: den RDS-Service und eine schmale Treiberschnittstelle implementieren; danach den Nutzer den manuellen Compile-only-Workflow erneut starten lassen.


## 48. Erster modularer RDS-Polling-Service implementiert (2026-11-01)

### Änderungen im Branch
- `firmware/ats-mini-os/src/tuner/TunerDriver.h`: neuer `TunerRdsData*-Datenvertrag und `pollRds()`-Schnittstelle; der Treiber bleibt Eigentümer des SI4735-Objekts.
- `firmware/ats-mini-os/src/tuner/TunerDriver.cpp`: liest pro Aufruf genau einen RDS-Status, verarbeitet gültige PI-/PTY-Blöcke und Gruppen 0 (PS) sowie 2A/2B (Radiotext). Text wird begrenzt in eigene Ausgabepuffer kopiert.
- `firmware/ats-mini-os/src/radio/RadioCore.h/.cpp`: fragt RDS alle 250 ms ab und überträgt nur aktualisierte Felder in den Snapshot. StationName und RadioText bleiben getrennt; der zuletzt gültige Sendername wird bei normalem Polling nicht durch leere Daten überschrieben.
- Es wurden keine UI-Dateien, Bootloader-, Flash- oder Partitionsdateien verändert.

### Commits
- Tuner-Schnittstelle: `a69dd10f958a5183eb197acdaef11139be863c5c`
- Tuner-Polling: `6d7dacb558dc518e617368a19d133def1cc7fa32`
- RadioCore-Deklaration: `2cfa531332f16424ce7c690148b90e26dc65655d`
- RadioCore-Integration: `72af1d4277f0be49e6c88da2916904901646712b`

### Einschränkungen und nächste Prüfung
- **Build noch nicht verifiziert.** Der Nutzer muss den Compile-only-Workflow manuell starten: https://github.com/13HN2709/ATS_Mini_-OS/actions/workflows/ats-mini-os-first-compile.yml
- Nach einem erfolgreichen Build soll die RDS-Textsegment-/A/B-Flag-Behandlung gezielt überprüft werden. Die PU2CLR-Bibliothek hält interne RT-Puffer; die aktuelle Schnittstelle übernimmt deren Inhalt, daher ist die saubere Trennung alter und neuer RT-Segmente noch im Build-/Hardwaretest zu validieren.
- `rdsAvailable` wird gesetzt, sobald ein empfangener RDS-Status vorliegt; tatsächliche Stabilität und Empfangsqualität müssen am Gerät geprüft werden.
- Kein Firmware-Flash, kein Hardwaretest und keine Änderung der Partitionstabelle durchgeführt. Die 8-MB-Compile-Referenzpartition bleibt ausschließlich für CI.


## 49. RDS-A/B-Pufferbereinigung und Startdiagnose (2026-11-01)

### Nach dem grünen Lauf 41274825544 vorgenommene Änderungen
- Nutzerbestätigter Lauf [41274825544](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41274825544): beide Compile-only-Jobs `esp32s3-ospi` und `esp32s3-qspi` erfolgreich.
- Danach wurde `TunerDriver.cpp` angepasst: Die PU2CLR-SI4735-2.1.8-Implementierung löscht die RT-Puffer bei Wechsel des Text-Flags A/B nicht automatisch. Beim erkannten Wechsel werden nun `clearRdsBuffer2A()` und `clearRdsBuffer2B()` aufgerufen, bevor das aktuelle Segment der neuen Meldung übernommen wird.
- `ats-mini-os.ino` gibt im 2-Sekunden-Serienstatus zusätzlich RDS-Verfügbarkeit, PI, PTY, PS und RT aus. Das erleichtert den ersten sicheren Bring-up über den seriellen Monitor, bevor Display und Encoder integriert werden.
- Commits nach Run 41274825544:
  - RT-Pufferbereinigung: `2e89494fa299931d5a7672089c2ea27d4fe9169d`
  - RDS-Seriendiagnose: `006702e8388153073f29b86d2eb474eae83855ba`

### Nächster Schritt
- Nutzer startet den Compile-only-Workflow manuell: https://github.com/13HN2709/ATS_Mini_-OS/actions/workflows/ats-mini-os-first-compile.yml
- Beide Profile müssen nach diesen letzten Commits erneut erfolgreich kompilieren.
- Wenn der Build grün bleibt, ist als nächster Entwicklungsblock die erste Display-/Encoder-Inventarisierung und eine separate, minimale UI-Anbindung vorzubereiten. Pins, Displaycontroller und benötigte Bibliotheken müssen vor Codeannahmen verifiziert werden.

### Grenzen
- Der Lauf 41274825544 liegt **vor** den zwei genannten Commits und verifiziert diese Änderungen nicht.
- Kein Workflow durch den Assistenten gestartet, kein Firmware-Flash, keine Bootloader-/APP0-/APP1-/Recovery-/Partitionsänderung.
- Die 8-MB-Partitionsdatei bleibt ausschließlich Compile-Referenz und darf nicht auf das 16-MB-Gerät geflasht werden.


## 50. Compile-Fehler aus Run 41275391304 behoben (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41275391304/job/124756434503
- OSPI-Compile schlug fehl, weil `SI4735::clearRdsBuffer2A()` und `clearRdsBuffer2B()` in PU2CLR SI4735 2.1.8 als `protected` deklariert sind und daher nicht direkt von `TunerDriver` aufgerufen werden dürfen.
- Korrektur: `TunerSI4735` erbt von `SI4735` und stellt eine kleine öffentliche Adaptermethode `clearRadioTextBuffers()` bereit, die intern die beiden geschützten Methoden aufruft. `TunerDriver` verwendet nun diesen Adapter.
- Fix-Commits:
  - `48b29dff095a09c112ddefff4d891b7321cbac69`
  - `1c479ff4956eb8664db69d682dfb3cc68ec8f793`
- Der QSPI-Job des Runs war beim Abruf noch in Arbeit; unabhängig davon ist der OSPI-Fehler eindeutig.
- Nächster Schritt: Nutzer startet den Compile-only-Workflow manuell erneut und sendet den Run-Link. Keine GitHub Action durch den Assistenten gestartet.
- Keine Flash-/Bootloader-/Partitionsänderungen. Die 8-MB-Partitionsdatei bleibt Compile-Referenz-only und darf nicht auf das 16-MB-Gerät geflasht werden.


## 51. Compile-Fehler aus Run 41275768795 korrigiert (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41275768795
- Beide Compile-only-Jobs (QSPI und OSPI) scheiterten in `TunerDriver.cpp` an vier `const_cast`-Ausdrücken. Nach dem Wechsel des Feldtyps von `SI4735` zu `TunerSI4735` versuchten die Casts weiterhin, eine `const TunerSI4735`-Referenz direkt als `SI4735&` zu behandeln; das ist kein gültiger `const_cast`.
- Korrektur in Commit `e0bac0c1e56622295b0429123f990051a0908741`: Die vier Cast-Ziele in `frequencyKHz()`, `rssi()`, `snr()` und `stereo()` wurden auf `TunerSI4735&` geändert.
- Nächster Schritt: Nutzer startet den manuellen Compile-only-Workflow erneut und sendet den neuen Run-Link. Der Assistent löst keine GitHub Actions selbst aus.
- Die Warnung über möglicherweise uninitialisiertes `mjd` in der SI4735-Upstream-Implementierung ist nicht der Grund für diesen Build-Abbruch; sie ist eine separate Warnung und kann nach Stabilisierung des Builds geprüft werden.
- Kein Flashen und keine Änderungen an Bootloader, APP0, APP1, Recovery oder Partitionen. Die 8-MB-Partitionsdatei bleibt ausschließlich Compile-Referenz und darf nicht auf das 16-MB-Gerät geflasht werden.


## 52. Compile-only-Build wieder grün – Run 41276405931 (2026-11-01)

- Nutzerbestätigter Lauf: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41276405931
- Branch: `dev/first-build-preparation`; geprüfter Commit: `b7374e047549cbcb18d392c4d50203cc630b10cd`.
- Beide Jobs erfolgreich:
  - QSPI: Job `124759364156`, Ergebnis `success`.
  - OSPI: Job `124759364372`, Ergebnis `success`.
- Der Workflow führte in beiden Profilen die Installation/Prüfung der SI4735-Bibliothek, das Staging der SI4735-Implementierung und den Compile-Schritt erfolgreich aus.
- Damit ist der letzte `const_cast`-Compilefehler behoben und der aktuelle Stand wieder compile-grün.
- **Wichtig:** Das ist ausschließlich ein Compile-only-Nachweis. Kein Geräteboot, kein Empfang/RDS-Test, keine Display-/Encoder-Prüfung und keine Flash-Aktion wurden bestätigt.
- Nächster sicherer Arbeitsschritt: gezielte, lesende Inventarisierung der Display- und Encoder-Hardware/API und Abgleich mit dem konkreten ATS Mini V4; danach erst einen isolierten Minimal-UI-Umfang planen.
- Keine GitHub Actions durch den Assistenten gestartet. Kein Flashen, keine Änderung an Bootloader, APP0, APP1, Recovery oder Partitionen. Die 8-MB-Partitionsdatei bleibt Compile-Referenz-only und darf nicht auf das 16-MB-Gerät geflasht werden.


## 53. ATS Mini DEV Displaytreiber isoliert integriert (2026-11-01)

- Neue Dateien: `firmware/ats-mini-os/src/display/DisplayDriver.h/.cpp`. Der LovyanGFX-Treiber basiert auf der lesend untersuchten Referenz `13HN2709/ats-mini-dev/ats-mini/Display.h`.
- Übernommen wurden der ATS-Mini-8-Bit-Parallelbus, die ST7789-Initialisierung, die GC9307-Variante und die RDDID-Erkennung. Die separate LILYGO-SI473X-Konfiguration wird für ATS Mini V4 nicht verwendet.
- Erste UI: schwarzer Hintergrund, neon-grüne Frequenz und Umrandung, separater Sendernamenbereich, eigene Radiotextzeile, RSSI/SNR und Stereo/Mono-Status.
- `ats-mini-os.ino` initialisiert den Displaytreiber und aktualisiert die Anzeige aus `RadioSnapshot`. Bei fehlgeschlagener Displayinitialisierung bleiben serielle Diagnosen verfügbar.
- Der manuelle Compile-Workflow installiert nun zusätzlich LovyanGFX und prüft den Header.
- **Noch nicht kompiliert.** Bitte den bestehenden Workflow manuell starten und den Run-Link teilen. Der Assistent hat keine GitHub Action gestartet.
- Pinbelegung/Panel-ID sind Referenzwerte und am konkreten Gerät nicht physisch bestätigt. Radiotext-Scrolling und Encoderbedienung sind noch nicht implementiert.
- Keine Flash-Aktion und keine Änderung an Bootloader, APP0/APP1, Recovery oder Partitionen. Die 8-MB-Profile bleiben ausschließlich Compile-Referenz.

## 54. Displaytreiber-Headerguard korrigiert nach Run 41281847197 (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41281847197
- Beide Compile-only-Jobs (QSPI und OSPI) erreichten den Sketch-Compile-Schritt, scheiterten jedoch in `DisplayDriver.h` mit `error: unterminated #ifndef`.
- Ursache: Am Ende der Headerdatei fehlte das abschließende `#endif` für `ATS_MINI_OS_DISPLAY_DRIVER_H`.
- Korrektur in Commit `ac4411d63e6c9ea641acfa1bbea5e813855f9a1e): `#endif // ATS_MINI_OS_DISPLAY_DRIVER_H` ergänzt.
- Die Bibliotheksinstallation und die Headerprüfungen für LovyanGFX und SI4735 waren erfolgreich; der zuvor vermutete fehlende LovyanGFX-Include-Pfad war nicht mehr der aktuelle Abbruchgrund.
- Nächster Schritt: Nutzer startet den Compile-only-Workflow manuell erneut und sendet den Run-Link. Der Assistent startet keine GitHub Actions selbst.
- Nutzer hat bestätigt, dass die Pinbelegung aus ATS Mini DEV auf seinem ATS Mini funktioniert. Die neue OS-Treiberintegration braucht trotzdem noch einen erfolgreichen Compile und anschließend einen vorsichtigen Hardwaretest.
- Kein Flashen und keine Änderung an Bootloader, APP0/APP1, Recovery oder Partitionen. Die 8-MB-Partitionsdatei bleibt Compile-Referenz-only und darf nicht auf das 16-MB-Gerät geflasht werden.

## 55. LovyanGFX-Linkerfehler aus Run 41282435431 eingegrenzt (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41282435431
- Beide Profile erreichen jetzt den Linker; die Headerdatei wird gefunden und der fehlende Headerguard ist behoben.
- Linker meldet zahlreiche `undefined reference`-Symbole aus `lgfx::LGFXBase`, `Panel_Device` und `LGFX_Device`. Die Compile-Ausgabe listet LovyanGFX nicht unter den tatsächlich verwendeten/kompilierten Bibliotheken.
- Korrektur in Commit `2ef743515afa5eb39bbd90873ca862f1c863dcc7`: die zwei singulären `--library`-Argumente entfernt; Arduino CLI soll LovyanGFX über den angegebenen Bibliotheksordner `--libraries "$HOME/Arduino/libraries"` regulär erkennen und dessen Implementierungsdateien mitkompilieren.
- Diese Änderung ist noch nicht verifiziert. Nutzer startet den manuellen Compile-only-Workflow erneut und teilt den Run-Link; Assistent startet keine Actions.
- Kein Flashen und keine Änderungen an Bootloader, APP0/APP1, Recovery oder Partitionen.


## 56. LovyanGFX weiterhin nicht gelinkt – Workflow-Anpassung nach Run 41283059049 (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41283059049
- Beide Profile scheitern weiterhin beim Linken mit zahlreichen fehlenden LovyanGFX-Symbolen. In der „Used library“-Liste stehen nur `Wire` und `Preferences`; LovyanGFX wird nicht als verwendete Bibliothek geführt.
- Wahrscheinlicher Auslöser: Der manuell ergänzte `-I.../LovyanGFX/src`-Include-Pfad lässt den Header direkt auffindbar werden, ohne dass Arduino CLI die Bibliothek korrekt als Abhängigkeit aufnimmt. Das ist eine Diagnose-Hypothese, noch kein bestätigter Befund.
- Workflow-Anpassung in Commit `1276a7bb119bc47b08d150b6e5492a555d8f3136`: LovyanGFX wird wieder ausdrücklich über `--library "$HOME/Arduino/libraries/LovyanGFX"` angegeben; der manuelle LovyanGFX-Include-Pfad wurde aus `compiler.cpp.extra_flags` entfernt. Der SI4735-Include-Pfad bleibt erhalten.
- Änderung noch nicht getestet. Nutzer startet den manuellen Workflow selbst und sendet den nächsten Run-Link. Keine GitHub Actions durch den Assistenten gestartet.
- Keine Flash-Aktion und keine Änderungen an Bootloader, APP0/APP1, Recovery oder Partitionen. Die 8-MB-Profile bleiben Compile-Referenz-only und dürfen nicht auf das 16-MB-Gerät geflasht werden.


## 57. Arduino CLI findet LovyanGFX aus User-Library-Ordner nicht – Run 41283468733 (2026-11-01)

- Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41283468733
- Beide Profile brechen nun bereits beim Header ab: `fatal error: LovyanGFX.hpp: No such file or directory`. Die vorherige Änderung, die den zusätzlichen Include-Pfad entfernt hatte, machte den Header nicht mehr sichtbar.
- Arduino CLI-Dokumentation beschreibt Library-Auflösung über installierte Libraries; es gibt außerdem einen aktuellen Arduino CLI Issue mit ähnlichem Verhalten bei User-Libraries unter `directories.user` (Issue #3253, 2026). Das ist ein plausibler Tooling-Fehler, nicht belegt als alleinige Ursache für diesen konkreten Build.
- Workflow-Workaround in Commit `eea3684a0f4f56f558154e2ea9d54b396e16a5a6`: Die installierte LovyanGFX-Bibliothek wird im temporären GitHub-Runner zusätzlich nach `~/.arduino15/packages/esp32/hardware/esp32/3.3.12/libraries/LovyanGFX` kopiert. Der Sketch-Compile-Schritt kann die Bibliothek dann als ESP32-Core-Bibliothek auflösen und deren Quellen einbinden. Diese Kopie existiert ausschließlich im kurzlebigen CI-Runner, nicht auf dem Radio.
- Noch ungetestet; Nutzer startet den Workflow manuell. Assistent startet keine GitHub Actions.
- Keine Flash-Aktion und keine Änderung an Bootloader, APP0/APP1, Recovery oder Partitionen. Die 8-MB-Profile bleiben Compile-Referenz-only.


## 58. Build #36 grün und Encoder-Modul angebunden (2026-11-02)

- Nutzerbestätigter Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41361949091
- Commit ef26676c4fedb141855464d480024dd33d51065e; beide Compile-only-Profile esp32s3-qspi und esp32s3-ospi erfolgreich.
- Der Fehler LGFX_Sprite.inl: No such file or directory ist für den aktuellen Workflow behoben.
- Nächster isolierter Entwicklungsschritt: Encoder GPIO A=2, B=1, Push=21 per Polling integrieren; Drehen verändert FM-Frequenz in 100-kHz-Schritten über RadioCore::nextChannel(). Kurz-/Langdruck werden zunächst nur seriell protokolliert; keine Menüaktion ist damit verbunden.
- Display-/Radio-Hardwareverhalten ist weiterhin nicht physisch bestätigt. Nach dem nächsten manuellen Compile-Lauf zunächst die Eingabeintegration compile-grün halten; Hardwaretest nur nach ausdrücklicher Nutzerentscheidung.
- Der Nutzer startet den Workflow manuell. Keine Workflow-Auslösung durch den Assistenten, kein Flashen, keine Änderung an Bootloader, APP0/APP1, Recovery oder Partitionen. Die 8-MB-Compile-Profile bleiben ausschließlich CI-Referenz und dürfen nicht auf das 16-MB-Gerät geflasht werden.

## Aktueller Handover – ATS Mini V4 GPIO-Abgleich und Audio/Backlight (2026-11-02)

### Nutzerentscheidung / verbindliche Referenz
- Der Nutzer hat ausdrücklich festgelegt: Für ATS Mini OS ist `13HN2709/ats-mini-dev` die Hardware-/GPIO-Referenz. Was dort funktioniert, soll im neuen OS nach Möglichkeit gleichartig übernommen werden.
- Handover laufend aktualisieren: neue Erkenntnisse, Commits, Buildstatus und nächste Schritte direkt hier dokumentieren, damit ein neuer Chat ohne Wiederholung fortsetzen kann.
- Keine GitHub Actions selbst starten. Der Nutzer startet den manuellen Workflow und schickt den Run-Link.
- Kein Flashen und keine Änderungen an Bootloader, APP0/APP1, Recovery oder Partitionstabelle. Die 8-MB-Profile sind Compile-Referenz und dürfen nicht auf das 16-MB-Gerät geflasht werden. Das bekannte Backup und der derzeitige Bootloader bleiben unangetastet.
- GPIO-Referenz hilft bei der Portierung, garantiert aber nicht, dass sämtliche Funktionen ohne Hardwaretest funktionieren.

### Geprüfte GPIOs aus `ats-mini-dev/ats-mini/Common.h`
| Funktion | GPIO | Referenzverhalten |
|---|---:|---|
| Radio-LDO | 15 | HIGH = ein |
| SI4735 Reset | 16 | Reset |
| I²C SDA / SCL | 18 / 17 | Daten / Takt |
| Audio-Mute | 3 | HIGH = stumm |
| Audioverstärker Enable | 10 | HIGH = ein |
| Display-Backlight | 38 | PWM, Referenz nutzt 16 kHz / 8 Bit |
| Encoder A / B | 2 / 1 | Drehgeber |
| Encoder-Taste | 21 | Eingang |
- Referenzlautstärke: 35/63. Nicht ändern.

### Änderungen auf `dev/first-build-preparation`
1. Commit `1120d02dfe0c0e7f98cbae0de42eb09c7b9e1c65`: `TunerDriver.cpp` registriert GPIO 3 nun zusätzlich mit `radio_.setAudioMuteMcuPin(kAudioMutePin)`. Mute bleibt vor der SI4735-Initialisierung HIGH, Amp GPIO 10 zunächst LOW, LDO GPIO 15 HIGH; nach Tuner-Setup wird Amp HIGH und Mute LOW gesetzt. Volume bleibt 35. Die offizielle PU2CLR-SI4735-API wurde read-only geprüft: `setAudioMuteMcuPin(int8_t)` setzt den Pin als Ausgang und hinterlegt ihn als externe Mute-Leitung; `setHardwareAudioMute(bool)` setzt den Pegel mit 300-µs-Verzögerung. Die Hardwarewirkung und das Verhalten bei Tune-/Power-Übergängen sind noch nicht am Gerät bestätigt.
2. Commit `e5c69045941baabf171d168a4061768f3a1f54d2`: `DisplayDriver.cpp` initialisiert bei Nicht-LILYGO-Builds GPIO 38 per `ledcAttach(38, 16000, 8)`, schaltet die Hintergrundbeleuchtung vor Panel-Init aus und setzt sie nach erfolgreichem Display-Init auf Helligkeitswert 180. Noch nicht am Gerät bestätigt.
- Links: [Audio-Mute-Anbindung](https://github.com/13HN2709/ATS_Mini_-OS/commit/1120d02dfe0c0e7f98cbae0de42eb09c7b9e1c65), [Backlight GPIO38](https://github.com/13HN2709/ATS_Mini_-OS/commit/e5c69045941baabf171d168a4061768f3a1f54d2).
- Die Dateien wurden im Branch `dev/first-build-preparation` geändert. Die `HANDOVER.md`-Einträge werden sowohl hier auf `main` als auch im Dev-Branch gepflegt.

### Letzter bestätigter Build und nächster Schritt
- Letzter bestätigter Compile-only-Run vor den beiden oben genannten Commits: [Run 41365786325 / Build #39](https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41365786325), beide Jobs OSPI und QSPI erfolgreich.
- Dieser grüne Build enthält die Commits `1120d02` und `e5c6904` noch nicht. Sie sind deshalb **noch nicht neu kompiliert** und nicht auf echter Hardware getestet.
- Nächster Schritt: Nutzer startet den manuellen Workflow [ATS Mini OS First Compile](https://github.com/13HN2709/ATS_Mini_-OS/actions/workflows/ats-mini-os-first-compile.yml) und teilt den Run-Link. Beide Profile OSPI/QSPI kontrollieren; erst danach sichere Hardwaretests nach Zustimmung des Nutzers planen.
- Build-Artefakte des bisherigen Laufs heißen `ats-mini-os-compile-report-esp32s3-ospi` und `ats-mini-os-compile-report-esp32s3-qspi`; je ca. 46 MB und Compile-Reports, kein bestätigter flashbarer Firmware-Release. Ablaufdatum 16.11.2026.
- Display-UI ist derzeit RDS-Testansicht (Frequenz, PS, PI/PTY, RT, Stereo/Mono, RSSI/SNR und RDS-Status). RT-Scrolling und Encoderbedienung/volle Radiofunktionen sind noch nicht vollständig integriert bzw. verifiziert. Compile-Erfolg allein bestätigt keine Hardwarefunktion.



## Buildstatus – Run 41367291150 (2026-11-02)

- Nutzer hat den manuellen Compile-only-Run geteilt: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41367291150
- Branch: `dev/first-build-preparation`; getesteter Commit: `4e31ef4ca13c20337e095ce77e238a983bf41b26`.
- Beide Jobs erfolgreich: `Compile-only / esp32s3-qspi` und `Compile-only / esp32s3-ospi`. Alle angezeigten Schritte einschließlich `Compile ATS Mini OS scaffold` und `Preserve compile report` waren erfolgreich.
- Damit sind die vorher dokumentierten Änderungen für Hardware-Audio-Mute an GPIO3 und Backlight-PWM an GPIO38 in diesem Compile-Lauf enthalten und kompilieren in beiden Profilen.
- Artefakte sind Compile-Reports, keine bestätigten flashbaren Firmware-Images: `ats-mini-os-compile-report-esp32s3-ospi` und `ats-mini-os-compile-report-esp32s3-qspi`; je 46,543,028 Bytes, erstellt am 2026-11-02, Ablauf 2026-11-16.
- Grenzen: nur Kompilierung bestätigt. Keine physischen Tests für Audio-Mute, Verstärker, Backlight, Display oder Radio; nichts geflasht und Bootloader/Partitionslayout nicht geändert.
- Nächster Schritt: späterer, kontrollierter Test am ATS Mini V4 nach ausdrücklicher Freigabe; zuerst die sichere Teststrategie und Image-/Partitionskompatibilität separat prüfen. Nutzer führt alle Workflows selbst aus.


## Encoder-Event-Queue-Korrektur nach Run 41367291150 (2026-11-02)

- Bei der Codeprüfung von `EncoderInput.cpp` wurde ein konkreter Fehler gefunden: Der bisherige einzelne `pendingEvent_`-Speicher konnte ein zuvor erkanntes Ereignis überschreiben, wenn innerhalb eines `update()`-Durchlaufs beispielsweise eine Drehung und ein Tastendruckereignis entstanden.
- Korrektur auf `dev/first-build-preparation`: feste Ringwarteschlange mit 8 `EncoderEvent)-Einträgen. `poll()` entnimmt Ereignisse FIFO; beim vollen Puffer wird das neueste Ereignis verworfen und eine serielle Diagnose ausgegeben, statt ein wartendes Ereignis zu überschreiben.
- Header-Commit: `e487dde375422df765602abc5d26850e1bfb63db` – [EncoderInput.h](https://github.com/13HN2709/ATS_Mini_-OS/blob/dev/first-build-preparation/firmware/ats-mini-os/src/input/EncoderInput.h).
- Implementierungs-Commit: `55e8b0e4bf1efe974ba24d1ee315469acc64a7f4` – [EncoderInput.cpp](https://github.com/13HN2709/ATS_Mini_-OS/blob/dev/first-build-preparation/firmware/ats-mini-os/src/input/EncoderInput.cpp).
- Noch nicht kompiliert; der vorherige grüne Run `41367291150` enthält diese beiden Commits nicht. Nutzer muss den manuellen Compile-only-Workflow erneut starten und QSPI/OSPI kontrollieren.
- Danach bleibt der praktische Encoder-Test am Gerät offen: Drehrichtung, Frequenzschritt 100 kHz, Grenzüberlauf 87,5/108,0 MHz, kurzer Tastendruck, langer Tastendruck und Anzeige-/Radio-Konsistenz. Die aktuelle Firmware ordnet kurzen/lange Tastendruck-Ereignisse bislang nur seriellen Diagnosemeldungen zu; Menüaktionen sind nicht implementiert.
- Grenzen unverändert: keine Actions durch den Assistenten gestartet, nichts geflasht, Bootloader/Partitionen und bestehende Firmware-Repositories unangetastet.


## Benannte OSPI-DE-/QSPI-DE-Artefakte (2026-11-02)

- Nutzerwunsch: getrennte deutsche Builds für OSPI und QSPI mit klaren Dateinamen im Stil `ATS-Mini-OS.ino.bin`, dazu Quellcode-ZIP; künftig nach geprüfter Kompatibilität auch eine echte OTA-Datei.
- Workflow-Commit auf `dev/first-build-preparation`: `c3f99a2360691128e5289bd6414326cff3a1b6a0` – [Workflow ansehen](https://github.com/13HN2709/ATS_Mini_-OS/commit/c3f99a2360691128e5289bd6414326cff3a1b6a0).
- Der manuelle Workflow verwendet nun `arduino-cli compile --export-binaries`, benennt das jeweilige App-Image `ATS-Mini-OS-OSPI-DE.ino.bin` bzw. `ATS-Mini-OS-QSPI-DE.ino.bin` und packt den Firmware-Projektordner als `ATS-Mini-OS-Source-Code-OSPI-DE.zip` bzw. `ATS-Mini-OS-Source-Code-QSPI-DE.zip` ein. Die Dateien werden in getrennten GitHub-Artefakten mit 14 Tagen Aufbewahrung bereitgestellt; ein Build-NOTEs-Text weist auf Grenzen hin.
- Wichtig: Diese Workflow-Änderung ist noch nicht durch einen neuen Run verifiziert. Der Nutzer muss den manuellen Workflow selbst starten; der Assistent startet keine Actions.
- OTA-Datei absichtlich noch nicht erzeugt: Vorher müssen OTA-Updatepfad, App-Binärformat und Partition-/Bootloader-Kompatibilität geprüft werden. Das bisherige 8-MiB-Compile-Profil ist kein Freigabenachweis für das 16-MiB-Gerät; nichts flashen.
- Vorhandene Build-Reports sind Diagnosematerial, nicht mit freigegebenen Firmware-Images gleichzusetzen. Compile-Erfolg beweist keine Hardwarefunktion.


## Build-Artefakt-Diagnose – 2026-11-02

- GitHub Actions Run: https://github.com/13HN2709/ATS_Mini_-OS/actions/runs/41369282686
- Beide Profile (esp32s3-qspi und esp32s3-ospi) haben den eigentlichen Compile-Schritt erfolgreich abgeschlossen (Sketch ca. 395 KB QSPI / 397 KB OSPI; diese Werte sind nur die gemeldete Sketchgröße).
- Fehler danach: Der Artefakt-Schritt suchte firmware/ats-mini-os/ats-mini-os.ino.bin, fand dort aber keine .bin; deshalb schlugen die beiden Jobs beim Vorbereiten/Upload der Artefakte fehl und es wurden keine Artefakte veröffentlicht.
- Korrektur auf dev/first-build-preparation: Commit 25f3ecc6a282aeffd403b800fab397f78a0b0d8e ändert die Ausgabe auf einen deterministischen --output-dir pro Profil (build/esp32s3-qspi bzw. build/esp32s3-ospi) und lässt den Artefakt-Schritt dort nach ats-mini-os.ino.bin suchen.
- Nächster Schritt: Nutzer startet manuell einen neuen Compile-only-Run und sendet den Link zur Prüfung. Assistent startet keine Workflows.
- Noch nicht bestätigt: erfolgreicher Export der .ino.bin, Source-Code-ZIPs, Hardwareverhalten, Flash-/Partitionskompatibilität und OTA-Verfahren. Keine Flashfreigabe; derzeitige Build-Option enthält weiterhin FlashSize=8M, während das Gerät 16 MB hat. Keine Firmware flashen.

## Artefaktgröße und schlanke Ausgabe – 2026-11-02

- Run `41369997620` auf `dev/first-build-preparation` ist für QSPI und OSPI vollständig erfolgreich.
- Artefakte enthielten dennoch rund 54,3 MiB, weil der komplette Quellbaum und Build-Ordner unter `compile-report/` kopiert wurden. Das war unnötig und wurde entfernt.
- Workflow-Änderungen auf Dev: `131411816bc58f19ca0616f470617cef044f1331` entfernt `compile-report/` und nimmt stattdessen einen kompakten `COMPILE-LOG.txt` auf; `a1f18297f0d74a419268a15bcd4f7cf0864c3543` aktiviert `pipefail`, damit ein echter Compilerfehler nicht durch `tee` verborgen wird.
- Erwarteter Inhalt des nächsten Artefakts: genau vier Dateien – benannte `.ino.bin`, benanntes Source-Code-ZIP, `BUILD-NOTES.txt` und `COMPILE-LOG.txt`.
- Nutzer startet den nächsten Workflow manuell. Kein Workflow durch den Assistenten auslösen.
- Keine Flashfreigabe: Firmware noch nicht auf Hardware getestet; `FlashSize=8M` gegenüber 16-MB-Gerät sowie Partitions-/Bootloader-/OTA-Kompatibilität sind offen.

## Erstflash-Vorbereitung: On-Device-Encoderdiagnose – 2026-11-02

- Zweck: Der Nutzer hat keinen PC/Serial Monitor zur Verfügung. Daher zeigt die Testanzeige nun den letzten Encoder-Eingang direkt am Display: `ENC:CW`, `ENC:CCW`, `ENC:SHORT` oder `ENC:LONG`, plus laufenden `EVENTS`-Zähler.
- Änderung auf `dev/first-build-preparation`: `firmware/ats-mini-os/ats-mini-os.ino` Commit `9c691031d6d92c5b49c9384c72fab6f1d79e5dd3`; `DisplayDriver.h` Commit `434dd23d5a6ae21c65b77c1120beeda420a2be03`; `DisplayDriver.cpp` Commit `f57e41a2dc31d001b2e9424c9ace7603b01918b8`.
- Die Änderung ist noch nicht kompiliert oder auf Hardware geprüft. Nutzer startet den nächsten Compile-only-Workflow manuell; Assistent löst keine Runs aus.
- Der Workflow wurde für kleinere Artefakte geändert: Commit `131411816bc58f19ca0616f470617cef044f1331` entfernt den kompletten `compile-report`-Ordner und nimmt stattdessen `COMPILE-LOG.txt` auf; Commit `a1f18297f0d74a419268a15bcd4f7cf0864c3543` aktiviert `pipefail` beim gleichzeitigen Speichern des Compilerlogs.
- Erwarteter Inhalt je Artefakt: `ATS-Mini-OS-OSPI-DE.ino.bin` oder `ATS-Mini-OS-QSPI-DE.ino.bin`, das passende `ATS-Mini-OS-Source-Code-*.zip`, `BUILD-NOTES.txt` und `COMPILE-LOG.txt`.
- Kein Flash-Go. Weiter offen: reale Hardwareprüfung, GPIO/Encoder-/Radio-/Displayverhalten, 8-MB-Buildkonfiguration gegen 16-MB-Gerät sowie Partitionen, Bootloader-/Bootmanager-Kompatibilität und OTA-Verfahren.

## Handover – Alpha-Meilenstein: Encoder, Radio/RDS und Build-Dateinamen (2026-11-02)

### Vom Nutzer am ATS Mini V4 bestätigter Hardwarestand
- **Encoder funktioniert jetzt**; die zuletzt eingespielte interruptbasierte Erfassung mit Beschleunigung hat das zuvor gemeldete Geschwindigkeitsproblem behoben. Hardwareergebnis vom Nutzer bestätigt.
- Radio startet unmittelbar mit Musik auf **95,400 MHz**; die vorherige manuelle Encoderbewegung zum „Aufwecken“ des Empfangs ist nicht mehr nötig.
- **RDS läuft stabil**: Sendername/Program Service (PS) und Radiotext (RT) flackern nicht mehr.
- Der Nutzer bewertet das Projekt als auf dem Weg zur **Alpha Stage**. Dies ist ein Entwicklungsmeilenstein, kein offizielles Release.

### Encoder-Änderungen, die dem erfolgreichen Test vorausgingen
- Commit `1d180464236ab73c2f321f6500b9a66495451b8d`: Encoder-Erfassung auf Interrupts für beide Pins GPIO2/GPIO1 umgestellt; vollständige Quadratur-Rastungen werden erfasst.
- Commit `e1efb792e52ec23f971f2bdd88ee13d950f51e25`: Header auf ISR-basierte Eingabe und Beschleunigungszustand angepasst.
- Commit `1f0eb43f206e6b9ee41049ce63c0387f613c930d`: erkannte Rastungen eines Ereignis-Bursts zusammengefasst und Beschleunigungsrampe an die ATS-Mini-Dev-Referenz angelehnt.
- Der Nutzer hat anschließend bestätigt: **„Encoder klappt“**. Damit ist der praktische Encoder-Test nicht mehr offen; künftige Änderungen daran trotzdem schrittweise testen.

### Build-Dateinamen mit Nummern bis zum Release
- Workflow-Datei: `.github/workflows/ats-mini-os-first-compile.yml`.
- Commit `399b6d21dfd47a09acdd422599b53b9cc3cd8924`: Firmware-BIN erhält nun Workflow-Laufnummer und Run-Versuch, z. B. `ATS-Mini-OS-OSPI-DE-Build-58-1.ino.bin` und `ATS-Mini-OS-QSPI-DE-Build-58-1.ino.bin`.
- Änderung wurde anschließend auf GitHub zurückgelesen und verifiziert; **noch kein neuer Build durch diese Änderung bestätigt**.
- Diese nummerierten Namen gelten für die Entwicklungs-/Alpha-Phase. Zum offiziellen Release später auf feste Versionsnummern umstellen.
- Workflow bleibt manuell (`workflow_dispatch)); der Nutzer startet jeden Build selbst. Der Assistent startet keine GitHub Actions.

### Nächste Arbeit – Fortsetzung am nächsten Tag
1. **Neues Radio-Layout modular implementieren**, ohne den funktionierenden Radio-/RDS-Kern unnötig anzufassen:
   - oben links gut lesbare Frequenz mit zwei Nachkommastellen, z. B. `95.40 MHz`;
   - oben rechts Statussymbole in dieser Reihenfolge: Bluetooth, WLAN, Akku ganz rechts;
   - horizontale Trennlinie unter der Kopfzeile;
   - Senderlogo links (ungefähr ein Drittel der Displaybreite), Sendername rechts daneben;
   - Radiotext darunter;
   - ganz unten RDS-PTY/Genre, z. B. `CURRENT AFFAIRS`, `NEWS`, `POP MUSIC`;
   - dunkler Hintergrund, wichtige Details neon-grün (`0x07E0`), Statusanzeigen cyan, lesbare Schrift.
2. **RadioDNS** für senderbezogene Metadaten/Logos weiterentwickeln: PI-Code/Frequenz/Länderkennung gemäß Spezifikation, begrenzter Download, lokaler Logo-Cache und sinnvoller Fallback, wenn kein Treffer oder kein Internet verfügbar ist.
3. Danach gezielte Alpha-Tests und Fehlerkorrekturen; bestehende funktionierende Empfangs-, Encoder- und RDS-Funktionen bei jeder Änderung erhalten.

### Verbindliche Schutz- und Arbeitsregeln
- Das bevorzugte **TEF6686-Layout bleibt unverändert**; neue Layoutarbeit darf es nicht beschädigen.
- Hardware-/GPIO-Referenz bleibt `13HN2709/ats-mini-dev`; Referenz nur lesen.
- Nutzer startet Builds manuell. Vor dem nächsten Build Änderungen erst fertigstellen und dann den Nutzer starten lassen.
- Kein Flashen durch den Assistenten; keine Änderungen an Bootloader, Bootmanager, Recovery, Partitionslayout oder den bestehenden APP-Slots ohne separate ausdrückliche Freigabe.
- Gerät ist ATS Mini V4 mit 16 MB Flash; die Workflow-Compile-Profile zeigen derzeit `FlashSize=8M` und sind **keine Freigabe zum Flashen**. Bestehendes Backup und Bootloader unangetastet lassen.
- Der letzte Encoder-Hardwaretest ist positiv, aber neue Layout-/RadioDNS-Änderungen sind noch nicht gebaut oder auf Hardware getestet.

## ToDo nach Fertigstellung des Hauptsystems – RT-Text-Tuning (2026-11-03)

- Der Nutzer vermutet, dass die längere Zeit bis zum ersten Radiotext (RT) möglicherweise nicht durch die RDS-Verarbeitung verursacht wird: Der Empfang zuhause scheint sich von dem am Arbeitsplatz zu unterscheiden. Daher **keine vorschnellen Änderungen** an RDS-Polling, SI4735-Konfiguration oder RT-Segmentverarbeitung vornehmen.
- **RT-Text-Tuning ausdrücklich zurückstellen, bis das Hauptsystem steht.** Zuerst das modulare Radio-Layout und die grundlegenden Systemfunktionen fertigstellen; danach unter vergleichbaren Empfangsbedingungen testen und bei Bedarf die RT-Erkennung/-Zusammensetzung mit ATS-Mini-DEV vergleichen.
- Beim späteren Test PS (Sendername) und RT (Radiotext) getrennt beobachten: Zeit bis zum ersten vollständigen RT, ob Text stückweise aufgebaut wird und ob sich das Verhalten zwischen Standort/Empfang unterscheidet.
- Der derzeit bestätigte Stand bleibt: PS/RT flackern nicht; Nutzer meldet stabiles RDS. Es gibt **noch keinen bestätigten RT-Fehler** und zu diesem ToDo wurden keine Codeänderungen oder Builds vorgenommen.
- Keine GitHub Actions durch den Assistenten starten; der Nutzer führt Builds manuell aus. Keine Flash-, Bootloader- oder Partitionsänderungen.

## Neues RadioDNS-Testlayout und WLAN-Dauerverbindung (2026-11-03)

- Nutzerentscheidung: Während der ersten RadioDNS-Logo-Tests WLAN beim Systemstart einschalten, mit dem aktuell konfigurierten WLAN verbinden und eingeschaltet lassen. Kein WLAN-Aus-/Einschalten nach jedem Senderwechsel in dieser Testphase.
- Der bestehende `WifiClient::begin()` wird bereits in `setup()` aufgerufen und nutzt Station-Modus, Auto-Reconnect und nicht persistente Laufzeitkonfiguration. Das neue Layout zeigt den Verbindungszustand an; WLAN bleibt im Hauptloop aktiv.
- Layoutänderung auf `dev/first-build-preparation`: Kopfzeile mit Frequenz (zwei Nachkommastellen), BT-/WLAN-Indikatoren und Batterie-Umriss, Trennlinie, reservierter Senderlogo-Bereich links, Sendername und Radiotext rechts, PTY-Genre unten. Dunkler Hintergrund, Neon-Grün und Cyan.
- Das Logo-Feld ist aktuell **nur ein Platzhalter**. RadioDNS-Auflösung, Logo-Download, Bilddecoder und lokaler Cache sind noch nicht implementiert. Der Batterie-Indikator zeigt bewusst keinen Prozentwert, da ein belastbarer Akkustand-Pin/-Treiber noch nicht identifiziert ist; BT ist vorerst ein Layout-Indikator, keine bestätigte Bluetooth-Verbindungsanzeige.
- Die Test-WLAN-Zugangsdaten liegen derzeit in einer privaten Testkonfiguration; vor einem öffentlichen Release müssen sie aus dem Quellcode entfernt und durch eine sichere Einstellungs-/Provisionierungslösung ersetzt werden.
- Noch nicht kompiliert oder am Gerät getestet. Nutzer startet den manuellen Compile-Workflow selbst. Keine GitHub Actions gestartet, nichts geflasht; keine Bootloader-/Partitionsänderungen.


## Storage Discovery für RadioDNS-Cache (2026-11-03)

- Nutzer hat die Umsetzung einer adressunabhängigen Speichererkennung ausdrücklich freigegeben.
- Neu hinzugefügt: `firmware/ats-mini-os/src/storage/StorageDiscovery.h/.cpp`.
- Die Routine liest die Laufzeit-Partitionstabelle mit der ESP-IDF-Partitions-API und prüft bekannte Datenpartitionen. In der aktuellen ATS-Mini-OS-Partitionstabelle ist `littlefs` die unterstützte Dateisystempartition; NVS, OTA-Metadaten, Settings und Coredump werden nicht als Dateisystem behandelt.
- LittleFS wird anhand des Partitionslabels `littlefs` eingebunden, nicht über eine fest codierte Flash-Adresse. Der Mount verwendet `formatOnFail=false`.
- Die Suche läuft lesend und erkennt `Schedules.json`, `schedules.bin`, `memories.json` und `splash.png` auch in Unterordnern (begrenzt auf vier Ebenen).
- Erst nach Fund einer Ankerdatei wird `/RDS` angelegt bzw. ein vorhandener Ordner wiederverwendet. Wenn kein Anker gefunden wird, bleibt der RadioDNS-Cache deaktiviert und es wird kein Ordner angelegt.
- `RadioDnsCache::begin()` nutzt jetzt `StorageDiscovery`; die übrigen Cache-Zugriffe verwenden weiterhin LittleFS auf dem ausgewählten Dateisystem.
- Wichtig: Die Implementierung scannt die Laufzeit-Partitionstabelle, mountet in dieser Version aber nur die im aktuellen Layout vorhandene LittleFS-Partition. FAT/FFat-Unterstützung ist noch nicht implementiert. Die Routine durchsucht keine App-Slots oder rohen Flashbereiche nach Dateien.
- Geänderte/angelegte Dateien: `firmware/ats-mini-os/src/storage/StorageDiscovery.h`, `firmware/ats-mini-os/src/storage/StorageDiscovery.cpp`, `firmware/ats-mini-os/src/radiodns/RadioDnsCache.cpp`.
- Status: Änderungen auf `dev/first-build-preparation` committed; **noch nicht kompiliert und nicht am Gerät getestet**. Keine GitHub Actions gestartet, nichts geflasht; Partitionstabelle, Bootloader und bestehende Slots unverändert.
- Nächster Schritt: statische Schnittstellen-/Compile-Prüfung, insbesondere ESP32-Arduino-3.3.12-Kompatibilität von `File::path()`, LittleFS-Mount mit Partitionslabel und Iteratorfreigabe; anschließend manueller Compile-Test durch den Nutzer. Bei erfolgreicher Suche sollte der Diagnose-Endpunkt Partitionlabel und gefundene Ankerdatei anzeigen.


## Eingrenzung der Storage Discovery auf zwei LittleFS-Adressen (2026-11-03)

- Nutzer präzisierte: ausschließlich die beiden bekannten LittleFS-Startadressen prüfen, nicht andere Partitionen oder beliebige Flashbereiche.
- Aktuelle ATS Mini V4 Partitionstabelle: LittleFS bei `0xA00000`.
- Read-only Referenz `13HN2709/ats-mini-dev/partitions.csv`: LittleFS bei `0x610000`, Größe `0x1D0000` (8-MiB-Layout).
- `StorageDiscovery.cpp` akzeptiert daher nur Partitionen, die die Runtime-Partitionstabelle als DATA-Partition mit Label `littlefs` meldet und deren Startadresse exakt `0xA00000` oder `0x610000` ist.
- Die Routine mountet niemals blind die rohe Adresse `0x610000`: Im dokumentierten 16-MB-ATS-Mini-V4-Layout liegt diese Adresse innerhalb von `app1`; in diesem Fall ist sie keine LittleFS-Partition und wird ignoriert.
- Die bekannten Ankerdateien werden weiterhin nur innerhalb des gültig gemounteten LittleFS gesucht. `/RDS` wird nur nach einem Fund angelegt; Mount erfolgt mit `formatOnFail=false`.
- Noch nicht kompiliert oder auf dem Gerät getestet. Keine GitHub Actions gestartet, nichts geflasht; keine Partitionstabelle, Bootloader- oder bestehenden Daten geändert.


## Storage Discovery: Reihenfolge der beiden Layouts präzisiert (2026-11-03)

- Nutzer hat die beiden Adressen erklärt: `0xA00000` ist LittleFS im Dual-Boot-Manager 4.2.2; `0x610000` ist LittleFS im Stock-/ATS-Mini-Dev-Layout ohne Dual Boot. Es handelt sich um alternative Layouts, nicht um zwei gleichzeitig zu verwendende Dateisysteme.
- `StorageDiscovery.cpp` prüft jetzt ausdrücklich zuerst `0xA00000`, danach als Fallback `0x610000`.
- Eine Adresse wird nur verwendet, wenn die Laufzeit-Partitionstabelle dort tatsächlich eine DATA-Partition mit Label `littlefs` ausweist. Kein Mount auf rohe Flash-Adressen; wichtig, weil `0x610000` beim Dual-Boot-Layout in einen App-Bereich fallen kann.
- Innerhalb des gemounteten Dateisystems wird nach `Schedules.json`, `schedules.bin`, `memories.json` oder `splash.png` gesucht. Erst wenn mindestens eine davon existiert, wird `/RDS` angelegt bzw. wiederverwendet und der Cache aktiviert. Wenn keine Ankerdatei gefunden wird, bleibt der persistente Logo-Cache deaktiviert.
- LittleFS wird mit `formatOnFail=false` eingebunden. Keine Formatierung, kein Löschen, keine Änderung an Partitionsgrößen oder Bootloader.
- Commit: `7ed519f5492a81ce711aa1bc1ca629ce1213f5ab`.
- Status: Codeänderung committed, noch nicht neu kompiliert oder am Gerät getestet. Nutzer startet den manuellen Compile-only-Workflow selbst; keine GitHub Actions durch den Assistenten gestartet und nichts geflasht.

## Referenz: offizieller GNBD ATS Mini DualBoot (2026-11-03)

- Vom Nutzer angegebener Release-Link für den tatsächlich verwendeten DualBoot Manager 4.2.2:
  https://github.com/GNBD/ats-mini-dualboot/releases#release-v4.2.2-dualboot
- Versionsgebundene README v4.2.2:
  https://github.com/GNBD/ats-mini-dualboot/blob/v4.2.2-dualboot/ats-mini-recovery-DBFM/README.md
- In der dokumentierten Partitionstabelle von v4.2.2 steht `littlefs` bei Offset `0xA00000`, Größe 5.58 MiB. Die gleiche LittleFS-Startadresse ist auch in der geprüften README von v4.3.5 dokumentiert:
  https://github.com/GNBD/ats-mini-dualboot/blob/v4.3.5-dualboot/ats-mini-recovery-DBFM/README.md
- v4.2.2 Release-Datum laut GitHub: 2026-10-30. Die Release-Beschreibung kennzeichnet es als Test-Build.
- Für die ATS-Mini-OS Storage Discovery bleibt `0xA00000` die erste Kandidatenadresse für das DBFM-Layout. `0x610000` bleibt ausschließlich der alternative Kandidat für das Stock-/ATS-Mini-Dev-Layout. Es sind alternative Layouts.
- Sicherheitsregel bleibt unverändert: Nur verwenden, wenn die aktive Laufzeit-Partitionstabelle tatsächlich eine DATA-Partition mit Label `littlefs` an genau der Kandidatenadresse meldet und eine bekannte Ankerdatei gefunden wird. Niemals rohe Flash-Adressen mounten; nicht formatieren, löschen oder Partitionen/Bootloader ändern.
- Diese Links bei späteren DBFM-Updates erneut prüfen, insbesondere die versionsgebundene README/Partitionstabelle. Eine bestätigte Adresse in der Dokumentation ersetzt nicht den späteren Gerätetest.


## RadioDNS-Test: fester Deutschland-Fallback (2026-11-03)

- Nutzerfreigabe: Für den aktuellen Logo-Test vorübergehend Deutschland fest einstellen; noch kein Settings-Menü.
- Wenn ein gültiger RDS-ECC empfangen wurde, bleibt der daraus gebildete GCC vorrangig.
- Wenn ECC fehlt, wird für den Test nur bei PI-Country-Nibble `D` (Deutschland) der GCC `DE0` verwendet. `DE0` folgt der GCC-Konstruktion aus PI-Ländercode `D` und deutschem ECC `E0`; die Fallback-Nutzung wird in Diagnose und RadioDNS-Log separat angezeigt.
- Bei PI-Ländercode ungleich `D` wird der feste DE-Test-Fallback nicht angewendet, damit grenzüberschreitende Sender nicht blind falsch identifiziert werden. Eine vollständige CountryCodeResolver-Tabelle und das spätere Settings-Menü bleiben separate Aufgaben.
- Status-Endpunkt `/radiodns-status` zeigt nun PI, ECC-Status, GCC, GCC-Quelle, FQDN und Identity-Status. RadioDNS-Log nennt ebenfalls die GCC-Quelle.
- Sobald die Identität gültig ist, läuft der vorhandene Ablauf unverändert weiter: SRV-Abfrage, Abruf von `SI.xml`, Logo-URL-Suche und Logo-Download in den bestehenden `/RDS`-Cache.
- Änderungen auf `dev/first-build-preparation`; noch nicht kompiliert oder am Gerät getestet. Nutzer startet den Compile-only-Workflow selbst. Keine GitHub Actions gestartet und nichts geflasht; keine Änderungen an Bootloader, Partitionen oder vorhandenen Daten.

## Wetter-Referenz: PV2KSM ATS-Mini r28 (2026-11-03)

- Vom Nutzer als Referenz für die spätere Wetterfunktion vorgemerkt:
  - Repository: https://github.com/PV2KSM/ats-mini-pv2ksm
  - Release: https://github.com/PV2KSM/ats-mini-pv2ksm/releases/tag/v2.45-PV2KSM-r28
  - Tag/Branch: `v2.45-PV2KSM-r28` / `pv2ksm-r28`
- Das Release beschreibt eine inoffizielle Community-Modifikation auf Basis von ATS-Mini v2.45. Die Release-Notizen nennen Wetteransichten für aktuelle Bedingungen, kommende Tage, kommende Stunden, METAR und Regenradar.
- Genannte Wetterdatenquellen: Open-Meteo (Wetter), aviationweather.gov (METAR) und RainViewer (Regenradar). Laut Release werden Wetterdaten nur abgerufen, während die Wetteransicht geöffnet ist.
- Laut Release wurde diese Wetterfunktion auf realer ATS-Mini-OSPI-Hardware getestet; QSPI und LilyGO T-Embed wurden dort nur kompiliert, nicht auf Hardware getestet.
- Verwendung für ATS Mini OS: **nur als lesende technische Referenz** untersuchen. Vor Übernahme einzelner Ideen/Codepfade Schnittstellen, Netzwerkverhalten, Speicherbedarf, Lizenz/Attribution und Nutzungsbedingungen der Datenanbieter prüfen. Das Release weist insbesondere auf Einschränkungen bei nicht-kommerzieller Nutzung von Open-Meteo und auf eigene Bedingungen von RainViewer hin.
- Noch keine Übernahme von Code, keine Änderung am Referenz-Repository und keine Änderung an der ATS Mini OS-Wetterimplementierung allein durch das Vormerken dieser Quelle. Die bereits dokumentierte Wetterplanung und die bestehenden Sicherheitsregeln bleiben bestehen.

## RadioDNS: Diagnosemeldungen in die abrufbare Logdatei schreiben (2026-11-03)

- Nutzer meldet weiterhin kein Senderlogo. Status bestätigt gültige Testidentität (z. B. `PI=D399`, `GCC=DE0`, `Identity status=3`), verbundenes WLAN und erkannte LittleFS-Partition; Log bestätigt wiederholt `lookup started`.
- Fehlerursache noch nicht bekannt: Der bisherige Code schrieb wichtige Ergebnisse/Fehler (SRV-Lookup fehlgeschlagen, SI.xml HTTP-Fehler, Logo-Download-Fehler und Cache-Schreibfehler) nur über `Serial.printf/println`. Die über den Webdiagnose-Endpunkt gespeicherte `/RDS/radiodns-debug.log` zeigte diese entscheidenden Ergebnisse deshalb nicht zuverlässig. Nutzer arbeitet ohne PC/Serial Monitor.
- Änderung in `firmware/ats-mini-os/src/radiodns/RadioDnsClient.cpp`: Cache-Hits, fehlgeschlagene SRV-Abfrage samt konfiguriertem DNS-Server, gefundener Metadatenserver, SI.xml-Fehler inkl. Status/URL, Logo-Download-Fehler inkl. Status/URL und erfolgreicher/fehlgeschlagener Cache-Schreibvorgang werden jetzt über `logRadioDns()` auch in die abrufbare Debug-Logdatei geschrieben. Fehler beim Anlegen/Starten des Lookup-Tasks ebenfalls in der Datei protokolliert.
- Commit: https://github.com/13HN2709/ATS_Mini_-OS/commit/59b2fd69128fc6d05cb8bf12d7be87ac2a0176ef
- Status: **noch nicht kompiliert oder am Gerät getestet**. Nutzer startet Compile-only-Workflow selbst; keine GitHub Actions gestartet und nichts geflasht. Bootloader, Partitionstabelle und vorhandene Dateien wurden nicht verändert.
- Nächster Schritt nach dem manuellen Compile-only-Test: Nutzer testet mit OSPI-DE, wartet auf `lookup started`, ruft danach `/radiodns-log` ab und sendet den neuen Log. Dann lässt sich anhand der tatsächlichen SRV-/HTTP-/Logo-Meldung gezielt weiterarbeiten. Recovery-Dateiansicht ist kein verlässlicher Beleg für das Vorhandensein/Fehlen von `/RDS`, sofern LittleFS dort nicht explizit angezeigt wird.

## RadioDNS: DNS-SRV-Fehler unterscheiden (2026-11-03)

- Nutzer hat das Log vom Gerät bereitgestellt. Entscheidend war: `SRV lookup failed: no _radioepg._tcp SRV record ... DNS server=10.186.154.24`. Das war bisher eine irreführend pauschale Meldung: Der Code hat Timeout, DNS-Antwortfehler und tatsächlich fehlende SRV-Antwort nicht unterschieden.
- `firmware/ats-mini-os/src/radiodns/RadioDnsClient.cpp` protokolliert jetzt getrennte Ergebnisse: kein DNS-Server, UDP-Socket-/Sendeproblem, Timeout, ungültige/abgeschnittene Antwort, NXDOMAIN, SERVFAIL, REFUSED, sonstiger DNS-Fehler und erfolgreiche Antwort ohne SRV-Record. Bei Fehlern werden FQDN, vollständiger SRV-Qname, DNS-Server, RCODE und Answer-Anzahl protokolliert; bei Erfolg der gefundene Metadatenserver und Port.
- Fallback-Pfad in `RadioCore.cpp` geprüft: gültiger empfangener ECC hat Vorrang; ohne ECC wird der temporäre Testwert `DE0` nur bei PI-Ländercode-Nibble `D` verwendet. Das ist weiterhin ein ausdrücklich temporärer Test-Fallback und kein Beweis, dass die Senderidentität korrekt ist.
- Commits:
  - DNS-Ergebnisse detaillierter protokollieren: https://github.com/13HN2709/ATS_Mini_-OS/commit/e021cace4cc991bea807b843b4b4ac1236cdc213
  - DNS-Answer-Anzahl auch bei DNS-Fehlerantworten erfassen: https://github.com/13HN2709/ATS_Mini_-OS/commit/d60df0fbfe903491084f09c4055292e18462b45a
- Noch nicht kompiliert oder am Gerät getestet. Nutzer startet den manuellen Compile-only-Workflow selbst. Keine Actions gestartet, nichts geflasht und keine Bootloader-/Partition-/Datenänderung vorgenommen.
- Nächster Schritt: manueller Compile-only-Build, danach Gerätetest und Abruf von `/radiodns-log`. Die neue Meldung soll zeigen, ob der Resolver ein echtes NXDOMAIN, einen Timeout oder einen anderen DNS-Fehler liefert. Erst dann die Ursache beheben.



## RadioDNS: CNAME-Auflösung und Logo-Test nur im RAM (2026-11-03)

- Nutzerfreigabe: Für die Fehlersuche wird das dauerhafte Speichern von SI.xml-Metadaten und heruntergeladenen Senderlogos vorübergehend deaktiviert. Der Ablauf soll erst RadioDNS korrekt auflösen, SI.xml und Logo laden und das Logo direkt anzeigen. Die Ursache für das spätere Speichern wird getrennt untersucht.
- DNS-Reihenfolge korrigiert: Zuerst wird der RadioDNS-FQDN (z. B. `09540.d399.de0.fm.radiodns.org`) als CNAME abgefragt. Der zurückgegebene authoritative FQDN wird anschließend für `_radioepg._tcp.<authoritative-FQDN>` als SRV-Abfrage verwendet. CNAME-/SRV-Ergebnisse, DNS-Server, RCODE und Antwortanzahl werden im bestehenden RadioDNS-Debug-Log protokolliert. Das entspricht dem RadioDNS-Core-Lookup-Verfahren aus ETSI TS 103 270; ein fehlender CNAME wird nun als fehlender angefragter DNS-Record gemeldet.
- Die SI.xml-Antwort bleibt nur im temporären Arbeitsspeicher; `saveMetadata()` wird in diesem Diagnosepfad nicht mehr aufgerufen. Heruntergeladene Logo-Bytes werden in PSRAM (mit Heap-Fallback) gehalten und per Mutex an den Display-Renderer übergeben. Das Display decodiert PNG/JPG direkt aus dem RAM-Puffer statt aus LittleFS.
- Der Diagnose-Log bleibt weiterhin persistent unter `/RDS/radiodns-debug.log`, damit der Nutzer ihn über `/radiodns-log` am Smartphone abrufen kann. Vorhandene Logo-/Metadatendateien werden weder gelesen noch überschrieben oder gelöscht. Die Cache-Schreibmethoden bleiben im Projekt erhalten, sind aber im aktuellen Testablauf nicht aktiv.
- Geänderte Dateien:
  - `firmware/ats-mini-os/src/radiodns/RadioDnsClient.h`
  - `firmware/ats-mini-os/src/radiodns/RadioDnsClient.cpp`
  - `firmware/ats-mini-os/src/display/DisplayDriver.h`
  - `firmware/ats-mini-os/src/display/DisplayDriver.cpp`
  - `firmware/ats-mini-os/ats-mini-os.ino`
- Änderungskette auf `dev/first-build-preparation`:
  - [RAM-Logo-Schnittstelle](https://github.com/13HN2709/ATS_Mini_-OS/commit/e782e703202dee463892c828df1e63575cfaa0c9)
  - [CNAME vor SRV, Logo nur im RAM](https://github.com/13HN2709/ATS_Mini_-OS/commit/3d6dc216cbd771ba4a9f1b5eee36e77cea3bf09e)
  - [Display rendert Logo aus RAM](https://github.com/13HN2709/ATS_Mini_-OS/commit/aa26a2adc1fe23b7dda9ea87944e89e2f1360470)
  - [RadioDNS-Client an Display übergeben](https://github.com/13HN2709/ATS_Mini_-OS/commit/26e7cfe0fc3d544874b3d207d99ae235898dfe0a)
  - [CNAME-Diagnose präzisiert](https://github.com/13HN2709/ATS_Mini_-OS/commit/8645f8e5ffa992673706d6ddef851ccb956d9d31)
- **Teststatus: noch nicht kompiliert und nicht auf dem Gerät geprüft.** Der Assistent hat keine GitHub Actions gestartet und nichts geflasht. Nutzer soll den manuellen Compile-only-Workflow ausführen; erst nach erfolgreichem Compile folgt der Test auf dem ATS Mini V4. Bootloader, Partitionstabelle und bestehende Dateien wurden nicht verändert.
- Beim Gerätetest bitte `/radiodns-log` nach einer neuen Suche abrufen. Gesucht werden die Meldungen `CNAME DNS lookup succeeded`, `SRV DNS lookup succeeded`, `SI.xml received` und `logo ready in RAM for display`. Wenn der Ablauf bis zum RAM-Logo reicht, prüfen wir danach, ob es sichtbar ist. Erst danach wird das Speicherproblem separat angegangen.


- Ergänzende Absicherung im selben Diagnosezweig: Die Logo-Endung wird nicht mehr aus der URL geraten, sondern anhand der tatsächlichen Datei-Signatur (JPEG/PNG) erkannt. So wird ein Logo mit URL-Query oder abweichender Endung nicht automatisch an den falschen Decoder übergeben. Commit: https://github.com/13HN2709/ATS_Mini_-OS/commit/cb73054d79e5f95f9206cb6d72a1311969d26ce0


## RadioDNS: modernes SPI-SRV vor Legacy-EPG testen (2026-11-03)

- Neues Geräteprotokoll: CNAME-Auflösung für Antenne Münster funktioniert und liefert `c276.radioplayer.org`. Die bisherige Abfrage `_radioepg._tcp.c276.radioplayer.org` endet mit NXDOMAIN. Daher erreicht der Ablauf SI.xml/Logo-Download noch nicht.
- Ursache wahrscheinlich im verwendeten Dienstpfad: RadioDNS SPI v3.2.1 definiert `_radiospi._tcp` für HTTPS; `_radioepg._tcp` ist der ältere HTTP-Pfad. Radioplayer hat außerdem mitgeteilt, dass sein Metadatenangebot seit Oktober 2023 ClientID-Authentifizierung verlangen kann.
- Änderung: zuerst `_radiospi._tcp.<authoritative-FQDN>` abfragen und bei Erfolg HTTPS für `/radiodns/spi/3.1/SI.xml` verwenden. Wenn der moderne Dienst nicht existiert, wird weiterhin `_radioepg._tcp` als Legacy-Fallback versucht. Beide Pfade protokollieren Dienstname, RCODE, Antwortanzahl und Zielserver. HTTP 401 wird als tatsächliches Authentifizierungsresultat protokolliert; es kann ClientID-Zugang durch Radioplayer erfordern.
- Geänderte Datei: `firmware/ats-mini-os/src/radiodns/RadioDnsClient.cpp`.
- Noch nicht kompiliert oder am Gerät getestet. Keine GitHub Actions gestartet, nichts geflasht; Bootloader, Partitionstabelle und bestehende Daten bleiben unverändert.


## FM-DX-Webserver Logoquelle als erster Testpfad (2026-11-03)

- Nutzerentscheidung: zunächst die Logoquelle des FM-DX-Webserver-Plugins verwenden, da sie bereits in der Android-App des Nutzers für den TEF6686 hilfreich ist.
- Plugin-Repository geprüft: `Highpoint2000/webserver-station-logos`. Das Plugin lädt Logos vom öffentlichen Verzeichnis `https://tef.noobish.eu/logos/`, verwendet Länderordner und PI-basierte Dateinamen; PNG/SVG sind die üblichen Formate. Für den ATS Mini OS wird SVG vorerst nicht verwendet, da der bestehende Anzeigeweg JPEG/PNG decodiert.
- Diagnoseimplementierung auf `dev/first-build-preparation`: bei deutscher Identität mit PI beginnend `D` und GCC-Endung `E0` wird zuerst im Länderordner `D` nach `<PI>.png`, `.jpg` und `.jpeg` gesucht. Bei Erfolg wird das Bild über dieselbe RAM-only Logoübergabe angezeigt. Bei Misserfolg läuft der bisherige RadioDNS-Pfad weiter.
- Es werden weiterhin keine Senderlogos oder SI.xml-Daten dauerhaft gespeichert. Das bestehende RadioDNS-Debug-Log bleibt für die Fehlersuche persistent.
- Die Länderordnerzuordnung ist absichtlich zunächst nur für den bestätigten deutschen Testfall umgesetzt; weitere Länder werden erst nach Prüfung der FM-DX-Ländercodeliste ergänzt.
- **Status: Code geändert, noch nicht kompiliert und nicht auf dem Gerät geprüft.** Keine GitHub Actions gestartet und nichts geflasht. Nutzer führt den manuellen Compile-only-Workflow aus; Bootloader, Partitionstabelle und vorhandene Dateien bleiben unangetastet.


## RadioDNS: Logo-Identität zwischen Download und Display angleichen (2026-11-03)

- Nutzer bestätigt: FM-DX-Logo-Download ist erfolgreich (HTTP 200, PNG mit 15.538 Bytes in RAM), aber auf dem Display bleibt NO LOGO.
- Ursache im Code gefunden: RadioDnsClient verwendet radio.radioDnsIdentity(), das bei fehlendem ECC für den deutschen Test-PI D399 korrekt den vorübergehenden GCC-Fallback DE0 bildet. DisplayDriver::render() hat die Identität dagegen erneut aus dem Snapshot aufgebaut und bei fehlendem ECC einen leeren GCC übergeben. Dadurch blieb logoKey leer und lockLogo() konnte die erfolgreich geladenen Bytes nie an den Renderer übergeben.
- Korrektur: Die bereits gebildete RadioDnsFmIdentity wird jetzt aus ats-mini-os.ino an DisplayDriver::render() übergeben. Das Display verwendet damit exakt dieselbe Identität und denselben Schlüssel wie der Downloader; es gibt keine doppelte, abweichende Identitätsbildung mehr.
- Geänderte Dateien: firmware/ats-mini-os/src/display/DisplayDriver.h, firmware/ats-mini-os/src/display/DisplayDriver.cpp, firmware/ats-mini-os/ats-mini-os.ino.
- Noch nicht kompiliert oder am Gerät getestet. Keine GitHub Actions gestartet, nichts geflasht. Der Nutzer führt wie vereinbart zuerst den manuellen Compile-only-Workflow aus. Bootloader, Partitionstabelle und vorhandene Dateien bleiben unverändert.


## Vollbild-Encoder-Menü – Vorabimplementierung (2026-11-03)

Auf Branch `dev/first-build-preparation` wurde ein erstes scrollbares Vollbild-Menü im TEF6686-inspirierten dunklen Stil ergänzt. Es verwendet die vorhandene Encoder-Eingabe und den vorhandenen LovyanGFX-Renderer; kein neuer Font- oder UI-Unterbau wurde hinzugefügt.

- Kurzer Encoder-Klick öffnet das Hauptmenü.
- Erste Zeile ist **Zurueck**; ausgewählt und angeklickt beendet sie das Menü.
- Darunter folgt **Lautstaerke**; Audio-Lautstärke ist mit dem SI4735 verbunden (0–63, Startwert 25).
- Weitere Hauptpunkte: Anzeige, Radio, WLAN und System. Sie öffnen jeweils einfache Listen-Untermenüs als vorbereitete Platzhalter, keine separaten Bildschirmdialoge.
- Drehen bewegt die Auswahl. In Audio > Lautstaerke wird durch Drehen der Wert verändert; kurzer Klick kehrt ins Hauptmenü zurück.
- Langer Klick schließt das Menü von jeder Menüebene.
- Noch nicht implementiert sind die Platzhalterfunktionen Helligkeit, Schrittweite, WLAN-Modus, Web Control und CPU-Profil.
- Keine GitHub Actions/Builds gestartet, kein Flashen. Erst manuell Compile-only ausführen und Ergebnisse prüfen.


## FM-DX-Logosuche: Kandidaten der Reihe nach prüfen (2026-11-03)

- Nutzerfreigabe: alle sinnvollen, vom bestehenden Renderer unterstützten Dateivarianten nacheinander abfragen und beim ersten verwendbaren Logo stoppen.
- In firmware/ats-mini-os/src/radiodns/RadioDnsClient.cpp probiert der deutsche Testpfad zuerst die bisherigen PI-only-Dateinamen mit .png, .jpg, .jpeg; anschließend bekannte PI-/Programmnamenvarianten:
  - D392_WDR3 (WDR 3)
  - D394_WDR4 (WDR 4)
  - D399_ANTENNEMUNSTER und die normalisierte Umlaut-Schreibvariante D399_ANTENNEMUENSTER (Antenne Münster)
- Für jede URL werden HTTP-Status und Ergebnis protokolliert. HTTP 200 führt nur dann zum Stopp, wenn die Antwort anhand der Signatur als PNG oder JPEG erkannt und erfolgreich in RAM übernommen wurde. Bei nicht unterstützten Bilddaten oder Fehlern wird der nächste Kandidat versucht; erst nach Ausschöpfen der Rasterkandidaten geht es mit RadioDNS weiter.
- SVG wird bewusst nicht als Erfolg gewertet, da der aktuelle Firmwarepfad nur PNG/JPEG decodiert. Die öffentliche Plugin-Dokumentation bestätigt PI_PROGRAMNAME-Dateinamen und PNG/SVG-Unterstützung auf Plugin-Seite; das bestätigt nicht, dass jede konkrete Kandidatendatei auf dem Server vorhanden ist.
- Änderungen auf dev/first-build-preparation, Commits:
  - Kandidatenfolge erweitert: eea42b2aa80b616d5a122f442f6f67329ddfbcce
  - Doppelte Kandidaten entfernt: 60223fbc70a0c8027f163b006c1bd0b349f08660
- Noch nicht kompiliert und nicht am Gerät geprüft. Der Assistent hat keine GitHub Actions gestartet und nichts geflasht. Nächster Schritt: Nutzer startet wie vereinbart manuell Compile-only, danach Gerätetest und /radiodns-log senden. Der Log soll für D392/D394 zeigen, welche Varianten 404/200 liefern und welche tatsächlich als Rasterbild übernommen werden.


## 23. Logo proportional an die Layoutfläche anpassen – 2026-11-03

- In `firmware/ats-mini-os/src/display/DisplayDriver.cpp` wird vor dem Zeichnen die Quellgröße des Logos aus dem PNG-IHDR beziehungsweise aus dem JPEG-Start-of-Frame-Header gelesen.
- Für die vorhandene Logo-Innenfläche (84 × 74 Pixel) wird ein einheitlicher Skalierungsfaktor berechnet. Das vollständige Logo wird proportional verkleinert/vergrößert und innerhalb des bestehenden Rahmens zentriert.
- Wichtig: LovyanGFX behandelt `maxWidth` und `maxHeight` als Begrenzung/Clipping, nicht als automatische proportionale Anpassung. Der neue Code verwendet deshalb die expliziten `scale_x`-/`scale_y`-Parameter.
- Rahmen, Layout und Logo-Downloadpfad bleiben unverändert. Wenn die Bilddimensionen nicht erkannt werden, wird das Logo nicht gezeichnet und die bestehende Fehleranzeige greift.
- Änderung committed: `a21f219d9a65465fe5818215cfc59f1cadb81ae6` (DisplayDriver.cpp).
- **Noch nicht kompiliert und nicht auf dem ATS Mini V4 getestet.** Kein GitHub-Actions-Workflow gestartet und kein Flash durchgeführt.


## 2026-11-03 – Logo-Fallback-Umbau (noch nicht kompiliert)

- Die aktive RadioDNS-Implementierung wurde aus dem Firmwarepfad entfernt: keine CNAME-/SRV-Abfragen, kein Radioplayer-/SI.xml-Zugriff, keine ECC/GCC-basierte RadioDNS-Identität und keine RadioDNS-Status-/Log-Endpunkte mehr.
- Neue Klasse: `firmware/ats-mini-os/src/logo/StationLogoClient.h/.cpp`.
- Logoquelle 1 bleibt FM-DX Webserver, PI-basierte PNG/JPEG-Kandidaten; Logoquelle 2 ist Radio Browser mit Sendername und `countrycode=DE`.
- Radio-Browser-Treffer werden nur bei exaktem normalisiertem Sendernamen und Deutschland angenommen; die heruntergeladenen Bytes müssen als PNG oder JPEG erkannt werden.
- Logos bleiben ausschließlich im RAM. Es wurden keine LittleFS-Schreibvorgänge, Formatierungen, Partitionen, Memories oder Schedules geändert.
- Display-Logo-Rahmen und die bereits bestätigte Skalierungslogik wurden nicht geändert.
- WDR 3, WDR 4 und WDR COSMO müssen am Gerät/API noch verifiziert werden; Verfügbarkeit korrekter Community-Favicons ist nicht garantiert. Die PI-Zuordnung für COSMO muss anhand tatsächlicher RDS-Daten bestätigt werden.
- **Buildstatus:** nicht kompiliert; keine GitHub Actions gestartet. **Flashstatus:** nicht geflasht.


## 2026-11-03 – FM-DX Programm-Logos

- Quelle geprüft: `Highpoint2000/webserver-station-logos`, Plugin `StationLogo/updateStationLogo.js`.
- Das Plugin verwendet `https://tef.noobish.eu/logos/`, den ITU-Länderordner und sucht priorisiert `PI_PROGRAMNAME.svg`, danach `PI_PROGRAMNAME.png`, danach allgemeine `PI.svg` / `PI.png`.
- Programmname wird in Großbuchstaben umgewandelt; Leerzeichen und Sonderzeichen werden entfernt. Bei gleichem PI kann damit das konkrete Programm ausgewählt werden.
- `StationLogoClient.cpp` versucht jetzt vor dem generischen PI-Logo die FM-DX-Dateinamen `PI_PROGRAMNAME.png) mit erwarteten Programmnamen, RDS-PS und Radiotext. SVG wird nicht an den aktuellen ESP32-Decoder übergeben, da die Anzeige aktuell PNG/JPEG erwartet.
- PI-Zuordnung `D391 -> 1LIVE` als gezielte Kandidatenzuordnung ergänzt; muss am Gerät mit empfangenem PI bestätigt werden.
- Änderung noch nicht kompiliert und nicht auf Hardware getestet. Keine Actions gestartet, kein Flash.


## 2026-11-04 – Lokale PI-Logos und temporärer Online-Fallback (noch nicht kompiliert)

- Nutzer hat die PNG-Senderlogos über den Dual Boot Firmware Manager einzeln in das LittleFS-Stammverzeichnis hochgeladen. Die Dateien tragen den PI-Code am Anfang, teils mit weiteren Sendernamen im Dateinamen, z. B. `D381_NDR1_Niedersachsen.png`, `D399_Antenne_Muenster.png` und `D496_COSMO.png`.
- Gewünschte Suchreihenfolge:
  1. zuerst lokal unter `/logos` nach einem unterstützten Bild suchen; der PI-Code darf irgendwo im Dateinamen vorkommen,
  2. bei lokalem Fehltreffer WLAN nur für die Online-Suche aktivieren,
  3. FM-DX zuerst, Radio Browser als Alternative,
  4. Online-Treffer lokal cachen,
  5. wenn kein Treffer vorliegt, temporär aktiviertes WLAN wieder beenden; eine bereits vorher aktive Verbindung darf nicht getrennt werden.
- Änderungen auf `dev/first-build-preparation`:
  - `StorageDiscovery.cpp`: Root-PNG-Dateien (maximal 64 Kandidaten) außer `splash.png` werden nach `/logos` verschoben. Kopie wird anhand Bytezahl und Zieldateigröße geprüft; erst danach wird die Quelle entfernt. Existierende Zieldateien werden nicht überschrieben. Es wird nicht formatiert und nicht rekursiv in Unterordnern verschoben.
  - `ats-mini-os.ino`: `StorageDiscovery::begin()` wird beim Start vor dem Logo-Client aufgerufen.
  - `StationLogoClient.cpp`: lokaler Dateiname wird case-insensitiv nach PI-Teilstring und PNG/JPG/JPEG-Endung durchsucht; lokale Suche erfolgt vor dem Onlinepfad. Online-Logos werden nach erfolgreicher RAM-Übernahme unter `/logos/<PI>_REMOTE.png|jpg` zwischengespeichert.
  - `WifiClient.cpp`: der bisherige automatische WLAN-Start mit Test-SSID wurde aus `begin()` entfernt; Startmeldung nennt WLAN-Standby.
  - Online-Fallback: temporäre WLAN-Moduswiederherstellung ist über einen Scope-Guard vorgesehen.
- Relevante Commits dieser Änderungskette:
  - Root-PNG-Organisation: `68662cae182ea6d97677311b4e8ace413806dbba`
  - Storage-Initialisierung beim Boot: `49cad1f314e980d14d3054cd12d2601d721e15a7`
  - WLAN-Standby: `ad844ca8eaaba84233cf77be6f27800cc10b2252`
  - Lokale PI-Dateisuche: `813595055e029b6c001f64a910068c9df49c34bf`
  - Temporärer WLAN-Fallback: `5d84542fa184ab4cc812ff1482a4e64be50f46d9`
  - Remote-Logo-Cache: `a7d02c0a12ff7b515eaa0b80fe6e51bd792e4349`
- **Wichtige Prüfhinweise vor bzw. nach dem Compile-only-Build:**
  - Der Storage-Discovery-Mount setzt weiterhin eine bekannte Ankerdatei voraus (Schedules.json, schedules.bin, memories.json oder splash.png). Wenn auf dem Gerät keine dieser Dateien vorhanden ist, werden die Root-Logos aktuell nicht automatisch organisiert und die lokale Suche bleibt deaktiviert. Vor dem Gerätetest prüfen bzw. gezielt beheben, ohne Formatierung.
  - Der WLAN-Scope-Guard stellt den Modus wieder her, wenn er zuvor WIFI_OFF oder WIFI_AP war. Falls der ESP32 vorher bereits im WIFI_STA-Modus, aber nicht verbunden war, muss noch geprüft werden, dass die temporär gestartete Verbindung ebenfalls zuverlässig beendet wird und die vorherige Auto-Reconnect-Einstellung wiederhergestellt wird.
  - Die lokalen Dateien werden beim Lesen als PNG/JPG/JPEG anhand der Bildsignatur in `storeLogoInRam()` validiert; der bestehende Renderer unterstützt weiterhin PNG/JPEG, nicht SVG.
  - PI-/Sendername-Zuordnungen in `expectedName()` sind nicht vollständig verifiziert; insbesondere D392 ist im aktuellen Code noch als WDR 3 bezeichnet, obwohl die lokale Dateisuche für vorhandene PI-Dateinamen davon unabhängig funktioniert.
- Display-Logo-Rahmen, Proportionen und Skalierung wurden nicht geändert. Keine Partitions-/Bootloaderänderung, kein Formatieren und keine absichtliche Löschung bestehender Nutzerdaten.
- **Teststatus: noch nicht kompiliert und nicht auf dem ATS Mini V4 getestet.** Nutzer startet den manuellen Compile-only-Workflow selbst. Assistent hat keine GitHub Actions gestartet und nichts geflasht.
- Nächster Schritt: Compile-only-Ergebnis abwarten. Bei Fehlern zuerst kompilierbare Schnittstellen/Includes korrigieren; bei Erfolg vor dem Gerätetest die oben genannten Mount- und WLAN-Wiederherstellungsbedingungen prüfen.

## 2026-11-04 – Antenne-Münster-Logo D394 überarbeitet; nächster Schritt FM-Layout

- Der Nutzer hat den erfolgreichen lokalen Logo-Import auf dem ATS Mini V4 bestätigt. Beim bisherigen Logo von Antenne Münster war der Sendername auf schwarzem Displayhintergrund kaum lesbar, weil die Buchstaben innen schwarz und nur hell umrandet waren.
- Neues Logo erstellt: `D394_Antenne_Muenster.png`.
  - PNG, 168 × 148 Pixel, RGBA mit transparentem Hintergrund.
  - Sendername weiß gefüllt, mit schwarzer Kontur und heller Außenkante für hohen Kontrast auf schwarzem Display.
  - Frequenz `95.4 MHz` und Signalbalken in Rot mit heller Kontur.
  - Typografie und Signalbalken optisch geglättet/neu gezeichnet, um den pixeligen Eindruck zu reduzieren.
  - Die Datei wurde dem Nutzer zum Download bereitgestellt und kann direkt nach `/logos` kopiert werden.
- Das bestehende Logo-Anzeigefenster, dessen Rahmen, Proportionen und Skalierungslogik bleiben unverändert.
- **Nächster Arbeitsschritt:** gemeinsam das erste richtige FM-Layout für den ATS Mini OS entwickeln. Das Layout soll als eigenständiger, modularer FM-Bildschirm aufgebaut werden; vor Änderungen zuerst den tatsächlichen Branch-/Quellcode-Stand prüfen und vorhandene Display-/Radio-Schnittstellen berücksichtigen.
- Noch keine FM-Layout-Implementierung begonnen, kein Build/keine GitHub Actions gestartet und nichts geflasht.


## 2026-11-04 – Vorhandenes Senderlogo bei gleichem PI automatisch ersetzen

- Nutzerwunsch: Ein neu hochgeladenes Logo soll ein vorhandenes Logo automatisch ersetzen, wenn bereits eine Datei mit demselben PI-Code existiert. Das gilt auch bei abweichendem Dateinamen; so kann ein schöneres Logo eingespielt werden, ohne alte Varianten manuell zu entfernen.
- `StorageDiscovery.cpp` wurde angepasst:
  - PI wird aus den ersten vier Hex-Zeichen des Dateinamens erkannt (z. B. `D394_Antenne_Muenster.png` oder `D394.png`).
  - Die neue Root-PNG wird zunächst nach `/logos/.replace_<PI>.tmp` kopiert und anhand der Bytezahl/Dateigröße geprüft.
  - Erst nach erfolgreicher Prüfung wird die bisherige Datei mit exakt gleichem Zielnamen vorübergehend gesichert, die neue Datei installiert und danach werden andere Logo-Dateien mit demselben PI-Präfix entfernt.
  - Wenn Kopie/Prüfung oder Installation fehlschlägt, bleibt die bisherige Logo-Datei erhalten; bei einem Fehler beim Installieren wird die exakt gleichnamige Sicherung zurückbenannt.
  - `splash.png` bleibt ausdrücklich ausgeschlossen. Es wird nicht formatiert und es werden keine Partitionen verändert.
- Commit: [771c3d839249e782cff0fa8420b2b044ed675c3a](https://github.com/13HN2709/ATS_Mini_-OS/commit/771c3d839249e782cff0fa8420b2b044ed675c3a)
- **Status: Code geändert, noch nicht kompiliert und nicht auf dem ATS Mini V4 getestet.** Keine GitHub Actions gestartet, nichts geflasht.
- Nächster Schritt: Nutzer führt den manuellen Compile-only-Build aus. Danach testen, indem ein neues Logo mit bereits vorhandenem PI (z. B. D394) in das LittleFS-Stammverzeichnis hochgeladen wird; nach dem Neustart muss die neue Datei unter `/logos` liegen und die alte PI-Variante entfernt sein.


## 2026-11-04 – Logo-Import ersetzt vorhandenes PI-Logo automatisch

- Nutzerwunsch: Wird ein neues Senderlogo in das LittleFS-Stammverzeichnis hochgeladen und existiert unter `/logos` bereits ein Logo mit demselben PI-Code, soll das neue Bild das alte automatisch ersetzen. Das ist ausdrücklich gewünscht, damit ein schöneres/aktualisiertes Logo ohne manuelles Löschen übernommen wird.
- `StorageDiscovery.cpp` verwendet beim Boot-Import den vierstelligen PI-Präfix am Dateianfang (z. B. `D394_Antenne_Muenster.png`). Vorhandene unterstützte Bilddateien mit demselben PI (`.png`, `.jpg`, `.jpeg`) werden nach erfolgreicher Installation des neuen Logos entfernt, auch wenn der Dateiname anders lautet.
- Sicherheitsablauf: neues Root-PNG zunächst nach `/logos/.replace_<PI>.tmp` kopieren und Dateigröße/Bytezahl prüfen; bei identischem Zieldateinamen wird die alte Datei vorübergehend als Backup umbenannt; erst nach erfolgreichem Installieren wird das Backup und werden andere alte Varianten desselben PI entfernt. Bei fehlgeschlagener Installation wird das exakte alte Ziel zurückbenannt und die Root-Quelldatei bleibt erhalten.
- Vorhandene temporäre/Backup-Dateien führen jetzt zum Abbruch dieses PI-Imports, statt automatisch gelöscht zu werden. Nicht unterstützte Dateitypen werden bei der PI-Bereinigung nicht entfernt. `splash.png` bleibt weiterhin ausgeschlossen.
- Änderung: `firmware/ats-mini-os/src/storage/StorageDiscovery.cpp`.
- Commit: https://github.com/13HN2709/ATS_Mini_-OS/commit/ceb62787fb33853e815a117634f0217259456254
- **Status: noch nicht kompiliert und nicht erneut auf dem ATS Mini V4 getestet.** Kein Build/keine GitHub Actions gestartet, nichts geflasht. Nutzer startet den manuellen Compile-only-Workflow selbst.


## 2026-11-04 – Dateimanager: LittleFS-Verwaltung als Modul begonnen

- Nutzerfreigabe: Dateimanager direkt in ATS Mini OS mit Ordnernavigation und Kopieren, Verschieben, Löschen, Umbenennen, Ordner anlegen sowie Anzeige von Dateigröße/Speicherbelegung.
- Neue Dateien:
  - `firmware/ats-mini-os/src/storage/FileManager.h`
  - `firmware/ats-mini-os/src/storage/FileManager.cpp`
- Implementierte API: Verzeichnisliste (begrenzt), Ordner anlegen, Datei kopieren, verschieben, umbenennen, löschen und leere Ordner entfernen.
- Schutzmechanismen: kein Formatieren; nur nutzbares, bereits von `StorageDiscovery` gemountetes LittleFS; Root und `splash.png` geschützt; interne Logo-Transaktionsdateien werden nicht in der normalen Liste angezeigt; Kopieren/Verschieben kopiert und verifiziert zuerst, erst danach wird die Quelle entfernt. Überschreiben vorhandener Ziele ist gesperrt.
- `ats-mini-os.ino` initialisiert den Dateimanager nach `StorageDiscovery::begin()`.
- Commits: `0ac2e66ed58c71829053855941c85b45d80576cb` (Header), `100592495d9783c845444f19efa10d47801161dc` (Implementierung), `a0969909a2b50fee88892b51146bfa65fd1454cb` (Initialisierung).
- **Status: Modulgrundlage integriert; Encoder-Menüoberfläche und interaktive Dateiauswahl/Aktionsbestätigung sind noch nicht fertig. Nicht kompiliert und nicht am Gerät getestet.** Keine Actions gestartet, nichts geflasht, keine Formatierung und keine Partitionsänderung.
- Nächster Schritt: Dateimanager als eigene Ansicht im vorhandenen Vollbild-Menü einhängen; Navigation und sichere Aktionsdialoge implementieren, danach Compile-only.
