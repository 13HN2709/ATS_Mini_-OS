# Modulkarte und Schnittstellenplanung

**Status:** Entwurf. Noch keine APIs oder Implementierungen verbindlich festgelegt.

## Geplante Module

| Modul | Verantwortung | Abhängigkeiten / offene Fragen |
|---|---|---|
| App | Systemstart, Lebenszyklus und Zustände | Boot-/Framework-Basis offen |
| HAL | stabile Hardwareverträge | Hardware-Inventar offen |
| Display Driver | Display initialisieren und zeichnen | Controller, Bus und Pins offen |
| Tuner Driver | Tuner steuern und Status lesen | Chip und Bibliothek offen |
| Radio Core | Frequenz, Band, Modulation und Tuning | konkrete Tuner-Fähigkeiten offen |
| UI | Layouts, Menüs, Encoder und Status | Display- und Eingabeschnittstellen offen |
| Spectrum Scanner | Scanablauf, Peaks und Ergebnisse | unterstützte Frequenzen/Modi offen |
| Dual RX | parallele Empfangsfunktion nur bei belegter Unterstützung | Hardwarebeleg zwingend |
| Web Control | Weboberfläche und Verbindungsmodi | Netzwerk- und Sicherheitskonzept offen |
| Config Service | Einstellungen speichern und migrieren | Speichermedium offen |
| Logging Service | Diagnosen und Fehlerberichte | Speicher-/Ausgabekanal offen |
| Performance Manager | CPU-Leistungsprofile zentral steuern | unterstützte Takte und Framework offen |

## Schnittstellenprinzipien

- Module erhalten Abhängigkeiten über explizite Schnittstellen statt direkter globaler Hardwarezugriffe.
- Fähigkeiten werden zur Laufzeit oder über eine geprüfte Hardwarekonfiguration bereitgestellt.
- Hardware- und I/O-Fehler werden als definierte Fehlerzustände zurückgegeben.
- UI und Web Control sollen Radio-Kernfunktionen über dieselben fachlichen Schnittstellen ansprechen.
- Module müssen initialisierbar und – wo sinnvoll – kontrolliert deaktivierbar sein.
- Keine API wird als final markiert, bevor Hardware und Referenzquellcode geprüft sind.

## Spectrum Scanner

Geplante Trennung zwischen Scan-Engine und Anzeige. Frequenzbereiche, Schrittweiten, Modulationswechsel, Abbruch und Peak-Daten werden später als Anforderungen spezifiziert.

## Dual RX

Dual RX wird nur angeboten, wenn Tuner, Hardwarepfad und Treiber die erforderliche parallele Empfangsfunktion tatsächlich unterstützen. Zwei UI-Empfänger oder zwei Softwareobjekte allein beweisen keinen gleichzeitigen Empfang.

## Web Control

Web Control wird als separates Modul geplant. WLAN-Modi, Verbindungsstatus und Anzeigezustand sollen nicht mit dem Radio-Kern vermischt werden.

## Performance Manager

Layouts melden einen Bedarf bzw. ein Leistungsprofil; sie setzen nicht eigenständig an beliebigen Stellen den CPU-Takt. Waterfall ist ein vorgesehener Anwendungsfall für maximale Leistung. Das Wiederherstellen des vorherigen Profils beim Verlassen der Ansicht ist ein Anforderungspunkt.

## Speicherzugriff

Nur die vorgesehenen OSPI-/QSPI-Pfade berücksichtigen. Treiber und Pins erst nach der Hardwareprüfung festlegen.


## RadioDNS / Senderlogo (Schnittstellenentwurf)

Die detaillierten, noch unverbindlichen Verträge stehen in [RADIODNS_INTERFACES_AND_TEST_LAYOUT.md](RADIODNS_INTERFACES_AND_TEST_LAYOUT.md).

