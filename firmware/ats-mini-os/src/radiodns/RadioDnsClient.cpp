#include "RadioDnsClient.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ctype.h>
#include <new>
#include <stdio.h>
#include <string.h>

namespace {
constexpr size_t kDnsPacketSize = 1500;
constexpr size_t kMaxSiBytes = 180000;
constexpr size_t kMaxLogoBytes = 220000;
struct LookupJob { RadioDnsClient* self; RadioDnsFmIdentity identity; };
void logRadioDns(RadioDnsCache* cache, const String& message) {
  String line = String(millis()) + " ms " + message + "\n";
  Serial.print("[RADIODNS] "); Serial.println(message);
  if (cache) cache->appendLog(line.c_str());
}

uint16_t read16(const uint8_t* p) { return (uint16_t(p[0]) << 8) | p[1]; }
uint32_t read32(const uint8_t* p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }

bool skipDnsName(const uint8_t* packet, size_t length, size_t& pos) {
  while (pos < length) {
    uint8_t n = packet[pos++];
    if (n == 0) return true;
    if ((n & 0xC0) == 0xC0) {
      if (pos >= length) return false;
      ++pos;
      return true;
    }
    if ((n & 0xC0) != 0 || pos + n > length) return false;
    pos += n;
  }
  return false;
}

bool encodeDnsName(uint8_t* packet, size_t capacity, size_t& pos, const char* name) {
  const char* start = name;
  while (*start) {
    const char* dot = strchr(start, '.');
    const size_t n = dot ? size_t(dot - start) : strlen(start);
    if (!n || n > 63 || pos + n + 2 >= capacity) return false;
    packet[pos++] = static_cast<uint8_t>(n);
    memcpy(packet + pos, start, n);
    pos += n;
    if (!dot) break;
    start = dot + 1;
  }
  if (pos >= capacity) return false;
  packet[pos++] = 0;
  return true;
}

bool dnsSrvLookup(const char* fqdn, char* host, size_t hostCap, uint16_t& port) {
  host[0] = 0; port = 0;
  IPAddress dns = WiFi.dnsIP(0);
  if (dns == IPAddress(0,0,0,0)) return false;
  uint8_t packet[kDnsPacketSize] = {};
  const uint16_t id = static_cast<uint16_t>(esp_random());
  packet[0] = id >> 8; packet[1] = id & 0xff;
  packet[2] = 1; packet[5] = 1; // recursive query, one question
  size_t q = 12;
  char query[100];
  snprintf(query, sizeof(query), "_radioepg._tcp.%s", fqdn);
  if (!encodeDnsName(packet, sizeof(packet), q, query) || q + 4 > sizeof(packet)) return false;
  packet[q++] = 0; packet[q++] = 33; // SRV
  packet[q++] = 0; packet[q++] = 1;  // IN
  WiFiUDP udp;
  if (!udp.begin(0)) return false;
  if (!udp.beginPacket(dns, 53) || udp.write(packet, q) != q || !udp.endPacket()) { udp.stop(); return false; }
  const uint32_t start = millis();
  int packetSize = 0;
  while (millis() - start < 1800) {
    packetSize = udp.parsePacket();
    if (packetSize > 0) break;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  if (packetSize <= 0 || packetSize > int(sizeof(packet))) { udp.stop(); return false; }
  const int got = udp.read(packet, sizeof(packet));
  udp.stop();
  if (got < 12 || read16(packet) != id || !(packet[2] & 0x80) || (packet[3] & 0x0f) != 0) return false;
  const uint16_t questions = read16(packet + 4);
  const uint16_t answers = read16(packet + 6);
  size_t pos = 12;
  for (uint16_t i = 0; i < questions; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 4 > size_t(got)) return false;
    pos += 4;
  }
  for (uint16_t i = 0; i < answers; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 10 > size_t(got)) return false;
    const uint16_t type = read16(packet + pos);
    const uint16_t klass = read16(packet + pos + 2);
    const uint16_t rdlen = read16(packet + pos + 8);
    pos += 10;
    if (pos + rdlen > size_t(got)) return false;
    if (type == 33 && klass == 1 && rdlen >= 7) {
      port = read16(packet + pos + 4);
      size_t np = pos + 6;
      size_t end = pos + rdlen;
      String target;
      while (np < end) {
        uint8_t n = packet[np++];
        if (!n) break;
        if ((n & 0xC0) == 0xC0) {
          if (np >= end) break;
          size_t ptr = ((n & 0x3f) << 8) | packet[np];
          ++np;
          size_t guard = 0;
          while (ptr < size_t(got) && guard++ < 32) {
            uint8_t label = packet[ptr++];
            if (!label) break;
            if ((label & 0xC0) == 0xC0) {
              if (ptr >= size_t(got)) break;
              size_t next = ((label & 0x3f) << 8) | packet[ptr];
              ++ptr; ptr = next; continue;
            }
            if (ptr + label > size_t(got)) break;
            if (target.length()) target += '.';
            for (uint8_t j=0;j<label;++j) target += char(packet[ptr++]);
          }
          break;
        }
        if ((n & 0xC0) || np + n > end) break;
        if (target.length()) target += '.';
        for (uint8_t j=0;j<n;++j) target += char(packet[np++]);
      }
      if (target.length() && target != "." && target.length() < hostCap && port) {
        target.toCharArray(host, hostCap);
        while (host[0] && host[strlen(host)-1] == '.') host[strlen(host)-1] = 0;
        if (host[0]) return true;
      }
    }
    pos += rdlen;
  }
  return false;
}

