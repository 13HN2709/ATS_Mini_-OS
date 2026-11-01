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
