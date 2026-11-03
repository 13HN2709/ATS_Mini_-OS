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

bool dnsQuery(const char* name, uint16_t type, uint8_t* packet,
              size_t capacity, int& responseLength) {
  responseLength = 0;
  IPAddress dns = WiFi.dnsIP(0);
  if (dns == IPAddress(0, 0, 0, 0) || capacity < 64) return false;

  memset(packet, 0, capacity);
  const uint16_t id = static_cast<uint16_t>(esp_random());
  packet[0] = id >> 8;
  packet[1] = id & 0xff;
  packet[2] = 1; // recursion desired
  packet[5] = 1; // one question

  size_t q = 12;
  if (!encodeDnsName(packet, capacity, q, name) || q + 4 > capacity) return false;
  packet[q++] = static_cast<uint8_t>(type >> 8);
  packet[q++] = static_cast<uint8_t>(type & 0xff);
  packet[q++] = 0;
  packet[q++] = 1; // IN

  WiFiUDP udp;
  if (!udp.begin(0)) return false;
  if (!udp.beginPacket(dns, 53) || udp.write(packet, q) != q || !udp.endPacket()) {
    udp.stop();
    return false;
  }

  const uint32_t started = millis();
  int packetSize = 0;
  while (millis() - started < 2200) {
    packetSize = udp.parsePacket();
    if (packetSize > 0) break;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  if (packetSize <= 0 || packetSize > static_cast<int>(capacity)) {
    udp.stop();
    return false;
  }
  responseLength = udp.read(packet, capacity);
  udp.stop();
  if (responseLength < 12 || read16(packet) != id ||
      !(packet[2] & 0x80) || (packet[3] & 0x0f) != 0) {
    responseLength = 0;
    return false;
  }
  return true;
}

// Decodes a DNS name and follows compression pointers. consumed reports the
// number of bytes occupied at the original offset, not the expanded name.
bool decodeDnsName(const uint8_t* packet, size_t length, size_t offset,
                   char* output, size_t outputCapacity, size_t* consumed = nullptr) {
  if (!packet || !output || outputCapacity < 2 || offset >= length) return false;
  size_t pos = offset;
  size_t used = 0;
  size_t out = 0;
  bool jumped = false;
  size_t steps = 0;
  output[0] = '\0';

  while (pos < length && steps++ < 128) {
    const uint8_t label = packet[pos++];
    if (label == 0) {
      if (!jumped) used = pos - offset;
      output[out] = '\0';
      if (consumed) *consumed = used;
      return true;
    }
    if ((label & 0xC0) == 0xC0) {
      if (pos >= length) return false;
      const size_t pointer = (size_t(label & 0x3F) << 8) | packet[pos++];
      if (pointer >= length) return false;
      if (!jumped) used = pos - offset;
      jumped = true;
      pos = pointer;
      continue;
    }
    if ((label & 0xC0) != 0 || pos + label > length) return false;
    if (out && out + 1 >= outputCapacity) return false;
    if (out) output[out++] = '.';
    if (out + label >= outputCapacity) return false;
    for (uint8_t i = 0; i < label; ++i) {
      output[out++] = static_cast<char>(packet[pos++]);
    }
  }
  return false;
}

bool dnsResolveAuthoritativeFqdn(const char* radioDnsFqdn,
                                 char* authoritativeFqdn, size_t hostCapacity) {
  if (!radioDnsFqdn || !*radioDnsFqdn || !authoritativeFqdn || hostCapacity < 8) return false;
  authoritativeFqdn[0] = '\0';
  uint8_t packet[kDnsPacketSize] = {};
  int got = 0;
  // RadioDNS Core Lookup: the broadcast-derived RadioDNS FQDN must resolve
  // through a CNAME to the service provider's authoritative FQDN.
  if (!dnsQuery(radioDnsFqdn, 5, packet, sizeof(packet), got)) return false; // CNAME
  const uint16_t questions = read16(packet + 4);
  const uint16_t answers = read16(packet + 6);
  size_t pos = 12;
  for (uint16_t i = 0; i < questions; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 4 > static_cast<size_t>(got)) return false;
    pos += 4;
  }
  for (uint16_t i = 0; i < answers; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 10 > static_cast<size_t>(got)) return false;
    const uint16_t type = read16(packet + pos);
    const uint16_t klass = read16(packet + pos + 2);
    const uint16_t rdlen = read16(packet + pos + 8);
    pos += 10;
    if (pos + rdlen > static_cast<size_t>(got)) return false;
    if (type == 5 && klass == 1) { // CNAME
      if (decodeDnsName(packet, got, pos, authoritativeFqdn, hostCapacity) &&
          authoritativeFqdn[0]) {
        while (authoritativeFqdn[0] &&
               authoritativeFqdn[strlen(authoritativeFqdn) - 1] == '.') {
          authoritativeFqdn[strlen(authoritativeFqdn) - 1] = '\0';
        }
        return authoritativeFqdn[0] != '\0';
      }
    }
    pos += rdlen;
  }
  return false;
}

