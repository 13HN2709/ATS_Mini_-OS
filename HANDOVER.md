# Übergabeprotokoll – ATS Mini OS

> **Zweck:** Dieses Dokument ist die zentrale Übergabe für neue Chat-Sitzungen. Vor jeder Fortsetzung zuerst diese Datei lesen und anschließend den tatsächlichen Repository-Stand prüfen.
>
> **Stand:** 2026-11-01  
> **Repository:** https://github.com/13HN2709/ATS_Mini_-OS  
> **Status:** Erste Firmware-Scaffold-Dateien im Branch `dev/first-build-preparation` angelegt; noch kein getesteter Build und noch kein Radio-/Displaybetrieb

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