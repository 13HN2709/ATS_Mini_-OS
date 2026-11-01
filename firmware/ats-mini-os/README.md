# ATS Mini OS firmware scaffold

This is the first code-bearing scaffold, not yet the complete radio firmware.

## Current scope
- Arduino-ESP32 profiles for ESP32-S3 OSPI and QSPI, based on the read-only `ats-mini-dev` profile definitions.
- A central CPU profile manager with exactly four choices: Auto, FixedLow, FixedMedium, FixedHigh.
- Temporary performance leases for overlapping tasks, with expiry protection.
- Serial-only bring-up diagnostics.

## Explicitly not implemented yet
- ATS Mini parallel display driver and controller detection.
- SI4735 radio initialization, audio path, RDS and RadioDNS.
- Menu/UI to select the four CPU profiles.
- Actual clock changes: CPU operating points must be verified before calling `setCpuFrequencyMhz()`.
- Final 16 MB partition/dual-boot packaging. The checked-in partition CSV is a reference for compilation only, not a flash instruction.

Do not flash this scaffold to a radio. It is intended for the first compile check after the user authorizes build start.


## Dauerhafte Entwicklungsregel: Modularität

Jede fachlich eigenständige Funktion bekommt ihre eigenen `.h`- und `.cpp`-Dateien unter `src/`. Die `.ino`-Datei bleibt ein dünner Einstiegspunkt. Keine `.cpp`-Datei per `#include` einbinden. Module dürfen nur über dokumentierte Schnittstellen miteinander kommunizieren; Änderungen an einem Modul sollen keine unabhängigen Funktionen verändern. Bei Schnittstellenänderungen sind alle Aufrufer zu prüfen und die Übergabe in `HANDOVER.md` zu aktualisieren.

## Compile workflow

A manual-only compile workflow is prepared at `.github/workflows/ats-mini-os-first-compile.yml`. It requires the input `COMPILE` and builds OSPI/QSPI as separate jobs. It has no flash or device-write step and has not been run. The current partition CSV is a reference only and must not be used to flash the user's 16 MB dual-boot device.


## Modular FM bring-up (implementation added; compile pending)
- Added `src/tuner/TunerDriver.h/.cpp` as the sole SI4735 library/hardware owner.
- Added `src/radio/RadioCore.h/.cpp` as the radio-state API, with FM tune, 100 kHz channel stepping, a snapshot, and periodic RSSI/SNR/stereo reads.
- The `.ino` now initializes the radio at 103.9 MHz and prints diagnostic status over serial every two seconds. Display and encoder are deliberately not active yet.
- Compile workflow installs `PU2CLR SI4735` before compiling.
- Hardware assumptions are based on the ATS Mini V4 reference pin map: LDO 15, reset 16, SDA 18, SCL 17. Must validate compile first, then validate on hardware with serial diagnostics. Do not flash until explicitly approved. No workflow run has been started for this code yet.


## Persistente Einstellungen

`src/settings/SettingsStore.h/.cpp` verwaltet OS-eigene Einstellungen über ESP32 NVS (`Preferences`) in der separaten Namespace `atsmini_os`. Die letzte FM-Frequenz wird beim Start geladen, auf 87,5–108,0 MHz validiert und nach drei Sekunden ohne weiteres Abstimmen gespeichert, um unnötige Flash-Schreibvorgänge zu reduzieren. Wenn NVS nicht verfügbar ist, startet das Radio mit der definierten Standardfrequenz. Aktuell ist die Frequenz die erste persistente Einstellung; weitere Werte (CPU-Profil, Lautstärke, Stummschaltung, Betriebsart) sollen über dasselbe Modul ergänzt werden. NVS ist bewusst gewählt, um nicht von einer gemeinsam genutzten LittleFS-Partition oder deren Layout abhängig zu sein. Noch nicht kompiliert/getestet.
