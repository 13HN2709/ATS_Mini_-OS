# Spracheinstellungen für die erste ATS Mini OS-Version

**Entscheidung des Nutzers:** Zunächst hauptsächlich Deutsch. Andere Sprachen und arabische Schrift dürfen vorbereitet werden, sind aber zunächst nicht aktiv.

## Vorgabe für die erste Version

- Standardsprache der Oberfläche: Deutsch.
- Die deutsche Zeichentabelle muss vollständig funktionieren: ä, ö, ü, Ä, Ö, Ü, ß.
- Die neue Schrift soll zuerst im separaten Layout-Test auf Lesbarkeit und Speicherbedarf geprüft werden.
- Bestehende Layouts und ihre Schriften bleiben unverändert.
- Weitere Sprachen werden vorerst weder im normalen Menü angezeigt noch automatisch aktiviert.
- Internationale Glyphen dürfen vorbereitet werden, sollen aber nur dann in die Firmware aufgenommen werden, wenn sie für die deutsche Oberfläche oder den geplanten Test gebraucht werden.
- Arabische Schrift bleibt deaktiviert. Ihre Unterstützung erfordert mehr als Glyphen: arabische Buchstabenformung und Rechts-nach-links-Anordnung müssen gesondert geprüft werden.
- Eine spätere Sprachumschaltung kann ergänzt werden, ohne die deutsche Standardoberfläche zu verändern.

## Umsetzungsvorgaben

1. Deutsch ist die feste Ausgangssprache.
2. UTF-8 und die Anzeige deutscher Sonderzeichen werden explizit getestet.
3. Keine automatischen Ersetzungen wie ae/oe/ue für Umlaute.
4. Zusätzliche Sprachpakete und Schriftdateien modular halten, um Flash-Speicher zu sparen.
5. Vor dem ersten Build: tatsächliche Zeichenabdeckung, Lizenz, Größe und Kompatibilität prüfen.
6. Kein Build und kein Flash ohne gesonderte Abstimmung mit dem Nutzer.