Vorgesehene Verantwortlichkeiten:
1. **RDS Snapshot Provider:** genau ein FIFO-Leser, der gültige Gruppen an einen gemeinsamen Snapshot verteilt.
2. **RadioDNS Identity Builder:** normalisiert FM-Frequenz, vollständigen PI und GCC; erstellt FQDN, ServiceIdentifier und Bearer. Keine Netzwerkzugriffe in diesem Modul.
3. **RadioDNS Discovery:** DNS-CNAME und SRV-Auflösung, asynchron, abbrechbar und an eine Sendergeneration gebunden.
4. **SI Parser / Bearer Matcher:** begrenzt SI-XML lesen und den exakt passenden FM-Bearer zuordnen.
5. **Logo Selector / Cache:** Logo-Variante wählen, Ressource prüfen, begrenzt herunterladen und vorhandene gültige Cachedatei schützen.
6. **Diagnostic Layout:** zeigt Daten und Status an; Frequenzänderungen gehen ausschließlich über den Radio Core.

### Gemeinsame Regeln
- Keine direkte RDS-FIFO-Lesung aus RadioDNS/UI.
- Keine Identität nur aus PS oder der festen EU-Region ableiten.
- Keine alten DNS-/Logo-Ergebnisse nach Frequenzwechsel übernehmen: jede Anfrage trägt die Snapshot-/Sendergeneration.
- Netzwerkfehler dürfen Radio, Audio oder Encoder nicht blockieren.
- Interfaces bleiben Entwurf, bis Framework, DNS-/HTTP-Stack, Display-API und Font-Renderer verifiziert sind.

## Diagnoselayout und Schrift

- Das Testlayout soll Frequenz oben links groß zeigen; der Encoder stimmt über den Radio Core ab.
- RDS-/RadioDNS-Zwischenschritte und Logo-Status werden separat dargestellt.
- **DSEGstended 14-Segment Regular** ist der ausgewählte erste Digitalschriften-Kandidat. Die konkrete Fontdatei muss auf `Ä Ö Ü ä ö ü ß`, italienische Akzente, Zahlen und Renderer-Kompatibilität geprüft werden. Projekt und Lizenz: https://github.com/ZeusOfTheCrows/DSegstended (SIL OFL 1.1 laut Upstream). Noch nicht integriert.
- **Noto Sans Arabic** ist für einen separaten arabischen Rendering-Test vorgemerkt. Ohne verifizierte RTL-/Shaping-Unterstützung darf Arabisch nicht als korrekt unterstützt gelten.
- Siehe [RadioDNS-Schnittstellen und Diagnose-Testlayout](RADIODNS_INTERFACES_AND_TEST_LAYOUT.md).


## Verbindlicher Dateiaufbau

Für jedes unabhängige Modul gilt: **eigene Header- und Implementierungsdatei**. Beispiel:

```text
firmware/ats-mini-os/
  ats-mini-os.ino              # nur Einstiegspunkt / setup / loop
  src/
    system/
      PerformanceManager.h
      PerformanceManager.cpp
    radio/
      RadioCore.h
      RadioCore.cpp
    tuner/
      TunerDriver.h
      TunerDriver.cpp
    display/
      DisplayDriver.h
      DisplayDriver.cpp
    input/
      EncoderInput.h
      EncoderInput.cpp
    services/
      RadioDnsService.h
      RadioDnsService.cpp
```

Die Struktur wird bei Bedarf um weitere Module ergänzt; die Beispiele sind keine Behauptung, dass diese Module schon implementiert sind.

## Änderungs- und Isolationsregeln

1. Eine Funktion hat einen klaren fachlichen Besitzer.
2. Header enthalten nur den öffentlichen Vertrag und notwendige Typen; Implementierung gehört in die passende `.cpp`.
3. Keine direkte Änderung fremder interner Variablen, keine mehrfachen Besitzer eines Hardware- oder FIFO-Zugriffs.
4. Module hängen von kleinen Schnittstellen ab; zirkuläre Includes und unnötige globale Zustände sind zu vermeiden.
5. Bei einer Änderung zuerst Aufrufer/Abhängigkeiten ermitteln, dann nur nötige Module ändern.
6. Neue/angepasste Funktionen bekommen mindestens einen dokumentierten Prüfweg; Schnittstellenänderungen erfordern die Prüfung aller Aufrufer.
7. Bei einem Fehler wird die Ursache im verantwortlichen Modul behoben, statt einen Workaround in einem fremden Layout oder Dienst einzubauen.
8. Vor Übergabe an eine neue Chat-Sitzung sind Branch, letzte Commits, offene Risiken, getestete/nicht getestete Teile und der genaue Build-/Flash-Status in `HANDOVER.md` festzuhalten.
