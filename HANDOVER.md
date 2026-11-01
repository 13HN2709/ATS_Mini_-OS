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
