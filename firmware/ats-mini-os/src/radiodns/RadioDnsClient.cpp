#include "RadioDnsClient.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ctype.h>
#include <new>
#include <esp_heap_caps.h>
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

enum class DnsSrvResult {
  Success,
  NoDnsServer,
  UdpBeginFailed,
  SendFailed,
  Timeout,
  InvalidResponse,
  NameError,
  ServerFailure,
  Refused,
  OtherDnsError,
  NoSrvRecord,
  NoRequestedRecord,
  MalformedResponse
};

const char* dnsSrvResultText(DnsSrvResult result) {
  switch (result) {
    case DnsSrvResult::Success: return "success";
    case DnsSrvResult::NoDnsServer: return "no DNS server configured";
    case DnsSrvResult::UdpBeginFailed: return "UDP socket could not start";
    case DnsSrvResult::SendFailed: return "DNS query could not be sent";
    case DnsSrvResult::Timeout: return "DNS response timeout";
    case DnsSrvResult::InvalidResponse: return "invalid or mismatched DNS response";
    case DnsSrvResult::NameError: return "DNS NXDOMAIN (name does not exist)";
    case DnsSrvResult::ServerFailure: return "DNS SERVFAIL";
    case DnsSrvResult::Refused: return "DNS query refused";
    case DnsSrvResult::OtherDnsError: return "DNS error response";
    case DnsSrvResult::NoSrvRecord: return "DNS response has no SRV answer";
    case DnsSrvResult::NoRequestedRecord: return "DNS response has no requested record";
    case DnsSrvResult::MalformedResponse: return "malformed DNS response";
  }
  return "unknown DNS result";
}


bool decodeDnsName(const uint8_t* packet, size_t length, size_t& pos,
                   char* output, size_t outputCap) {
  if (!output || outputCap == 0) return false;
  output[0] = 0;
  size_t cursor = pos;
  size_t consumed = 0;
  bool jumped = false;
  size_t written = 0;
  for (size_t guard = 0; guard < 128 && cursor < length; ++guard) {
    const uint8_t n = packet[cursor++];
    if (n == 0) {
      if (!jumped) consumed = cursor - pos;
      pos += consumed;
      output[written] = 0;
      return true;
    }
    if ((n & 0xC0) == 0xC0) {
      if (cursor >= length) return false;
      const size_t ptr = (size_t(n & 0x3F) << 8) | packet[cursor++];
      if (ptr >= length) return false;
      if (!jumped) { consumed = cursor - pos; jumped = true; }
      cursor = ptr;
      continue;
    }
    if ((n & 0xC0) != 0 || cursor + n > length) return false;
    if (written && written + 1 >= outputCap) return false;
    if (written) output[written++] = '.';
    if (written + n >= outputCap) return false;
    for (uint8_t i = 0; i < n; ++i) {
      char ch = static_cast<char>(packet[cursor++]);
      output[written++] = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
  }
  return false;
}

DnsSrvResult dnsCnameLookup(const char* fqdn, char* target, size_t targetCap,
                            uint8_t& responseCode, uint16_t& answerCount) {
  target[0] = 0;
  responseCode = 0xff;
  answerCount = 0;
  IPAddress dns = WiFi.dnsIP(0);
  if (dns == IPAddress(0,0,0,0)) return DnsSrvResult::NoDnsServer;
  uint8_t packet[kDnsPacketSize] = {};
  const uint16_t id = static_cast<uint16_t>(esp_random());
  packet[0] = id >> 8; packet[1] = id & 0xff;
  packet[2] = 1; packet[5] = 1;
  size_t q = 12;
  if (!encodeDnsName(packet, sizeof(packet), q, fqdn) || q + 4 > sizeof(packet))
    return DnsSrvResult::MalformedResponse;
  packet[q++] = 0; packet[q++] = 5;
  packet[q++] = 0; packet[q++] = 1;
  WiFiUDP udp;
  if (!udp.begin(0)) return DnsSrvResult::UdpBeginFailed;
  if (!udp.beginPacket(dns, 53) || udp.write(packet, q) != q || !udp.endPacket()) {
    udp.stop(); return DnsSrvResult::SendFailed;
  }
  const uint32_t started = millis();
  int packetSize = 0;
  while (millis() - started < 1800) {
    packetSize = udp.parsePacket();
    if (packetSize > 0) break;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  if (packetSize <= 0) { udp.stop(); return DnsSrvResult::Timeout; }
  if (packetSize > int(sizeof(packet))) {
    while (udp.available()) udp.read();
    udp.stop(); return DnsSrvResult::MalformedResponse;
  }
  const int got = udp.read(packet, sizeof(packet));
  udp.stop();
  if (got < 12 || read16(packet) != id || !(packet[2] & 0x80))
    return DnsSrvResult::InvalidResponse;
  responseCode = packet[3] & 0x0f;
  answerCount = read16(packet + 6);
  if (responseCode == 3) return DnsSrvResult::NameError;
  if (responseCode == 2) return DnsSrvResult::ServerFailure;
  if (responseCode == 5) return DnsSrvResult::Refused;
  if (responseCode != 0) return DnsSrvResult::OtherDnsError;
  if (packet[2] & 0x02) return DnsSrvResult::MalformedResponse;
  size_t pos = 12;
  const uint16_t questions = read16(packet + 4);
  for (uint16_t i = 0; i < questions; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 4 > size_t(got))
      return DnsSrvResult::MalformedResponse;
    pos += 4;
  }
  for (uint16_t i = 0; i < answerCount; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 10 > size_t(got))
      return DnsSrvResult::MalformedResponse;
    const uint16_t type = read16(packet + pos);
    const uint16_t klass = read16(packet + pos + 2);
    const uint16_t rdlen = read16(packet + pos + 8);
    pos += 10;
    if (pos + rdlen > size_t(got)) return DnsSrvResult::MalformedResponse;
    if (type == 5 && klass == 1 && rdlen > 0) {
      size_t namePos = pos;
      if (decodeDnsName(packet, got, namePos, target, targetCap) && target[0])
        return DnsSrvResult::Success;
      return DnsSrvResult::MalformedResponse;
    }
    pos += rdlen;
  }
  return DnsSrvResult::NoRequestedRecord;
}

