#include "StorageDiscovery.h"

#include <LittleFS.h>
#include <FS.h>
#include <esp_partition.h>
#include <string.h>
#include <strings.h>
#include <vector>

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


// Move only root-level PNGs into /logos. splash.png is deliberately preserved
// as the boot image. A source is removed only after a complete verified copy.
void organizeRootLogos() {
  if (!LittleFS.exists("/logos") && !LittleFS.mkdir("/logos")) {
    Serial.println("[LOGO] could not create /logos; root files left untouched");
    return;
  }

  std::vector<String> candidates;
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    Serial.println("[LOGO] could not enumerate LittleFS root");
    return;
  }

  File entry = root.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      String path = entry.path();
      const int slash = path.lastIndexOf('/');
      const String base = slash >= 0 ? path.substring(slash + 1) : path;
      String lower = base;
      lower.toLowerCase();
      if (lower != "splash.png" && lower.endsWith(".png") && candidates.size() < 64) {
        candidates.push_back(path);
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();

  uint8_t buffer[512];
  for (const String& sourcePath : candidates) {
    const int slash = sourcePath.lastIndexOf('/');
    const String base = slash >= 0 ? sourcePath.substring(slash + 1) : sourcePath;
    const String destinationPath = String("/logos/") + base;

    if (LittleFS.exists(destinationPath)) {
      Serial.printf("[LOGO] destination exists; preserving both files: %s\n",
                    destinationPath.c_str());
      continue;
    }

    File source = LittleFS.open(sourcePath, "r");
    if (!source || source.isDirectory()) {
      if (source) source.close();
      continue;
    }
    const size_t expected = source.size();
    File destination = LittleFS.open(destinationPath, "w");
    if (!destination) {
      source.close();
      Serial.printf("[LOGO] cannot create %s; source preserved\n", destinationPath.c_str());
      continue;
    }

    size_t copied = 0;
    bool writeOk = true;
    while (source.available()) {
      const size_t count = source.read(buffer, sizeof(buffer));
      if (!count) break;
      if (destination.write(buffer, count) != count) {
        writeOk = false;
        break;
      }
      copied += count;
    }
    destination.flush();
    destination.close();
    source.close();

    File verify = LittleFS.open(destinationPath, "r");
    const bool verified = verify && copied == expected && verify.size() == expected;
    if (verify) verify.close();

    if (writeOk && verified) {
      if (LittleFS.remove(sourcePath)) {
        Serial.printf("[LOGO] moved %s -> %s (%u bytes)\n",
                      sourcePath.c_str(), destinationPath.c_str(),
                      static_cast<unsigned>(copied));
      } else {
        Serial.printf("[LOGO] copied %s but could not remove original\n",
                      sourcePath.c_str());
      }
    } else {
      LittleFS.remove(destinationPath);
      Serial.printf("[LOGO] copy verification failed for %s; source preserved\n",
                    sourcePath.c_str());
    }
  }
}
} // namespace

bool begin() {
  g_ready = false;
  g_partitionLabel[0] = '\0';
  g_anchorPath[0] = '\0';

  // Preference order matters if a future table contains both candidates:
  // first the ATS Mini V4 Dual Boot layout, then the stock/ATS-Mini-Dev layout.
  const uint32_t candidateAddresses[] = {0x00A00000UL, 0x00610000UL};
  Serial.println("[STORAGE] Read-only discovery; prefer Dual Boot LittleFS first");

  for (size_t candidate = 0;
       candidate < sizeof(candidateAddresses) / sizeof(candidateAddresses[0]);
       ++candidate) {
    const uint32_t wantedAddress = candidateAddresses[candidate];
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, nullptr);

    while (it != nullptr) {
      const esp_partition_t* part = esp_partition_get(it);
      if (part) {
        Serial.printf("[STORAGE] candidate label=%s addr=0x%08lX subtype=0x%02X size=%lu\n",
                      part->label, static_cast<unsigned long>(part->address),
                      static_cast<unsigned>(part->subtype),
                      static_cast<unsigned long>(part->size));

        // Accept only a runtime-declared DATA partition named "littlefs" at
        // the exact expected address. Never mount a raw flash offset: 0x610000
        // can overlap app1 in the Dual Boot layout.
        if (strcmp(part->label, "littlefs") == 0 &&
            part->address == wantedAddress) {
          Serial.printf("[STORAGE] checking LittleFS at 0x%08lX\n",
                        static_cast<unsigned long>(wantedAddress));
          strncpy(g_partitionLabel, part->label, sizeof(g_partitionLabel) - 1);
          g_partitionLabel[sizeof(g_partitionLabel) - 1] = '\0';

          if (LittleFS.begin(false, "/littlefs", 10, g_partitionLabel)) {
            File root = LittleFS.open("/");
            const bool found = root && findAnchor(root, 0);
            if (root) root.close();

            if (found) {
              Serial.printf("[STORAGE] anchor found: %s on partition '%s'\n",
                            g_anchorPath, g_partitionLabel);
              if (!LittleFS.exists("/RDS") && !LittleFS.mkdir("/RDS")) {
                Serial.println("[STORAGE] anchor found, but /RDS creation failed");
                LittleFS.end();
              } else {
                g_ready = true;
                Serial.println("[STORAGE] /RDS ready; filesystem was not formatted");
                organizeRootLogos();
              }
            } else {
              Serial.println("[STORAGE] no known anchor file found; no directory created");
              LittleFS.end();
            }
          } else {
            Serial.println("[STORAGE] LittleFS mount failed; no format attempted");
          }

          // A declared candidate was handled. If it contained no anchor, the
          // next address may be checked; no format or raw-flash access occurs.
          esp_partition_iterator_release(it);
          it = nullptr;
          break;
        }
      }
      it = esp_partition_next(it);
    }
    if (it) esp_partition_iterator_release(it);
    if (g_ready) break;
  }

  if (!g_ready) {
    g_partitionLabel[0] = '\0';
    g_anchorPath[0] = '\0';
    Serial.println("[STORAGE] no usable anchor found; persistent RadioDNS cache disabled");
  }
  return g_ready;
}

bool ready() { return g_ready; }
const char* partitionLabel() { return g_partitionLabel; }
const char* anchorPath() { return g_anchorPath; }
} // namespace StorageDiscovery
