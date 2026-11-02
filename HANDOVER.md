# Übergabeprotokoll – ATS Mini OS

> **Zweck:** Dieses Dokument ist die zentrale Übergabe für neue Chat-Sitzungen. Vor jeder Fortsetzung zuerst diese Datei lesen und anschließend den tatsächlichen Repository-Stand prüfen.
>
> **Stand:** 2026-11-01  
> **Repository:** https://github.com/13HN2709/ATS_Mini_-OS  
> **Status:** Grundgerüst angelegt – weiterhin Bestandsaufnahme / Architekturplanung; kein ausführbarer OS-Code

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