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


## First-flash packaging gate (2026-11-02)

The successful compile run #41438359444 produced OSPI-DE and QSPI-DE review bundles, but neither bundle is approved for flashing. The current Arduino CLI compile invocation does not produce `flash_args`; offsets printed in the bundle are reference notes, not a generated flashing command. The generated merged image is an alternative to separate images, not something to combine with them.

The workflow now writes `FLASH-STATUS.txt` with `FIRST-FLASH STATUS: NOT APPROVED` and sanitizes `WifiTestConfig.h` in the downloadable source artifact by blanking its test SSID/password. The private working branch's local test configuration is not modified by that packaging step. The next user-started compile should verify both the sanitized source artifact and the new status file.

Before approving any first flash:
1. Use OSPI-DE only as the primary target; keep `FlashSize=8M`, `PSRAM=opi`, and the current partition CSV unchanged.
2. Compare the actual partition table, bootloader, offsets, and boot method against the user's known-good backup and the real reference build—not only the checked-in CSV comparison.
3. Confirm which method is intended: the merged image at offset `0x0000`, or separate files with verified offsets. Never mix the two methods.
4. Inspect the generated bundle and verify no required files are missing; the current missing `flash_args` means no ready-to-run flashing command should be inferred.
5. Flash only after the user explicitly approves the specific profile and method. No workflow step writes to a device.
