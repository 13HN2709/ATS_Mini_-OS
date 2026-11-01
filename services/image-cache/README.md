# Image service and cache

**Status:** Design placeholder only. No decoder or downloader implemented.

## Purpose

Provide a shared, bounded way for optional modules to use local images and, later, selected remote images without each module implementing its own download and memory handling.

## Proposed stages

1. Local assets and simple icons included with the firmware.
2. Small HTTPS image downloads with strict size/time limits.
3. Validated decode into a display-compatible buffer or line-by-line rendering path.
4. Optional cache in an appropriate storage partition only after storage/partition compatibility is verified.

## Safety and resource limits

- Allowlist supported formats and reject malformed or oversized files.
- Bound response bytes, image width/height, decoded pixel count, timeouts and cache use.
- Never trust remote dimensions or content type without validating the actual data.
- Do not let image downloads/decoding block radio operation.
- Consider PSRAM availability but do not assume it until the exact target profile is verified.
- Do not alter boot partitions or existing firmware slots to make room without explicit approval.

## Format decision

JPEG, BMP and PNG are candidates, not confirmed commitments. Check the chosen display stack, decoder support, license, peak memory and display dimensions before selecting formats.