# Weather module

**Status:** Requirements and design only. No executable code.

## Goal

Show a current weather report for a user-selected location, initially entered as a German postal code. The feature should be optional and use the radio's existing Wi-Fi connection only when available.

## Planned behavior

- Settings: postal code, country (default Germany), units, update interval and optional manual refresh.
- Resolve the postal code to coordinates through a geocoding service; handle ambiguous or unknown postal codes explicitly.
- Fetch current conditions and a short forecast through a weather provider selected after checking availability, terms, privacy and API-key requirements.
- Show a local icon for sun, cloud, rain, snow, thunderstorm or unknown conditions; show temperature and selected additional values where the display permits.
- Display the timestamp of the last successful update.
- Cache the last valid result and show it with a stale/offline indicator when Wi-Fi or the service is unavailable.
- Avoid blocking the radio loop, tuning, audio, encoder, scanning or Dual RX during network requests.
- Rate-limit requests and use bounded timeouts and bounded response buffers.

## Image support

Weather symbols should initially be local, optimized icons. Downloading a new image for every weather refresh is unnecessary and consumes bandwidth, RAM and flash. A later shared image service may support small remote weather maps or other images after supported formats and decoder/memory costs are verified.

## Open questions

- Which weather/geocoding provider meets the user's needs and can be used within its terms?
- Is a postal code alone sufficient, or should a place name be shown for ambiguous matches?
- How much screen space should the weather view use?
- What update interval balances freshness, service limits and battery use?
- Which network, TLS and JSON libraries are compatible with the selected firmware base?

## Acceptance criteria

- Works when Wi-Fi is connected and remains harmless when offline.
- A failed or slow request never freezes the radio UI.
- Invalid postal codes and provider errors are shown clearly.
- Last successful data remains visible with its timestamp when a refresh fails.
- No build or device test is claimed until explicitly run and verified.