String httpGet(const String& url, size_t maxBytes, int& status) {
  status = -1;
  HTTPClient http;
  WiFiClient plain;
  WiFiClientSecure secure;
  secure.setInsecure(); // TLS server identity cannot be validated on this minimal target.
  bool started = url.startsWith("https://") ? http.begin(secure, url) : http.begin(plain, url);
  if (!started) return String();
  http.setConnectTimeout(3500);
  http.setTimeout(4500);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  status = http.GET();
  String body;
  if (status == HTTP_CODE_OK) {
    int len = http.getSize();
    if (len < 0 || size_t(len) <= maxBytes) {
      WiFiClient* stream = http.getStreamPtr();
      body.reserve(len > 0 ? len : 2048);
      uint32_t lastData = millis();
      while (http.connected() && body.length() <= maxBytes && millis() - lastData < 1500) {
        while (stream->available()) {
          body += char(stream->read());
          lastData = millis();
          if (body.length() > maxBytes) break;
        }
        if (body.length() > maxBytes) break;
        vTaskDelay(pdMS_TO_TICKS(2));
      }
    }
  }
  http.end();
  if (body.length() > maxBytes) body = String();
  return body;
}

String attrValue(const String& tag, const char* attr) {
  String key = String(attr) + "=\"";
  int p = tag.indexOf(key);
  if (p < 0) { key = String(attr) + "='"; p = tag.indexOf(key); }
  if (p < 0) return String();
  p += key.length();
  char quote = tag[p - 1];
  int end = tag.indexOf(quote, p);
  return end < 0 ? String() : tag.substring(p, end);
}

String findLogoUrl(const String& xml, const char* bearer) {
  int pos = 0;
  while ((pos = xml.indexOf("<service", pos)) >= 0) {
    if (pos + 8 < xml.length() && isalnum(static_cast<unsigned char>(xml[pos + 8]))) { pos += 8; continue; }
    int end = xml.indexOf("</service>", pos);
    if (end < 0) break;
    String service = xml.substring(pos, end);
    String expected = String("id=\"") + bearer + "\"";
    String expectedSingle = String("id='") + bearer + "'";
    if (service.indexOf(expected) < 0 && service.indexOf(expectedSingle) < 0) { pos = end + 10; continue; }
    String best;
    int m = 0;
    while ((m = service.indexOf("<multimedia", m)) >= 0) {
      int close = service.indexOf('>', m);
      if (close < 0) break;
      String tag = service.substring(m, close + 1);
      String url = attrValue(tag, "url");
      String type = attrValue(tag, "type");
      String mime = attrValue(tag, "mimeValue");
      if (url.startsWith("http://") || url.startsWith("https://")) {
        // Prefer square 128px logos, then the largest common square fallback.
        String width = attrValue(tag, "width"), height = attrValue(tag, "height");
        if (type.indexOf("logo") >= 0 || mime.startsWith("image/")) {
          if (width == "128" && height == "128") return url;
          if (best.isEmpty() || (width == "600" && height == "600")) best = url;
        }
      }
      m = close + 1;
    }
    return best;
  }
  return String();
}
}

bool RadioDnsClient::begin(RadioDnsCache* cache) {
  cache_ = cache;
  return cache_ != nullptr;
}

