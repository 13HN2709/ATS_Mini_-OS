# Compile and flash safety

The user confirmed that the OSPI-DE build has compiled successfully and runs in their Dual Boot setup. A previous CI run (#41374394331) also compiled both OSPI and QSPI profiles successfully. These results verify compilation and, for the user's OSPI-DE artifact, a working boot/run in their setup; they do not validate the current unbuilt source revision or make QSPI-DE interchangeable with OSPI-DE.

The OSPI-DE profile remains at `FlashSize=8M`, `PSRAM=opi`, CPU 80 MHz, flash mode qio and the checked-in custom partition reference. QSPI-DE is a separate experimental profile using `FlashSize=8M` and `PSRAM=enabled`. Do not change the flash-size setting or partition offsets/sizes without a deliberate decision based on the known-good ATS-Mini-Dev configuration. Physical 16 MB flash capacity does not by itself require this build profile to use `FlashSize=16M`.

The workflow is manual-only and compile-only. It does not flash a device. The user starts any build explicitly. A successful compile alone does not validate hardware compatibility.

The source ZIP is intended to include the sketch tree as it exists on the runner after LovyanGFX and SI4735 implementation sources are staged. It includes those staged source copies used for the compile, but it is not a fully self-contained offline toolchain bundle. Library install commands currently fetch upstream repository heads rather than a pinned commit; strict dependency reproducibility remains an open item.

The private development branch contains temporary phone-hotspot credentials in `src/network/WifiTestConfig.h` for the user's first connectivity test. Keep that file private and do not copy it into public repositories or public release artifacts.

## Remaining before next user-started compile
1. Confirm the source archive includes the staged third-party source files and correct named artifacts.
2. Pin third-party library revisions or document the exact versions resolved by the runner.
3. Review the source tree for compile errors and stale documentation.
4. Keep OSPI-DE as the primary next test; do not change the 8 MB profile or partition table as part of this cleanup.
