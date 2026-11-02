# ATS Mini OS firmware scaffold

This is the first code-bearing scaffold, not yet the complete radio firmware.

## Current scope
- Arduino-ESP32 profiles for ESP32-S3 OSPI and QSPI, based on the read-only `ats-mini-dev` profile definitions.
- A central CPU profile manager with exactly four choices: Auto, FixedLow, FixedMedium, FixedHigh.
- Temporary performance leases for overlapping tasks, with expiry protection.
- Serial-only bring-up diagnostics.

## Explicitly not implemented yet
- ATS Mini parallel display driver and controller detection.
- Full SI4735 radio integration, RDS and RadioDNS pipeline.
- Menu/UI to select the four CPU profiles.
- Actual clock changes: CPU operating points must be verified before calling `setCpuFrequencyMhz()`.
- Final firmware release and verified dual-boot packaging.

## Build and flash safety
The manual-only workflow `.github/workflows/ats-mini-os-first-compile.yml` compiles OSPI and QSPI as separate profiles. Both use `FlashSize=8M`; OSPI-DE uses `PSRAM=opi` and QSPI-DE uses `PSRAM=enabled`. The user has confirmed OSPI-DE builds successfully and runs in their Dual Boot setup. This does not make the QSPI profile interchangeable with OSPI-DE.

The partition CSV is retained as a compile reference. Do not flash this scaffold or change partition offsets/sizes based only on the physical 16 MB flash capacity. A compile is not proof of partition, bootloader, or hardware compatibility. The workflow has no flash/device-write step and must only be started by the user.

The source ZIP is assembled from the actual sketch tree after third-party source files are staged by the runner. It includes those staged implementations, along with project files. It is not a promise that every toolchain package is included for a completely offline build.

## Dauerhafte Entwicklungsregel: Modularität

Jede fachlich eigenständige Funktion bekommt ihre eigenen `.h`- und `.cpp`-Dateien unter `src/`. Die `.ino`-Datei bleibt ein dünner Einstiegspunkt. Keine `.cpp`-Datei per `#include` einbinden. Module dürfen nur über dokumentierte Schnittstellen miteinander kommunizieren; Änderungen an einem Modul sollen keine unabhängigen Funktionen verändern. Bei Schnittstellenänderungen sind alle Aufrufer zu prüfen und die Übergabe in `HANDOVER.md` zu aktualisieren.

## Modular FM bring-up (implementation added; compile pending)
- Added `src/tuner/TunerDriver.h/.cpp` as the sole SI4735 library/hardware owner.
- Added `src/radio/RadioCore.h/.cpp` as the radio-state API, with FM tune, 100 kHz channel stepping, a snapshot, and periodic RSSI/SNR/stereo reads.
- The `.ino` initializes the radio at 103.9 MHz and prints diagnostic status over serial every two seconds. Display and encoder are deliberately not active yet.
- Hardware assumptions are based on the ATS Mini V4 reference pin map: LDO 15, reset 16, SDA 18, SCL 17. Validate compile first, then hardware with serial diagnostics. Do not flash until explicitly approved.

## Persistente Einstellungen

`src/settings/SettingsStore.h/.cpp` verwaltet OS-eigene Einstellungen über ESP32 NVS (`Preferences`) in der separaten Namespace `atsmini_os`. Die letzte FM-Frequenz wird beim Start geladen, auf 87,5–108,0 MHz validiert und nach drei Sekunden ohne weiteres Abstimmen gespeichert, um unnötige Flash-Schreibvorgänge zu reduzieren. Wenn NVS nicht verfügbar ist, startet das Radio mit der definierten Standardfrequenz. Aktuell ist die Frequenz die erste persistente Einstellung; weitere Werte (CPU-Profil, Lautstärke, Stummschaltung, Betriebsart) sollen über dasselbe Modul ergänzt werden. NVS ist bewusst gewählt, um nicht von einer gemeinsam genutzten LittleFS-Partition oder deren Layout abhängig zu sein. Noch nicht kompiliert/getestet.

## Geplante Layout-Familien: Future UI

Die Layout-Architektur wird getrennte Familien für FM/VHF und ALL-BAND erhalten. Diese Aufteilung ist als verbindliche Designentscheidung aufgenommen; sie ist noch keine implementierte oder hardwaregetestete Funktion.

### FM/VHF – mit Senderlogos
- `FutureFM_1_layout.cpp` (erstes Layout)
- `FutureFM_2.cpp` (zweite Variante)
- Für UKW-spezifische Inhalte: Senderlogo, Sendername/PS, RDS-Radiotext, Stereo/Mono und Signalstärke.
- RadioDNS ist der bevorzugte Kandidat für die spätere automatische Senderlogo-Auflösung; Verfügbarkeit hängt vom Sender und den Empfangs-/Netzwerkdaten ab.

### ALL-BAND – AM/USB/LSB und weitere unterstützte Bänder
- `FutureALLBand_1.cpp` (erstes Layout im futuristischen „Future Broadcast“-Stil)
- `FutureALLBand_2.cpp` (zweite Variante)
- Eigene Anzeige für Betriebsart, Frequenz und Einheit, BFO bei SSB, Bandbreite, S-Meter und später optional Spektrum/Wasserfall.
- Kein reservierter FM-Senderlogo-Bereich in dieser Layout-Familie.

### Modulregeln
- Ein zentraler `LayoutManager` wählt Layouts passend zu Betriebsart/Band und Nutzerauswahl.
- Jedes Layout bleibt ein eigenständiges Modul mit eigener `.h`-/`.cpp`-Schnittstelle; gemeinsame Zeichenhilfen dürfen separat bereitgestellt werden.
- `Layout-TEF6686.cpp` bleibt weitgehend unverändert und wird nicht als Experimentierfläche für die neuen Designs verwendet.
- Erst die First-Flash-/Boot-/Partitions-Sicherheitsprüfung und die dafür nötigen Grundfunktionen abschließen; anschließend die Layout-Familien schrittweise implementieren und jeweils separat kompilieren/testen.

### Umsetzungspriorität und Reihenfolge
Die Umsetzung erfolgt bewusst in kleinen, prüfbaren Etappen – nicht erst alle FM-Funktionen fertigstellen, bevor ALL-BAND beginnt:

1. **Zuerst das erste FM-Layout fertigstellen:** das bereits begonnene und gut funktionierende Layout weiterführen, stabilisieren und optisch abschließen. Senderlogos und FM-spezifische Informationen gehören zu diesem Layout-Ziel.
2. **Danach das erste ALL-BAND-Layout erstellen:** `FutureALLBand_1.cpp` für AM/USB/LSB und weitere unterstützte Bänder im „Future Broadcast“-Stil.
3. **Anschließend die übrigen Funktionen ergänzen:** weitere Layout-Varianten, RadioDNS-/Logo-Verbesserungen, zusätzliche Bedien- und Radiofunktionen sowie die restlichen Betriebsarten schrittweise integrieren und testen.

Diese Reihenfolge ersetzt die frühere Planung, ALL-BAND erst nach vollständigem Abschluss sämtlicher FM-Funktionen zu beginnen. Das erste FM-Layout bleibt der unmittelbare Schwerpunkt; das erste ALL-BAND-Layout folgt direkt danach.