DnsSrvResult dnsSrvLookup(const char* fqdn, char* host, size_t hostCap,
                          uint16_t& port, uint8_t& responseCode,
                          uint16_t& answerCount) {
  host[0] = 0;
  port = 0;
  responseCode = 0xff;
  answerCount = 0;
  IPAddress dns = WiFi.dnsIP(0);
  if (dns == IPAddress(0,0,0,0)) return DnsSrvResult::NoDnsServer;

  uint8_t packet[kDnsPacketSize] = {};
  const uint16_t id = static_cast<uint16_t>(esp_random());
  packet[0] = id >> 8; packet[1] = id & 0xff;
  packet[2] = 1; packet[5] = 1; // recursive query, one question
  size_t q = 12;
  char query[100];
  snprintf(query, sizeof(query), "_radioepg._tcp.%s", fqdn);
  if (!encodeDnsName(packet, sizeof(packet), q, query) || q + 4 > sizeof(packet))
    return DnsSrvResult::MalformedResponse;
  packet[q++] = 0; packet[q++] = 33; // SRV
  packet[q++] = 0; packet[q++] = 1;  // IN

  WiFiUDP udp;
  if (!udp.begin(0)) return DnsSrvResult::UdpBeginFailed;
  if (!udp.beginPacket(dns, 53) || udp.write(packet, q) != q || !udp.endPacket()) {
    udp.stop();
    return DnsSrvResult::SendFailed;
  }

  const uint32_t started = millis();
  int packetSize = 0;
  while (millis() - started < 1800) {
    packetSize = udp.parsePacket();
    if (packetSize > 0) break;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  if (packetSize <= 0) {
    udp.stop();
    return DnsSrvResult::Timeout;
  }
  if (packetSize > int(sizeof(packet))) {
    while (udp.available()) udp.read();
    udp.stop();
    return DnsSrvResult::MalformedResponse;
  }
  const int got = udp.read(packet, sizeof(packet));
  udp.stop();
  if (got < 12 || read16(packet) != id || !(packet[2] & 0x80))
    return DnsSrvResult::InvalidResponse;

  responseCode = packet[3] & 0x0f;
  answerCount = read16(packet + 6);
  if (responseCode == 3) return DnsSrvResult::NameError;
  if (responseCode == 2) return DnsSrvResult::ServerFailure;
  if (responseCode == 5) return DnsSrvResult::Refused;
  if (responseCode != 0) return DnsSrvResult::OtherDnsError;
  if (packet[2] & 0x02) return DnsSrvResult::MalformedResponse; // truncated UDP response

  const uint16_t questions = read16(packet + 4);
  size_t pos = 12;
  for (uint16_t i = 0; i < questions; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 4 > size_t(got))
      return DnsSrvResult::MalformedResponse;
    pos += 4;
  }
  for (uint16_t i = 0; i < answerCount; ++i) {
    if (!skipDnsName(packet, got, pos) || pos + 10 > size_t(got))
      return DnsSrvResult::MalformedResponse;
    const uint16_t type = read16(packet + pos);
    const uint16_t klass = read16(packet + pos + 2);
    const uint16_t rdlen = read16(packet + pos + 8);
    pos += 10;
    if (pos + rdlen > size_t(got)) return DnsSrvResult::MalformedResponse;
    if (type == 33 && klass == 1 && rdlen >= 7) {
      const uint16_t candidatePort = read16(packet + pos + 4);
      size_t np = pos + 6;
      const size_t recordEnd = pos + rdlen;
      String target;
      while (np < recordEnd) {
        uint8_t n = packet[np++];
        if (!n) break;
        if ((n & 0xC0) == 0xC0) {
          if (np >= recordEnd) break;
          size_t ptr = ((n & 0x3f) << 8) | packet[np++];
          size_t guard = 0;
          while (ptr < size_t(got) && guard++ < 32) {
            uint8_t label = packet[ptr++];
            if (!label) break;
            if ((label & 0xC0) == 0xC0) {
              if (ptr >= size_t(got)) break;
              size_t next = ((label & 0x3f) << 8) | packet[ptr++];
              ptr = next;
              continue;
            }
            if ((label & 0xC0) || ptr + label > size_t(got)) break;
            if (target.length()) target += '.';
            for (uint8_t j = 0; j < label; ++j) target += char(packet[ptr++]);
          }
          break;
        }
        if ((n & 0xC0) || np + n > recordEnd) break;
        if (target.length()) target += '.';
        for (uint8_t j = 0; j < n; ++j) target += char(packet[np++]);
      }
      if (target.length() && target != "." && target.length() < hostCap && candidatePort) {
        target.toCharArray(host, hostCap);
        while (host[0] && host[strlen(host) - 1] == '.')
          host[strlen(host) - 1] = 0;
        if (host[0]) {
          port = candidatePort;
          return DnsSrvResult::Success;
        }
      }
    }
    pos += rdlen;
  }
  return DnsSrvResult::NoSrvRecord;
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
  if (!logoMutex_) logoMutex_ = xSemaphoreCreateMutex();
  return cache_ != nullptr && logoMutex_ != nullptr;
}