void RadioDnsClient::update(const RadioDnsFmIdentity& identity, bool wifiConnected) {
  static int lastGateCode = -1;
  const int gateCode = (!cache_ || !cache_->ready()) ? 1 : (!wifiConnected ? 2 : (identity.status != RadioDnsIdentityStatus::Ready ? 10 + static_cast<int>(identity.status) : 0));
  if (gateCode != lastGateCode) {
    lastGateCode = gateCode;
    if (cache_ && cache_->ready()) {
      const char* gccSource = identity.countryFallbackUsed ? "DE-fallback" :
                              (identity.status == RadioDnsIdentityStatus::Ready ? "RDS-ECC" : "unavailable");
      logRadioDns(cache_, String("gate state=") + gateCode +
        " wifi=" + (wifiConnected ? "connected" : "disconnected") +
        " identityStatus=" + String(static_cast<int>(identity.status)) +
        " PI=" + identity.pi + " GCC=" + identity.gcc +
        " GCC-source=" + gccSource + " FQDN=" + identity.fqdn);
    }
  }
  if (!cache_ || !cache_->ready() || !wifiConnected || busy_ || identity.status != RadioDnsIdentityStatus::Ready) return;
  const uint32_t now = millis();
  if (haveRequested_ && strcmp(lastRequestedFqdn_, identity.fqdn) == 0 &&
      now - lastAttemptMs_ < 60000) return;
  strncpy(lastRequestedFqdn_, identity.fqdn, sizeof(lastRequestedFqdn_) - 1);
  lastRequestedFqdn_[sizeof(lastRequestedFqdn_) - 1] = 0;
  haveRequested_ = true;
  lastAttemptMs_ = now;
  LookupJob* job = new (std::nothrow) LookupJob{this, identity};
  if (!job) { logRadioDns(cache_, "no memory for lookup job"); return; }
  busy_ = true;
  if (xTaskCreate(taskEntry, "RadioDNS", 8192, job, 1, nullptr) != pdPASS) {
    busy_ = false;
    delete job;
    logRadioDns(cache_, "could not start lookup task");
  }
}

void RadioDnsClient::taskEntry(void* arg) {
  LookupJob* job = static_cast<LookupJob*>(arg);
  RadioDnsClient* self = job->self;
  RadioDnsFmIdentity identity = job->identity;
  delete job;
  self->runLookup(identity);
  self->busy_ = false;
  vTaskDelete(nullptr);
}

void RadioDnsClient::runLookup(const RadioDnsFmIdentity& identity) {
  logRadioDns(cache_, String("lookup started: fqdn=") + identity.fqdn + " bearer=" + identity.bearer);
  if (!WiFi.isConnected()) { logRadioDns(cache_, "WiFi dropped before lookup task ran"); return; }
  char key[32];
  snprintf(key, sizeof(key), "%s_%s_%s", identity.gcc, identity.pi, identity.frequency);
  if (cache_->assetExists(key, "png") || cache_->assetExists(key, "jpg")) {
    logRadioDns(cache_, String("cache hit: /RDS/") + key + ".*");
    return;
  }

  char host[256] = {};
  uint16_t port = 0;
  if (!dnsSrvLookup(identity.fqdn, host, sizeof(host), port)) {
    logRadioDns(cache_, String("SRV lookup failed: no _radioepg._tcp SRV record for ") + identity.fqdn + "; DNS server=" + WiFi.dnsIP(0).toString());
    return;
  }
  logRadioDns(cache_, String("metadata server ") + host + ":" + String(unsigned(port)));
  String base = String("http://") + host + ":" + String(port) + "/radiodns/spi/3.1/SI.xml";
  int status = -1;
  String xml = httpGet(base, kMaxSiBytes, status);
  if (status != HTTP_CODE_OK || xml.isEmpty()) {
    logRadioDns(cache_, String("SI.xml request failed (HTTP/status ") + String(status) + "); URL=" + base);
    return;
  }
  logRadioDns(cache_, String("SI.xml received bytes=") + xml.length() + " saved=" + (cache_->saveMetadata(key, xml.c_str()) ? "yes" : "no"));

  String logoUrl = findLogoUrl(xml, identity.bearer);
  if (logoUrl.isEmpty()) {
    logRadioDns(cache_, String("no matching service/logo; bearer=") + identity.bearer + " XML preview=" + xml.substring(0, 900));
    return;
  }
  logRadioDns(cache_, String("logo URL selected: ") + logoUrl);
  String lower = logoUrl; lower.toLowerCase();
  const char* ext = lower.endsWith(".jpg") || lower.endsWith(".jpeg") ? "jpg" : "png";
  int logoStatus = -1;
  String logo = httpGet(logoUrl, kMaxLogoBytes, logoStatus);
  if (logoStatus != HTTP_CODE_OK || logo.isEmpty()) {
    logRadioDns(cache_, String("logo download failed (HTTP/status ") + String(logoStatus) + "); URL=" + logoUrl);
    return;
  }
  if (cache_->saveAsset(key, ext, reinterpret_cast<const uint8_t*>(logo.c_str()), logo.length()))
    logRadioDns(cache_, String("logo cached at /RDS/") + key + "." + ext + " bytes=" + String(logo.length()));
  else
    logRadioDns(cache_, String("logo cache write failed: /RDS/") + key + "." + ext + " bytes=" + String(logo.length()));
}
