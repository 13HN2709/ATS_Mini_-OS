# Compile and flash safety

Run #41374394331 compiled both OSPI and QSPI profiles successfully. This verifies compilation only, not hardware operation or boot compatibility.

The current partition table ends at 0x800000 (8 MiB) and the build profiles specify FlashSize=8M. The target device is reported as 16 MB, but the installed bootloader and dual-boot map are not independently verified. Do not flash these artifacts or change partition offsets/sizes based on this scaffold.

The source ZIP contains the firmware project, but third-party implementation files are staged by the workflow runner and are not committed in the project tree. The ZIP is therefore not a self-contained offline build bundle; use the repository workflow to reproduce the compile.

Wi-Fi credentials must not be committed or bundled in downloadable artifacts. The configuration now defaults to empty values and Wi-Fi startup is skipped until a private configuration method is provided.

Before the next compile: confirm the physical OSPI/QSPI/PSRAM variant, document the 16 MB bootloader/partition map from the known-good backup, and pin third-party dependency versions. No build or flash has been started by this change.
