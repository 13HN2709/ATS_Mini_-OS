#include "FileManager.h"

#include <LittleFS.h>
#include "StorageDiscovery.h"
#include <string.h>

namespace FileManager {
namespace {
bool g_ready = false;

bool validPath(const char* path) {
  if (!path || path[0] != '/' || path[1] == '\0') return false;
  if (strstr(path, "//") || strstr(path, "/../") || strstr(path, "/./")) return false;
  const size_t n = strlen(path);
  if (n >= 96 || path[n - 1] == '/') return false;
  return strcmp(path, "/") != 0;
}

bool isProtectedPath(const char* path) {
  if (!path) return true;
  return strcmp(path, "/") == 0 ||
         strcmp(path, "/splash.png") == 0 ||
         strcmp(path, "/Schedules.json") == 0 ||
         strcmp(path, "/schedules.bin") == 0 ||
         strcmp(path, "/memories.json") == 0 ||
         strncmp(path, "/.replace_", 10) == 0 ||
         strncmp(path, "/.backup_", 9) == 0;
}

bool sameParent(const char* a, const char* b) {
  char pa[96], pb[96];
  strncpy(pa, a, sizeof(pa)-1); pa[sizeof(pa)-1] = 0;
  strncpy(pb, b, sizeof(pb)-1); pb[sizeof(pb)-1] = 0;
  char* sa = strrchr(pa, '/'); char* sb = strrchr(pb, '/');
  if (!sa || !sb) return false;
  if (sa == pa) sa[1] = 0; else *sa = 0;
  if (sb == pb) sb[1] = 0; else *sb = 0;
  return strcmp(pa, pb) == 0;
}

bool copyVerified(const char* sourcePath, const char* destinationPath) {
  if (!validPath(sourcePath) || !validPath(destinationPath) ||
      isProtectedPath(sourcePath) || isProtectedPath(destinationPath) ||
      LittleFS.exists(destinationPath)) return false;
  File src = LittleFS.open(sourcePath, "r");
  if (!src || src.isDirectory()) { if (src) src.close(); return false; }
  const size_t expected = src.size();
  const String temp = String(destinationPath) + ".fm_tmp";
  if (temp.length() >= 96) { src.close(); return false; }
  if (LittleFS.exists(temp)) { src.close(); return false; }
  File dst = LittleFS.open(temp, "w");
  if (!dst) { src.close(); return false; }
  uint8_t buffer[512];
  size_t copied = 0;
  bool ok = true;
  while (src.available()) {
    size_t n = src.read(buffer, sizeof(buffer));
    if (!n) break;
    if (dst.write(buffer, n) != n) { ok = false; break; }
    copied += n;
  }
  dst.flush(); dst.close(); src.close();
  File verify = LittleFS.open(temp, "r");
  const bool verified = verify && ok && copied == expected && verify.size() == expected;
  if (verify) verify.close();
  if (!verified || !LittleFS.rename(temp, destinationPath)) {
    LittleFS.remove(temp);
    return false;
  }
  return true;
}
}

bool begin() {
  g_ready = StorageDiscovery::ready();
  return g_ready;
}
bool ready() { return g_ready && StorageDiscovery::ready(); }

bool list(const char* directory, Entry* entries, uint8_t capacity, uint8_t& count) {
  count = 0;
  if (!ready() || !directory || !entries || !capacity) return false;
  File dir = LittleFS.open(directory);
  if (!dir || !dir.isDirectory()) { if (dir) dir.close(); return false; }
  File item = dir.openNextFile();
  while (item && count < capacity) {
    Entry& out = entries[count];
    const String path = item.path();
    path.toCharArray(out.path, sizeof(out.path));
    const char* base = strrchr(out.path, '/');
    base = base ? base + 1 : out.path;
    strncpy(out.name, base, sizeof(out.name)-1);
    out.name[sizeof(out.name)-1] = 0;
    out.directory = item.isDirectory();
    out.size = out.directory ? 0 : item.size();
    // Hide internal transaction files from normal browsing.
    if (strncmp(out.name, ".replace_", 9) != 0 &&
        strncmp(out.name, ".backup_", 8) != 0 &&
        strstr(out.name, ".fm_tmp") == nullptr) ++count;
    item.close();
    item = dir.openNextFile();
  }
  if (item) item.close();
  dir.close();
  return true;
}

bool makeDirectory(const char* path) {
  return ready() && validPath(path) && !isProtectedPath(path) &&
         !LittleFS.exists(path) && LittleFS.mkdir(path);
}

bool copyFile(const char* source, const char* destination) {
  return ready() && copyVerified(source, destination);
}

bool moveFile(const char* source, const char* destination) {
  if (!ready() || !validPath(source) || !validPath(destination) ||
      isProtectedPath(source) || isProtectedPath(destination) ||
      LittleFS.exists(destination)) return false;
  // Copy and verify first. Only then remove the original.
  if (!copyVerified(source, destination)) return false;
  if (!LittleFS.remove(source)) {
    LittleFS.remove(destination);
    return false;
  }
  return true;
}

bool renamePath(const char* source, const char* destination) {
  if (!ready() || !validPath(source) || !validPath(destination) ||
      isProtectedPath(source) || isProtectedPath(destination) ||
      LittleFS.exists(destination)) return false;
  return LittleFS.rename(source, destination);
}

bool removeFile(const char* path) {
  if (!ready() || !validPath(path) || isProtectedPath(path)) return false;
  File f = LittleFS.open(path, "r");
  if (!f || f.isDirectory()) { if (f) f.close(); return false; }
  f.close();
  return LittleFS.remove(path);
}

bool removeEmptyDirectory(const char* path) {
  if (!ready() || !validPath(path) || isProtectedPath(path)) return false;
  File dir = LittleFS.open(path);
  if (!dir || !dir.isDirectory()) { if (dir) dir.close(); return false; }
  File child = dir.openNextFile();
  const bool empty = !child;
  if (child) child.close();
  dir.close();
  return empty && LittleFS.rmdir(path);
}
}