bool RadioDnsClient::lockLogo(const char* cacheKey, const uint8_t*& data,
                              size_t& length, bool& isJpg) {
  data = nullptr;
  length = 0;
  isJpg = false;
  if (!logoMutex_ || xSemaphoreTake(logoMutex_, portMAX_DELAY) != pdTRUE)
    return false;
  if (!logoData_ || !logoLength_ || !cacheKey || strcmp(logoKey_, cacheKey) != 0) {
    xSemaphoreGive(logoMutex_);
    return false;
  }
  data = logoData_;
  length = logoLength_;
  isJpg = logoIsJpg_;
  return true;
}

void RadioDnsClient::unlockLogo() {
  if (logoMutex_) xSemaphoreGive(logoMutex_);
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
  // Diagnostic phase: deliberately bypass the persistent asset cache. Existing
  // files are left untouched; each test lookup downloads into RAM only.

  char authoritativeFqdn[256] = {};
  uint8_t cnameRcode = 0xff;
  uint16_t cnameAnswers = 0;
  const DnsSrvResult cnameResult = dnsCnameLookup(
      identity.fqdn, authoritativeFqdn, sizeof(authoritativeFqdn),
      cnameRcode, cnameAnswers);
  if (cnameResult != DnsSrvResult::Success) {
    logRadioDns(cache_, String("CNAME DNS lookup failed: result=") + dnsSrvResultText(cnameResult) +
      "; fqdn=" + identity.fqdn + "; query=" + identity.fqdn +
      "; DNS-server=" + WiFi.dnsIP(0).toString() +
      "; rcode=" + (cnameRcode == 0xff ? String("n/a") : String(unsigned(cnameRcode))) +
      "; answers=" + String(unsigned(cnameAnswers)) +
      "; no authoritative FQDN discovered");
    return;
  }
  logRadioDns(cache_, String("CNAME DNS lookup succeeded: fqdn=") + identity.fqdn +
    "; authoritative-FQDN=" + authoritativeFqdn +
    "; answers=" + String(unsigned(cnameAnswers)));

  char host[256] = {};
  uint16_t port = 0;
  uint8_t dnsRcode = 0xff;
  uint16_t dnsAnswers = 0;
  const DnsSrvResult dnsResult =
      dnsSrvLookup(authoritativeFqdn, host, sizeof(host), port, dnsRcode, dnsAnswers);
  if (dnsResult != DnsSrvResult::Success) {
    logRadioDns(cache_, String("SRV DNS lookup failed: result=") + dnsSrvResultText(dnsResult) +
      "; fqdn=" + identity.fqdn +
      "; query=_radioepg._tcp." + String(authoritativeFqdn) +
      "; DNS-server=" + WiFi.dnsIP(0).toString() +
      "; rcode=" + (dnsRcode == 0xff ? String("n/a") : String(unsigned(dnsRcode))) +
      "; answers=" + String(unsigned(dnsAnswers)));
    return;
  }
  logRadioDns(cache_, String("SRV DNS lookup succeeded: answers=") + String(unsigned(dnsAnswers)) +
    "; metadata-server=" + host + ":" + String(unsigned(port)));
  String base = String("http://") + host + ":" + String(port) + "/radiodns/spi/3.1/SI.xml";
  int status = -1;
  String xml = httpGet(base, kMaxSiBytes, status);
  if (status != HTTP_CODE_OK || xml.isEmpty()) {
    logRadioDns(cache_, String("SI.xml request failed (HTTP/status ") + String(status) + "); URL=" + base);
    return;
  }
  logRadioDns(cache_, String("SI.xml received bytes=") + xml.length() + " persistence=disabled (RAM-only test)");

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
  uint8_t* ramCopy = static_cast<uint8_t*>(
    heap_caps_malloc(logo.length(), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!ramCopy) ramCopy = static_cast<uint8_t*>(heap_caps_malloc(logo.length(), MALLOC_CAP_8BIT));
  if (!ramCopy) {
    logRadioDns(cache_, String("logo downloaded but RAM allocation failed; bytes=") + String(logo.length()));
    return;
  }
  memcpy(ramCopy, logo.c_str(), logo.length());
  if (!logoMutex_ || xSemaphoreTake(logoMutex_, portMAX_DELAY) != pdTRUE) {
    heap_caps_free(ramCopy);
    logRadioDns(cache_, "logo downloaded but RAM handoff mutex unavailable");
    return;
  }
  uint8_t* oldLogo = logoData_;
  logoData_ = ramCopy;
  logoLength_ = logo.length();
  logoIsJpg_ = strcmp(ext, "jpg") == 0;
  strncpy(logoKey_, key, sizeof(logoKey_) - 1);
  logoKey_[sizeof(logoKey_) - 1] = 0;
  xSemaphoreGive(logoMutex_);
  if (oldLogo) heap_caps_free(oldLogo);
  logRadioDns(cache_, String("logo ready in RAM for display: key=") + key +
    "; format=" + ext + "; bytes=" + String(logo.length()) +
    "; persistent saving disabled");
}
