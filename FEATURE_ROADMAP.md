# ATS Mini OS – Feature Restoration Roadmap

Reference: `13HN2709/ats-mini-dev` (read-only). This roadmap separates feature restoration from the later FM DX Optimizer.

## Ground rules
- Preserve the tested FM main screen (logo, frequency and RDS ticker).
- Themes are color/style palettes; layouts are a separate setting and must remain independent.
- Keep modules under `src/<domain>/`; UI calls RadioCore/services, never the SI4735 driver directly.
- Reuse existing ClockService, WifiClient, SettingsStore and LittleFS infrastructure where appropriate.
- No partition-table, flash-offset, bootloader or slot changes.
- No automatic GitHub Actions, no flashing. User runs compile-only QSPI and OSPI builds.
- Verify every library/API and hardware dependency before enabling a control. No placeholder menu items that pretend a feature works.

## Reference feature inventory

### Appearance (not layouts)
- Color themes/palettes (DEV `Themes.h` / `Themes.cpp`).
- Independent layout selection.
- Brightness, menu zoom, encoder scroll direction and half-step behavior.
- RDS display mode, FM stereo/mono preference, FM region.
- Persist theme and appearance preferences in the existing `settings` NVS partition, using OS-owned keys/namespaces; never clear or overwrite DEV namespaces.

### Connectivity and remote control
- Bluetooth: Off, BLE UART/ad-hoc remote, BLE HID remote, clear paired devices.
- Wi-Fi modes: Off, access point, AP + station, station/client, NTP time sync.
- Web remote/control interface.
- TCP remote protocol.
- USB serial remote/control protocol.
- Connection status and safe start/stop lifecycle; scan/Dual RX command arbitration must be respected.

### EiBi shortwave schedule
- Download/update EiBi text data from the upstream schedule source.
- Parse and store a compact indexed local representation in LittleFS.
- Report data availability and last update/error state.
- Lookup current station by frequency and UTC time.
- Previous/next active schedule entry and other entries on the same frequency.
- Frequency/band label hints are separate from EiBi broadcaster schedule data.
- Requires working network download, LittleFS capacity/error handling, and a synchronized UTC clock for time-dependent results.

### Radio and reception
- Bandwidth (partly present), AGC/ATT, AVC, soft mute.
- BFO and USB/LSB calibration; modulation selection.
- Seek, scan, squelch, memories, VFO A/B, Dual RX.
- DSP patch selection/management.
- FM region and stereo/mono settings.
- Keep all receiver hardware calls inside TunerDriver via RadioCore.

### Spectrum and band tools
- VHF/FM spectrum and all-band spectrum scanner.
- User-selected start/end frequency, step and modulation; peak selection tunes through RadioCore.
- Bandplan Assistant and band-use labels.
- Keep spectrum work separate from the FM display's existing layout.

### Clock and system
- UTC/local time display and UTC offset configuration.
- NTP synchronization and manual time setting.
- Sleep timer and sleep modes.
- CPU/performance profile, calibration, system diagnostics/logging, About/device information.
- Firmware update/OTA only after bootloader and dual-slot compatibility are explicitly verified.

## OS baseline at Build #116
Already present in the current OS branch:
- FM main screen and RDS ticker; separate AM-family display layout.
- Band selection and band-dependent tuning step.
- Bandwidth selection (basic tuner integration).
- Display brightness control with NVS persistence code.
- Existing ClockService (UTC epoch, Europe/Berlin conversion, SNTP when Wi-Fi is connected).
- Existing Wi-Fi client (currently test-config-driven, not the DEV mode manager).
- LittleFS storage discovery and file manager.
- SettingsStore on the existing `settings` NVS partition.

Not yet restored as user-facing features:
- Color theme engine and theme selector.
- Bluetooth modes and BLE remote control.
- EiBi download, storage, lookup and navigation.
- Full Wi-Fi mode manager, Web Control, TCP and USB remote.
- Remaining radio controls and memory/VFO/Dual RX functions.
- Full spectrum and bandplan functions.
- Clock/system settings UI and system utilities.

## Implementation order
1. **Settings and appearance foundation:** add OS-owned preference fields and a standalone theme palette manager; integrate theme selection without changing layout selection or the tested FM layout geometry. Keep the existing black/white menu design unless a theme is explicitly intended to affect it.
2. **Clock and EiBi:** add robust EiBi download/parser/index as separate modules; use ClockService UTC; never block the main radio/input loop during network operations.
3. **Connectivity:** expand Wi-Fi modes first, then Web Control/TCP/USB; add BLE UART/HID after checking exact ESP32 core/library versions and memory constraints.
4. **Radio controls:** AGC/ATT, AVC, soft mute, BFO, modulation, seek/scan/squelch and settings persistence, each through TunerDriver/RadioCore.
5. **Memories/VFO/Dual RX and DSP patches.**
6. **Spectrum/bandplan tools.**
7. **System UI:** UTC/time, sleep, CPU profile, calibration, diagnostics and About. OTA remains gated by boot compatibility.

## Working status
- Build #116: QSPI and OSPI compile-only passed.
- This document is an inventory/plan, not a claim that the listed missing features have been implemented.
- Next coding slice: theme palette module and preference integration, then theme selection UI. Keep this separate from layout selection.
- After each coherent implementation slice, update HANDOVER.md and wait for the user's manual QSPI/OSPI compile-only result.
