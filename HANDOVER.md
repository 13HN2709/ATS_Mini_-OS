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
