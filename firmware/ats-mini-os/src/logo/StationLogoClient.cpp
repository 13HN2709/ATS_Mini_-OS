#include "StationLogoClient.h"

#include <WiFi.h>
#include <LittleFS.h>
#include "../storage/StorageDiscovery.h"
#include "../network/WifiTestConfig.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_heap_caps.h>
#include <new>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

namespace {
constexpr size_t kMaxLogoBytes = 220000;
constexpr size_t kMaxSearchBytes = 45000;
struct LookupJob { StationLogoClient* self; RadioSnapshot state; };
struct TemporaryLogoWifi {
  wifi_mode_t previousMode = WiFi.getMode();
  bool restoreMode = false;

  TemporaryLogoWifi() {
    if (WiFi.isConnected()) return;
    if (previousMode == WIFI_OFF) {
      WiFi.mode(WIFI_STA);
      restoreMode = true;
    } else if (previousMode == WIFI_AP) {
      WiFi.mode(WIFI_AP_STA);
      restoreMode = true;
    }
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.begin(WifiTestConfig::SSID, WifiTestConfig::PASSWORD);
    const uint32_t started = millis();
    while (!WiFi.isConnected() && millis() - started < 15000)
      vTaskDelay(pdMS_TO_TICKS(250));
  }

  ~TemporaryLogoWifi() {
    if (restoreMode) {
      WiFi.disconnect(false, false);
      WiFi.mode(previousMode);
      Serial.println("[LOGO] temporary Wi-Fi stopped; previous mode restored");
    }
  }
};


String httpGet(const String& url, size_t maxBytes, int& status) {
  status = -1;
  WiFiClientSecure secure;
  secure.setInsecure();
  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(7000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setUserAgent("ATS-Mini-OS/0.1 (station logo lookup)");
  if (!http.begin(secure, url)) return String();
  status = http.GET();
  String body;
  if (status == HTTP_CODE_OK) {
    const int size = http.getSize();
    if (size < 0 || static_cast<size_t>(size) <= maxBytes) {
      body = http.getString();
      if (body.length() > maxBytes) body = String();
    }
  }
  http.end();
  return body;
}

bool isPng(const String& s) {
  static const uint8_t sig[8] = {137,80,78,71,13,10,26,10};
  return s.length() >= 24 && memcmp(reinterpret_cast<const uint8_t*>(s.c_str()), sig, 8) == 0;
}
bool isJpeg(const String& s) {
  return s.length() >= 4 && static_cast<uint8_t>(s[0]) == 0xFF &&
         static_cast<uint8_t>(s[1]) == 0xD8 &&
         static_cast<uint8_t>(s[s.length()-2]) == 0xFF &&
         static_cast<uint8_t>(s[s.length()-1]) == 0xD9;
}
String jsonString(const String& object, const char* key) {
  String token = String("\"") + key + "\":";
  int p = object.indexOf(token);
  if (p < 0) return String();
  p += token.length();
  while (p < static_cast<int>(object.length()) && object[p] == ' ') ++p;
  if (p >= static_cast<int>(object.length()) || object[p] != '"') return String();
  ++p;
  String value;
  bool escaped = false;
  for (; p < static_cast<int>(object.length()); ++p) {
    const char c = object[p];
    if (escaped) { value += c; escaped = false; continue; }
    if (c == '\\') { escaped = true; continue; }
    if (c == '"') break;
    value += c;
  }
  return value;
}
String normalizeName(String s) {
  s.toLowerCase();
  s.replace(" ", ""); s.replace("-", ""); s.replace("_", "");
  s.replace("!", ""); s.replace(".", ""); s.replace("radio", "");
  return s;
}
String expectedName(const RadioSnapshot& s) {
  String pi = String(s.piCode, HEX); pi.toUpperCase();
  if (pi == "D391") return "1LIVE";
  if (pi == "D392") return "WDR 3";
  if (pi == "D394") return "WDR 4";
  if (pi == "D3A0" || pi == "D3A1" || pi == "D3A2") return "WDR COSMO";
  return String(s.stationName);
}
String encodeQuery(const String& input) {
  String out;
  for (size_t i=0; i<input.length(); ++i) {
    const char c=input[i];
    if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_')
      out += c;
    else if (c==' ') out += "%20";
  }
  return out;
}
}

bool StationLogoClient::begin() {
  logoMutex_ = xSemaphoreCreateMutex();
  if (!logoMutex_) Serial.println("[LOGO] could not create RAM logo mutex");
  return logoMutex_ != nullptr;
}
bool StationLogoClient::lockLogo(const char* key, const uint8_t*& data, size_t& length, bool& isJpg) {
  data=nullptr; length=0; isJpg=false;
  if (!logoMutex_ || xSemaphoreTake(logoMutex_, pdMS_TO_TICKS(20)) != pdTRUE) return false;
  if (!key || strcmp(key, logoKey_) != 0 || !logoData_ || !logoLength_) {
    xSemaphoreGive(logoMutex_); return false;
  }
  data=logoData_; length=logoLength_; isJpg=logoIsJpg_;
  return true;
}
void StationLogoClient::unlockLogo() { if (logoMutex_) xSemaphoreGive(logoMutex_); }

bool StationLogoClient::storeLogoInRam(const String& logo, const char* key, const char* source) {
  const bool png=isPng(logo), jpg=isJpeg(logo);
  if ((!png && !jpg) || logo.length()==0 || logo.length()>kMaxLogoBytes) {
    Serial.printf("[LOGO] rejected non-PNG/JPEG image from %s\n", source ? source : "(unknown)");
    return false;
  }
  uint8_t* copy=static_cast<uint8_t*>(heap_caps_malloc(logo.length(), MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  if (!copy) copy=static_cast<uint8_t*>(heap_caps_malloc(logo.length(), MALLOC_CAP_8BIT));
  if (!copy) { Serial.println("[LOGO] not enough RAM for image"); return false; }
  memcpy(copy, logo.c_str(), logo.length());
  if (!logoMutex_ || xSemaphoreTake(logoMutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
    heap_caps_free(copy); return false;
  }
  if (logoData_) heap_caps_free(logoData_);
  logoData_=copy; logoLength_=logo.length(); logoIsJpg_=jpg;
  strncpy(logoKey_,key,sizeof(logoKey_)-1); logoKey_[sizeof(logoKey_)-1]=0;
  xSemaphoreGive(logoMutex_);

  if (source && strncmp(source, "https://", 8) == 0 && StorageDiscovery::ready() && key) {
    const String pi = String(key).substring(0, 4);
    const String cachePath = String("/logos/") + pi + "_REMOTE" + (png ? ".png" : ".jpg");
    if (!LittleFS.exists(cachePath)) {
      File cached = LittleFS.open(cachePath, "w");
      if (cached) {
        const size_t written = cached.write(reinterpret_cast<const uint8_t*>(logo.c_str()), logo.length());
        cached.flush();
        cached.close();
        File verify = LittleFS.open(cachePath, "r");
        const bool valid = written == logo.length() && verify && verify.size() == logo.length();
        if (verify) verify.close();
        if (!valid) LittleFS.remove(cachePath);
        else Serial.printf("[LOGO] remote logo cached locally: %s\\n", cachePath.c_str());
      }
    }
  }
  Serial.printf("[LOGO] RAM logo loaded from %s (%u bytes)\n",source?source:"unknown",static_cast<unsigned>(logo.length()));
  return true;
}
void StationLogoClient::update(const RadioSnapshot& state, bool wifiConnected) {
  (void)wifiConnected;
  if (!state.piCode || busy_) return;
  char key[40];
  snprintf(key,sizeof(key),"%04X_%lu",static_cast<unsigned>(state.piCode),static_cast<unsigned long>(state.frequencyKHz));
  if (haveRequested_ && strcmp(lastRequestKey_,key)==0 && millis()-lastAttemptMs_<60000) return;
  strncpy(lastRequestKey_,key,sizeof(lastRequestKey_)-1); lastRequestKey_[sizeof(lastRequestKey_)-1]=0;
  haveRequested_=true; lastAttemptMs_=millis();
  Serial.printf("[LOGO] lookup requested: PI=%04X frequency=%lu kHz LittleFS=%s\\n",
    static_cast<unsigned>(state.piCode), static_cast<unsigned long>(state.frequencyKHz),
    StorageDiscovery::ready() ? "ready" : "not-ready");
  LookupJob* job=new(std::nothrow) LookupJob{this,state};
  if (!job) return;
  busy_=true;
  if (xTaskCreate(taskEntry,"stationLogo",8192,job,1,nullptr)!=pdPASS) {
    delete job; busy_=false; Serial.println("[LOGO] could not start lookup task");
  }
}
void StationLogoClient::taskEntry(void* arg) {
  LookupJob* job=static_cast<LookupJob*>(arg);
  StationLogoClient* self=job->self; RadioSnapshot state=job->state; delete job;
  self->runLookup(state); self->busy_=false; vTaskDelete(nullptr);
}
void StationLogoClient::runLookup(const RadioSnapshot& state) {
  char key[40];
  snprintf(key,sizeof(key),"%04X_%lu",static_cast<unsigned>(state.piCode),static_cast<unsigned long>(state.frequencyKHz));
  const String pi=String(state.piCode,HEX); String piUpper=pi; piUpper.toUpperCase();

  bool localLogoFound = false;
  // Local LittleFS lookup always has priority. Accept files in both /logos
  // and the filesystem root so manually uploaded ATS Mini logo packs work.
  const char* logoDirectories[] = {"/logos", "/"};
  for (const char* directoryPath : logoDirectories) {
    File directory = LittleFS.open(directoryPath);
    if (!directory || !directory.isDirectory()) {
      if (directory) directory.close();
      continue;
    }
    File entry = directory.openNextFile();
    while (entry) {
      if (!entry.isDirectory()) {
        String path = entry.path();
        String filename = path;
        int slash = filename.lastIndexOf('/');
        if (slash >= 0) filename = filename.substring(slash + 1);
        filename.toUpperCase();
        const bool supported = filename.endsWith(".PNG") || filename.endsWith(".JPG") || filename.endsWith(".JPEG");
        if (supported && filename.indexOf(piUpper) >= 0 && entry.size() <= kMaxLogoBytes) {
          String image;
          image.reserve(entry.size());
          uint8_t chunk[256];
          while (entry.available()) {
            size_t n = entry.read(chunk, sizeof(chunk));
            for (size_t i = 0; i < n; ++i) image += static_cast<char>(chunk[i]);
          }
          entry.close();
          directory.close();
          if (storeLogoInRam(image, key, path.c_str())) {
            localLogoFound = true;
            Serial.printf("[LOGO] local PI match: %s\\n", path.c_str());
          }
          return;
        }
      }
      entry.close();
      entry = directory.openNextFile();
    }
    directory.close();
  }


  if (!localLogoFound) Serial.printf("[LOGO] no local file matched PI %s in /logos or /\\n", piUpper.c_str());

  TemporaryLogoWifi temporaryWifi;
  if (!WiFi.isConnected()) {
    Serial.println("[LOGO] Wi-Fi connection unavailable; remote lookup skipped");
    return;
  }
  // FM-DX Station Logo plugin lookup order: PI_PROGRAM first, then PI.
  // Program suffixes are uppercase and stripped of spaces/punctuation exactly
  // as in the upstream plugin. The directory's SVG logos are intentionally
  // skipped here because the ESP32 display decoder accepts PNG/JPEG only.
  if (piUpper.length()==4 && piUpper[0]=='D') {
    String programs[3] = { expectedName(state), String(state.stationName), String() };
    programs[2] = String(state.radioText);
    for (String program : programs) {
      program.toUpperCase();
      String clean;
      for (size_t i=0; i<program.length(); ++i) {
        const char ch=program[i];
        if ((ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')) clean+=ch;
      }
      if (clean.length()<2) continue;
      const String url=String("https://tef.noobish.eu/logos/D/")+piUpper+"_"+clean+".png";
      int status=-1; String image=httpGet(url,kMaxLogoBytes,status);
      if (status==HTTP_CODE_OK && storeLogoInRam(image,key,url.c_str())) return;
    }
    const char* exts[]={".png",".jpg",".jpeg"};
    for (const char* ext : exts) {
      const String url=String("https://tef.noobish.eu/logos/D/")+piUpper+ext;
      int status=-1; String image=httpGet(url,kMaxLogoBytes,status);
      if (status==HTTP_CODE_OK && storeLogoInRam(image,key,url.c_str())) return;
    }
  }
  // Radio Browser fallback: query exact station name in Germany and only accept
  // an exact normalized name match with a declared PNG/JPEG favicon.
  const String station=expectedName(state);
  if (station.length()<3) { Serial.println("[LOGO] no usable RDS station name for Radio Browser fallback"); return; }
  const String query=String("https://de1.api.radio-browser.info/json/stations/search?name=")+encodeQuery(station)+
    "&countrycode=DE&limit=12&hidebroken=true";
  int status=-1; String json=httpGet(query,kMaxSearchBytes,status);
  if (status!=HTTP_CODE_OK || json.isEmpty()) {
    Serial.printf("[LOGO] Radio Browser search failed, HTTP/status=%d\n",status); return;
  }
  const String target=normalizeName(station);
  int cursor=0;
  while ((cursor=json.indexOf('{',cursor))>=0) {
    int end=json.indexOf('}',cursor); if(end<0) break;
    const String item=json.substring(cursor,end+1); cursor=end+1;
    const String name=jsonString(item,"name");
    const String country=jsonString(item,"countrycode");
    String favicon=jsonString(item,"favicon");
    if (normalizeName(name)!=target || country!="DE" || favicon.length()<8) continue;
    if (!(favicon.startsWith("https://") || favicon.startsWith("http://"))) continue;
    int imageStatus=-1; String image=httpGet(favicon,kMaxLogoBytes,imageStatus);
    if (imageStatus==HTTP_CODE_OK && storeLogoInRam(image,key,favicon.c_str())) return;
  }
  Serial.printf("[LOGO] no exact PNG/JPEG favicon found for '%s' in Radio Browser\n",station.c_str());
}
