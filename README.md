# ATS Mini OS

Ein neues, modular geplantes OS-/Firmwareprojekt für den ATS Mini V4 und perspektivisch kompatible ESP32-basierte Radios.

## Projektstatus

**Phase:** Bestandsaufnahme und Architekturplanung  
**Firmware-Code:** Noch nicht implementiert  
**Builds / GitHub Actions / Flashen:** Nicht freigegeben

Dieses Repository ist ein neues, getrenntes Projekt. Bestehende Firmware-Repositories sowie die installierte Firmware in Slot A und DiamondRadio in Slot B bleiben unangetastet.

## Dokumentation

- [Vorbereitung bis zum ersten Build](docs/FIRST_BUILD_READINESS.md)

- [RadioDNS-Senderlogo: benötigte Daten und Diagnoseplan](docs/RADIODNS_LOGO_DATA_REQUIREMENTS.md)
- [RadioDNS-Schnittstellen und Diagnose-Testlayout](docs/RADIODNS_INTERFACES_AND_TEST_LAYOUT.md)

- [Übergabeprotokoll](HANDOVER.md)
- [Architekturentwurf](docs/ARCHITECTURE.md)
- [Hardware-Inventar](docs/HARDWARE_INVENTORY.md)
- [Partitionen und Boot-Kompatibilität](docs/PARTITIONS_AND_BOOT.md)
- [Modul- und Schnittstellenplan](docs/MODULES_AND_INTERFACES.md)
- [Risiken und Prüfplan](docs/RISK_REGISTER_AND_CHECKLIST.md)

## Geplante Bereiche

Die Verzeichnisse enthalten zunächst nur Beschreibungen und Platzhalter. Konkrete Implementierungen folgen erst nach Quellcode- und Hardwareprüfung sowie entsprechender Freigabe.

- `app/` – Systemstart und Lebenszyklus
- `hal/` – Hardware-Abstraktionsschnittstellen
- `drivers/` – Display-, Speicher- und Tuner-Treiber
- `radio/` – gemeinsamer Radio-Kern
- `modules/` – Spectrum, Dual RX, Web Control und Wetter
- `ui/` – Layouts und Bedienung
- `services/` – Konfiguration, Logging, Performance- und Netzwerkdienste
- `tests/` – spätere Testspezifikation
- `tools/` – spätere, nicht destruktive Hilfswerkzeuge

## Grundregeln

1. Keine Annahmen als bestätigte Hardwarefakten darstellen.
2. Schnittstellen vor Implementierungen definieren.
3. Hardwarefähigkeiten explizit prüfen, insbesondere bei Dual RX.
4. OSPI und QSPI sind die einzigen vorgesehenen Speicherbus-Optionen im betrachteten Umfang.
5. CPU-Leistungsprofile zentral verwalten; rechenintensive Ansichten wie Waterfall dürfen einen Performance-Modus anfordern.
6. WLAN-Funktionen wie Wetter und optionale Bilddownloads modular und nicht-blockierend entwerfen; Offline-Radio muss weiter funktionieren.
7. Vor jeglichen Builds oder Flash-Vorgängen müssen Slotgrößen, Imageformat, Bootpfad und Wiederherstellung geprüft und separat freigegeben werden.


## Font-Plan für das Diagnose-Testlayout

- Ausgewählter erster Kandidat: **DSEGstended 14-Segment Regular** (SIL Open Font License 1.1 laut Projekt).
- Quelle: https://github.com/ZeusOfTheCrows/DSegstended
- Geprüft werden deutsche Umlaute und `ß`, italienische Akzente, Buchstaben, Ziffern und passende Darstellungsgrößen für Frequenz, MHz, Sendername und RadioDNS-Diagnose.
- Arabisch bleibt ein separater späterer Rendering-Test mit Noto Sans Arabic; RTL-/Shaping-Unterstützung muss nachgewiesen werden.
- Fontdateien sind noch nicht integriert; keine Builds oder GitHub Actions gestartet.