bool dnsSrvLookup(const char* authoritativeFqdn, const char* serviceName,
                  char* host, size_t hostCapacity, uint16_t& port) {
  host[0] = '\0';
  port = 0;
  if (!authoritativeFqdn || !*authoritativeFqdn || !serviceName || !*serviceName) return false;

  char query[320];
  const int queryLength = snprintf(query, sizeof(query), "_%s._tcp.%s",
                                   serviceName, authoritativeFqdn);
  if (queryLength <= 0 || static_cast<size_t>(queryLength) >= sizeof(query)) return false;

  uint8_t packet[kDnsPacketSize] = {};
  int got = 0;
  if (!dnsQuery(query, 33, packet, sizeof(packet), got)) return false; // SRV
  const uint16_t questions = read16(packet + 4);
  const uint16_t answers = read16(packet + 6);
  size_t pos = 12;
  for (uint16_t i = 0; i < questions; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 4 > static_cast<size_t>(got)) return false;
    pos += 4;
  }
  for (uint16_t i = 0; i < answers; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 10 > static_cast<size_t>(got)) return false;
    const uint16_t type = read16(packet + pos);
    const uint16_t klass = read16(packet + pos + 2);
    const uint16_t rdlen = read16(packet + pos + 8);
    pos += 10;
    if (pos + rdlen > static_cast<size_t>(got)) return false;
    if (type == 33 && klass == 1 && rdlen >= 7) {
      const uint16_t candidatePort = read16(packet + pos + 4);
      char target[256] = {};
      if (decodeDnsName(packet, got, pos + 6, target, sizeof(target)) &&
          target[0] && candidatePort && strcmp(target, ".") != 0 &&
          strlen(target) < hostCapacity) {
        strncpy(host, target, hostCapacity - 1);
        host[hostCapacity - 1] = '\0';
        while (host[0] && host[strlen(host) - 1] == '.') {
          host[strlen(host) - 1] = '\0';
        }
        if (host[0]) {
          port = candidatePort;
          return true;
        }
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

String findLogoUrl(const String& xml, const char* bearer, String& extension) {
  extension = "png";
  int pos = 0;
  while ((pos = xml.indexOf("<service", pos)) >= 0) {
    if (pos + 8 < xml.length() && isalnum(static_cast<unsigned char>(xml[pos + 8]))) {
      pos += 8;
      continue;
    }
    const int end = xml.indexOf("</service>", pos);
    if (end < 0) break;
    const String service = xml.substring(pos, end);
    bool bearerMatches = false;

    // In SPI SI.xml the service element normally has no id attribute. The FM
    // bearer is a child element: <bearer id="fm:de0.d394.10000" ... />.
    int b = 0;
    while ((b = service.indexOf("<bearer", b)) >= 0) {
      if (b + 7 < service.length() && isalnum(static_cast<unsigned char>(service[b + 7]))) {
        b += 7;
        continue;
      }
      const int close = service.indexOf('>', b);
      if (close < 0) break;
      const String tag = service.substring(b, close + 1);
      const String id = attrValue(tag, "id");
      if (id.equalsIgnoreCase(String(bearer))) {
        bearerMatches = true;
        break;
      }
      b = close + 1;
    }

    if (!bearerMatches) {
      pos = end + 10;
      continue;
    }

    String best;
    String bestExtension = "png";
    int m = 0;
    while ((m = service.indexOf("<multimedia", m)) >= 0) {
      if (m + 11 < service.length() &&
          isalnum(static_cast<unsigned char>(service[m + 11]))) {
        m += 11;
        continue;
      }
      const int close = service.indexOf('>', m);
      if (close < 0) break;
      const String tag = service.substring(m, close + 1);
      const String url = attrValue(tag, "url");
      const String type = attrValue(tag, "type");
      const String mime = attrValue(tag, "mimeValue");
      const String width = attrValue(tag, "width");
      const String height = attrValue(tag, "height");
      if ((url.startsWith("http://") || url.startsWith("https://")) &&
          (type.indexOf("logo") >= 0 || mime.startsWith("image/"))) {
        String candidateExtension = "png";
        if (mime.equalsIgnoreCase("image/jpeg") || mime.equalsIgnoreCase("image/jpg")) {
          candidateExtension = "jpg";
        } else if (mime.equalsIgnoreCase("image/png")) {
          candidateExtension = "png";
        } else {
          String lowerUrl = url;
          lowerUrl.toLowerCase();
          const int queryAt = lowerUrl.indexOf('?');
          if (queryAt >= 0) lowerUrl = lowerUrl.substring(0, queryAt);
          if (lowerUrl.endsWith(".jpg") || lowerUrl.endsWith(".jpeg")) candidateExtension = "jpg";
        }

        if (width == "128" && height == "128") {
          extension = candidateExtension;
          return url;
        }
        if (best.isEmpty() || (width == "600" && height == "600")) {
          best = url;
          bestExtension = candidateExtension;
        }
      }
      m = close + 1;
    }
    if (!best.isEmpty()) {
      extension = bestExtension;
      return best;
    }
    return String();
  }
  return String();
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
  if (!job) { Serial.println("[RADIODNS] no memory for lookup job"); return; }
  busy_ = true;
  if (xTaskCreate(taskEntry, "RadioDNS", 8192, job, 1, nullptr) != pdPASS) {
    busy_ = false;
    delete job;
    Serial.println("[RADIODNS] could not start lookup task");
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
  logRadioDns(cache_, String("lookup started: fqdn=") + identity.fqdn +
                      " bearer=" + identity.bearer);
  if (!WiFi.isConnected()) {
    logRadioDns(cache_, "lookup aborted: WiFi disconnected before DNS discovery");
    return;
  }

  char key[32];
  snprintf(key, sizeof(key), "%s_%s_%s", identity.gcc, identity.pi, identity.frequency);
  if (cache_->assetExists(key, "png") || cache_->assetExists(key, "jpg")) {
    logRadioDns(cache_, String("cache hit: /RDS/") + key + " (logo already stored)");
    return;
  }

  char authoritativeFqdn[256] = {};
  if (!dnsResolveAuthoritativeFqdn(identity.fqdn, authoritativeFqdn,
                                   sizeof(authoritativeFqdn))) {
    logRadioDns(cache_, String("CNAME lookup failed for RadioDNS FQDN: ") + identity.fqdn +
                        " (station may be unregistered or DNS unavailable)");
    return;
  }
  logRadioDns(cache_, String("authoritative FQDN: ") + authoritativeFqdn);

  char host[256] = {};
  uint16_t port = 0;
  int status = -1;
  String xml;
  String base;

  if (dnsSrvLookup(authoritativeFqdn, "radioepg", host, sizeof(host), port)) {
    base = String("http://") + host + ":" + String(port) +
           "/radiodns/spi/3.1/SI.xml";
    logRadioDns(cache_, String("metadata server: ") + base);
    xml = httpGet(base, kMaxSiBytes, status);
    if (status != HTTP_CODE_OK || xml.isEmpty()) {
      logRadioDns(cache_, String("HTTP SI.xml request failed: HTTP=") +
                          String(status) + " bytes=" + String(xml.length()) +
                          "; trying TLS _radiospi");
      host[0] = '\0';
      port = 0;
    }
  } else {
    logRadioDns(cache_, String("no _radioepg._tcp SRV record for ") +
                        authoritativeFqdn + "; trying TLS _radiospi");
  }

  // Current SPI deployments may publish the TLS-only _radiospi service.
  // Try it both when HTTP discovery is absent and when the HTTP endpoint fails.
  if (xml.isEmpty() || status != HTTP_CODE_OK) {
    if (!dnsSrvLookup(authoritativeFqdn, "radiospi", host, sizeof(host), port)) {
      logRadioDns(cache_, String("no usable _radiospi._tcp SRV record for ") +
                          authoritativeFqdn);
      return;
    }
    base = String("https://") + host + ":" + String(port) +
           "/radiodns/spi/3.1/SI.xml";
    logRadioDns(cache_, String("TLS metadata server: ") + base);
    xml = httpGet(base, kMaxSiBytes, status);
  }

  if (status != HTTP_CODE_OK || xml.isEmpty()) {
    logRadioDns(cache_, String("SI.xml request failed: url=") + base +
                        " HTTP=" + String(status) + " bytes=" + String(xml.length()));
    return;
  }
  const bool metadataSaved = cache_->saveMetadata(key, xml.c_str());
  logRadioDns(cache_, String("SI.xml received: bytes=") + xml.length() +
                      " saved=" + (metadataSaved ? "yes" : "no"));

  String extension;
  String logoUrl = findLogoUrl(xml, identity.bearer, extension);
  if (logoUrl.isEmpty()) {
    logRadioDns(cache_, String("no service bearer/logo match in SI.xml; bearer=") +
                        identity.bearer + " XML preview=" + xml.substring(0, 700));
    return;
  }
  logRadioDns(cache_, String("logo URL selected: ") + logoUrl +
                      " extension=" + extension);

  int logoStatus = -1;
  String logo = httpGet(logoUrl, kMaxLogoBytes, logoStatus);
  if (logoStatus != HTTP_CODE_OK || logo.isEmpty()) {
    logRadioDns(cache_, String("logo download failed: HTTP=") + String(logoStatus) +
                        " bytes=" + String(logo.length()) + " url=" + logoUrl);
    return;
  }
  if (cache_->saveAsset(key, extension.c_str(),
                       reinterpret_cast<const uint8_t*>(logo.c_str()), logo.length())) {
    logRadioDns(cache_, String("logo cached: /RDS/") + key + "." + extension +
                        " bytes=" + String(logo.length()));
  } else {
    logRadioDns(cache_, String("logo cache write failed: /RDS/") + key + "." + extension);
  }
}
