#pragma once

// Read-only diagnostic for the partition table visible to the running app.
// This module never mounts, formats, erases, or writes any partition.
namespace PartitionDiagnostics {
  void printToSerial();
}
