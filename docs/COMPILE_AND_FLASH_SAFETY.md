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

The successful compile run #41438359444 produced OSPI-DE and QSPI-DE review bundles, but neither bundle is approved for flashing. The initial review packaging missed `flash_args` because it was under the sketch-local build directory; the source artifact later revealed it. In run #41442794297, that file reported `--flash-mode dio` even though the requested profile specifies `FlashMode=qio`, so the mismatch must be investigated before flashing. Offsets printed in the bundle are reference notes, not a substitute for checking the generated arguments. The generated merged image is an alternative to separate images, not something to combine with them.

The workflow now writes `FLASH-STATUS.txt` with `FIRST-FLASH STATUS: NOT APPROVED` and sanitizes `WifiTestConfig.h` in the downloadable source artifact by blanking its test SSID/password. The private working branch's local test configuration is not modified by that packaging step. The next user-started compile should verify both the sanitized source artifact and the new status file.

Before approving any first flash:
1. Use OSPI-DE only as the primary target; keep `FlashSize=8M`, `PSRAM=opi`, and the current partition CSV unchanged.
2. Compare the actual partition table, bootloader, offsets, and boot method against the user's known-good backup and the real reference build—not only the checked-in CSV comparison.
3. Confirm which method is intended: the merged image at offset `0x0000`, or separate files with verified offsets. Never mix the two methods.
4. Inspect the generated bundle, verify no required files are missing, and resolve the generated `flash_args` mode mismatch (`dio` versus requested `qio`) before inferring any flashing command.
5. Flash only after the user explicitly approves the specific profile and method. No workflow step writes to a device.


## Review of run #41442794297

The run completed both compile jobs successfully. Inspecting the downloaded OSPI-DE source artifact revealed two packaging issues that were not obvious from the green job status:

- The generated `flash_args` was present under the sketch-local build directory, outside the workflow's original search root. Its first line reports `--flash-mode dio --flash-freq 80m --flash-size 8MB`, while the requested profile specifies `FlashMode=qio`. This mismatch is a hard stop; do not flash until the origin of the differing mode is understood and the actual build output is verified.
- The source-code artifact accidentally included a sketch-local `build/` directory with compiled binaries, ELF/map files, and generated build metadata. The workflow now excludes that directory from the source-only artifact and searches both build output roots for review files.

Commit `90e4a311ad9beeb9750258b6fa1da4d2047501c9` updates the workflow to include a discovered `flash_args` in the review bundle, report a mismatch against the requested QIO profile, and keep build outputs out of the source archive. This workflow edit has not yet been tested by a user-started run. The first-flash gate remains closed; the next run must verify that the review bundle contains the real `flash_args`, explicitly reports the mode mismatch if it remains, and that the source archive no longer contains `build/`.


## Flash-mode correction prepared (2026-11-02)

Commit `3b56cebd4560d7c498172250f1fcb930cd2ced3d` changes the manual compile workflow to explicitly set `build.flash_mode=qio` and `build.flash_freq=80m` in addition to the selected FQBN options. It removes the sketch-local exported build directory before each profile compile and passes Arduino CLI's `--clean` option to prevent a stale `flash_args` file from being mistaken for current output. Compile logs are written outside the cleaned output directory.

After compiling, the workflow now requires a freshly generated `flash_args` file and checks that it declares both `--flash-mode qio` and `--flash-size 8MB`; otherwise the compile job fails before packaging the profile as valid. This change has been committed but is not yet tested. The next manually started run must prove that the generated file agrees with QIO/8 MB. Do not flash until the new run confirms this and bootloader/Dual Boot compatibility is separately reviewed.
