#include "PartitionDiagnostics.h"

#include <Arduino.h>
#include <esp_partition.h>

namespace PartitionDiagnostics {
void printToSerial() {
  Serial.println("[PARTITIONS] Read-only runtime partition inventory");
  Serial.println("[PARTITIONS] No mount/format/erase/write operation is performed.");

  esp_partition_iterator_t it = esp_partition_find(
      ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);

  if (it == nullptr) {
    Serial.println("[PARTITIONS] No partitions exposed by ESP-IDF partition API.");
    Serial.println("[PARTITIONS] This is diagnostic only; do not infer flash corruption from this alone.");
    return;
  }

  while (it != nullptr) {
    const esp_partition_t* p = esp_partition_get(it);
    if (p != nullptr) {
      Serial.printf(
          "[PARTITION] label=%s type=0x%02X subtype=0x%02X address=0x%06lX size=0x%06lX (%lu bytes) encrypted=%s\n",
          p->label,
          static_cast<unsigned>(p->type),
          static_cast<unsigned>(p->subtype),
          static_cast<unsigned long>(p->address),
          static_cast<unsigned long>(p->size),
          static_cast<unsigned long>(p->size),
          p->encrypted ? "yes" : "no");
    }
    it = esp_partition_next(it);
  }
  Serial.println("[PARTITIONS] Inventory complete.");
}
}  // namespace PartitionDiagnostics
