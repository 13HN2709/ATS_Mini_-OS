#include "StorageDiscovery.h"

#include <LittleFS.h>
#include <FS.h>
#include <esp_partition.h>
#include <string.h>
#include <strings.h>

namespace StorageDiscovery {
namespace {
bool g_ready = false;
char g_partitionLabel[17] = {};
char g_anchorPath[96] = {};

bool isAnchorName(const char* path) {
  if (!path || !*path) return false;
  const char* base = strrchr(path, '/');
  base = base ? base + 1 : path;
  return strcasecmp(base, "Schedules.json") == 0 ||
         strcasecmp(base, "schedules.bin") == 0 ||
         strcasecmp(base, "memories.json") == 0 ||
         strcasecmp(base, "splash.png") == 0;
}

// Read-only recursive walk, deliberately bounded to avoid pathological trees.
bool findAnchor(File directory, uint8_t depth) {
  if (!directory || !directory.isDirectory() || depth > 4) return false;
  File entry = directory.openNextFile();
  while (entry) {
    const String path = entry.path();
    if (entry.isDirectory()) {
      const bool found = findAnchor(entry, depth + 1);
      entry.close();
      if (found) return true;
    } else {
      const bool found = isAnchorName(path.c_str());
      if (found) {
        path.toCharArray(g_anchorPath, sizeof(g_anchorPath));
        entry.close();
        return true;
      }
      entry.close();
    }
    entry = directory.openNextFile();
  }
  return false;
}
} // namespace

bool begin() {
  g_ready = false;
  g_partitionLabel[0] = '\0';
  g_anchorPath[0] = '\0';

  Serial.println("[STORAGE] Read-only discovery of data partitions");
  esp_partition_iterator_t it = esp_partition_find(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  bool mounted = false;
  while (it != nullptr) {
    const esp_partition_t* part = esp_partition_get(it);
    if (part && part->label) {
      Serial.printf("[STORAGE] candidate label=%s subtype=0x%02X size=%lu\n",
                    part->label, static_cast<unsigned>(part->subtype),
                    static_cast<unsigned long>(part->size));

      // Check ONLY the two LittleFS start addresses known for this project:
      // 0xA00000 = current ATS Mini V4 layout; 0x610000 = ats-mini-dev layout.
      // An address is accepted only when the runtime partition table itself
      // declares a DATA partition named "littlefs" at that exact offset.
      // Never mount raw flash at 0x610000 if it overlaps app1 in another layout.
      const bool knownLittleFsAddress =
          part->address == 0x00A00000UL || part->address == 0x00610000UL;
      if (strcmp(part->label, "littlefs") == 0 && knownLittleFsAddress) {
        Serial.printf("[STORAGE] approved LittleFS offset: 0x%08lX\\n",
                      static_cast<unsigned long>(part->address));
        strncpy(g_partitionLabel, part->label, sizeof(g_partitionLabel) - 1);
        g_partitionLabel[sizeof(g_partitionLabel) - 1] = '\0';
        mounted = LittleFS.begin(false, "/littlefs", 10, g_partitionLabel);
        if (mounted) {
          File root = LittleFS.open("/");
          const bool found = root && findAnchor(root, 0);
          if (root) root.close();
          if (found) {
            Serial.printf("[STORAGE] anchor found: %s on partition '%s'\n",
                          g_anchorPath, g_partitionLabel);
            if (!LittleFS.exists("/RDS") && !LittleFS.mkdir("/RDS")) {
              Serial.println("[STORAGE] anchor found, but /RDS creation failed");
              LittleFS.end();
              mounted = false;
            } else {
              g_ready = true;
              Serial.println("[STORAGE] /RDS ready; filesystem was not formatted");
              break;
            }
          } else {
            Serial.println("[STORAGE] no known anchor file found; no directory created");
            LittleFS.end();
            mounted = false;
          }
        } else {
          Serial.println("[STORAGE] LittleFS mount failed; no format attempted");
        }
      }
    }
    it = esp_partition_next(it);
  }
  if (it) esp_partition_iterator_release(it);

  if (!g_ready) {
    g_partitionLabel[0] = '\0';
    Serial.println("[STORAGE] discovery incomplete; persistent RadioDNS cache disabled");
  }
  return g_ready;
}

bool ready() { return g_ready; }
const char* partitionLabel() { return g_partitionLabel; }
const char* anchorPath() { return g_anchorPath; }
} // namespace StorageDiscovery
