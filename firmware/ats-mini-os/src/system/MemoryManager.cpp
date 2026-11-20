#include "MemoryManager.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <stdio.h>
#include <string.h>

#include "../storage/StorageDiscovery.h"

namespace MemoryManager {
namespace {
constexpr uint32_t SAMPLE_INTERVAL_MS = 15000;
constexpr size_t LOG_SEGMENT_LIMIT = 24 * 1024;
constexpr char LOG_DIR[] = "/system";
constexpr char LOG_PATH[] = "/system/memory.log";
constexpr char LOG_PREVIOUS[] = "/system/memory.prev.log";

bool g_started = false;
bool g_logReady = false;
uint32_t g_lastSample = 0;
uint32_t g_allocationFailures = 0;
char g_lastFailure[72] = "none";

uint32_t largestInternal() {
  return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}
uint32_t largestPsram() {
  return heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void rotateLogIfNeeded() {
  File current = LittleFS.open(LOG_PATH, "r");
  if (!current) return;
  const size_t size = current.size();
  current.close();
  if (size < LOG_SEGMENT_LIMIT) return;

  // Rotate only our own diagnostic files. User data and other log files are untouched.
  LittleFS.remove(LOG_PREVIOUS);
  LittleFS.rename(LOG_PATH, LOG_PREVIOUS);
}

void appendLine(const char* line) {
  if (!g_logReady || !line) return;
  rotateLogIfNeeded();
  File file = LittleFS.open(LOG_PATH, "a");
  if (!file) {
    g_logReady = false;
    return;
  }
  file.println(line);
  file.flush();
  file.close();
}

void appendSnapshot(const char* event) {
  const Snapshot s = snapshot();
  char line[320];
  snprintf(line, sizeof(line),
    "t=%lu,event=%s,psram_total=%lu,psram_free=%lu,psram_largest=%lu,psram_min=%lu,"
    "internal_total=%lu,internal_free=%lu,internal_largest=%lu,internal_min=%lu,"
    "alloc_failures=%lu,last_failure=%s",
    static_cast<unsigned long>(s.sampleUptimeMs), event,
    static_cast<unsigned long>(s.psramTotal),
    static_cast<unsigned long>(s.psramFree),
    static_cast<unsigned long>(s.psramLargest),
    static_cast<unsigned long>(s.psramMinimumFree),
    static_cast<unsigned long>(s.internalTotal),
    static_cast<unsigned long>(s.internalFree),
    static_cast<unsigned long>(s.internalLargest),
    static_cast<unsigned long>(s.internalMinimumFree),
    static_cast<unsigned long>(s.allocationFailures), g_lastFailure);
  appendLine(line);
}
}  // namespace

bool begin() {
  if (g_started) return true;
  g_started = true;

  // StorageDiscovery owns the single LittleFS mount. Never mount it again here.
  if (StorageDiscovery::ready()) {
    if (!LittleFS.exists(LOG_DIR)) LittleFS.mkdir(LOG_DIR);
    File test = LittleFS.open(LOG_PATH, "a");
    g_logReady = static_cast<bool>(test);
    if (test) test.close();
  }
  appendSnapshot("boot");
  g_lastSample = millis();
  return true;
}

void update() {
  if (!g_started) begin();
  const uint32_t now = millis();
  if (static_cast<uint32_t>(now - g_lastSample) < SAMPLE_INTERVAL_MS) return;
  g_lastSample = now;
  appendSnapshot("periodic");
}

Snapshot snapshot() {
  Snapshot s{};
  s.psramTotal = ESP.getPsramSize();
  s.psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  s.psramLargest = largestPsram();
  s.psramMinimumFree = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  s.internalTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  s.internalFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  s.internalLargest = largestInternal();
  s.internalMinimumFree = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  s.allocationFailures = g_allocationFailures;
  s.sampleUptimeMs = millis();
  s.psramAvailable = s.psramTotal > 0;
  s.logReady = g_logReady;
  return s;
}

void recordAllocationFailure(const char* module, size_t bytes) {
  ++g_allocationFailures;
  snprintf(g_lastFailure, sizeof(g_lastFailure), "%s:%u",
    (module && *module) ? module : "unknown", static_cast<unsigned>(bytes));
  appendSnapshot("allocation_failure");
}

const char* resetReasonName() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON: return "power_on";
    case ESP_RST_EXT: return "external";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt_watchdog";
    case ESP_RST_TASK_WDT: return "task_watchdog";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep_sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unknown";
  }
}

bool logAvailable() {
  return g_logReady;
}
}  // namespace MemoryManager
