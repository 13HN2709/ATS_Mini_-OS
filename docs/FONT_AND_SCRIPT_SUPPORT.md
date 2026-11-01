# Mehrsprachige Schriftarten und arabische Schrift

**Status:** Vorbereitungsplan für das erste Layout-Testprojekt. Noch keine Firmware-Implementierung und kein Build.

## Ziel

Das ATS Mini OS soll mehrere austauschbare Schriftstile unterstützen, darunter Digital-Segment, Retro-Pixel und eine gut lesbare Standardschrift. Zeichen aus verschiedenen Sprachen müssen korrekt dargestellt werden, ohne dass Texte von Hand umgeschrieben werden müssen.

## Anforderungen

- Lateinisch erweitert: Deutsch, Italienisch, Französisch, Spanisch, Portugiesisch, Polnisch, Tschechisch, Türkisch und weitere europäische Zeichen.
- Mindestens: `ä ö ü Ä Ö Ü ß à è é ì ò ù ç ñ ã õ ł č ş ğ İ`.
- Griechische und kyrillische Zeichen, sofern sie in die gewählte Schriftdatei passen.
- Arabisch mit korrekter Schreibrichtung von rechts nach links und verbundenen Buchstaben.
- Erweiterte arabische Zeichen für Persisch und Urdu als Ziel, abhängig von der geprüften Glyphenabdeckung.
- Fehlende Zeichen müssen beim Test sichtbar auffallen; keine stillschweigende Ersetzung durch Fragezeichen.
- Schriftdateien klein halten und nur tatsächlich benötigte Zeichensätze einbinden.
- Schriftlizenz, Speicherbedarf und Kompatibilität mit dem vorhandenen Display-System vor Übernahme prüfen.

## Kandidaten zur Prüfung

### Digitalanzeige
- DSEG: https://github.com/keshikan/DSEG
- DSegstended: https://github.com/ZeusOfTheCrows/DSegstended

DSegstended ergänzt unter anderem griechische und kyrillische Großbuchstaben sowie weitere lateinische Sonderzeichen. Die tatsächliche Abdeckung muss aus der konkreten Font-Datei geprüft werden; sie ist nicht automatisch vollständig.

### Retro-/Terminalschrift
- VT323: https://fonts.google.com/specimen/VT323

Die vollständige Abdeckung der gewünschten europäischen Sprachen muss vor Auswahl anhand der Font-Glyphen geprüft werden.

### Arabisch
- ArabicGFX: https://github.com/4SWED/ArabicGFX
- Noto Naskh Arabic: https://notofonts.github.io/noto-docs/specimen/NotoNaskhArabic/

ArabicGFX dokumentiert Unterstützung für LovyanGFX sowie arabische Buchstabenverbindungen, Lam-Alef und Schreibrichtung. Das ist ein Kandidat für einen kleinen, isolierten Adapter. Vor Integration sind Quellcode, Version, Lizenz, RAM-Verbrauch und Kompatibilität mit der konkreten ATS-Display-Konfiguration zu prüfen. Die zugehörigen Schriftdateien haben eigene Lizenzhinweise.

## Architekturentscheidung für die spätere Implementierung

1. Ein zentraler Font-/Textdienst wählt eine Schriftfamilie pro UI-Element.
2. Unicode-Text bleibt in UTF-8 gespeichert; keine manuellen Ersetzungen wie ae/oe/ue.
3. Lateinisch, Griechisch und Kyrillisch werden über geprüfte Glyphenabdeckung dargestellt.
4. Arabischer Text läuft durch eine gesonderte Formungs- und Rechts-nach-links-Stufe, bevor er an die Zeichenroutine geht.
5. Kein zusätzlicher Leser oder Nebenpfad darf Radio-, RDS- oder Display-Abläufe blockieren.
6. Schriften werden modular eingebunden, damit ungenutzte Dateien nicht den Flash-Speicher füllen.

## Abnahmetest für das Layout

Der Testbildschirm muss mindestens diese Zeilen zeigen:

- Deutsch: `ÄÖÜ äöü ß`
- Italienisch: `perché, città, più, così`
- Französisch: `Élève, cœur, Noël, français`
- Spanisch: `año, pingüino, canción`
- Portugiesisch: `ação, coração, não, maçã`
- Polnisch/Tschechisch: `Zażółć gęślą jaźń; Příliš žluťoučký kůň`
- Türkisch: `İstanbul, Iğdır, şeker, çığ`
- Griechisch: `Ελλάδα`
- Kyrillisch: `Привет`
- Arabisch: `مرحبا بالعالم`
- Arabisch mit lateinischen Zeichen/Zahlen: `لوحة ATS 123`
- Optional Persisch/Urdu: `سلام فارسی` / `اردو`

Ein Kandidat gilt erst als bestanden, wenn alle benötigten Zeichen vorhanden sind, die Ausrichtung stimmt und der Text auf dem ATS-Display lesbar ist.

## Build-Grenze

Diese Datei dokumentiert Anforderungen und Kandidaten. Noch keine Abhängigkeiten als fest installiert markieren, keine GitHub Actions starten und kein Gerät flashen. Vor dem ersten Build werden Treiber und Bibliotheksversionen mit dem Referenzquellcode abgeglichen und die konkrete Freigabegrenze mit dem Nutzer abgestimmt.
