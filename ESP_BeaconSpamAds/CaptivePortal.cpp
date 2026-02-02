#include "CaptivePortal.h"
#include "Storage.h"
#include "web_index.h"

#ifdef ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif

// ===== Security Configuration =====
#define MAX_FAILED_ATTEMPTS 5    // Max failed login attempts before lockout
#define LOCKOUT_DURATION 300000  // Lockout duration in ms (5 minutes)
#define MIN_REQUEST_INTERVAL 100 // Min ms between requests (rate limiting)
#define MAX_INPUT_LENGTH 128     // Max length for user input fields
#define MAX_SSID_LENGTH 32       // Max SSID length per WiFi spec

// Security state tracking
static uint8_t failedAttempts = 0;
static unsigned long lockoutStart = 0;
static unsigned long lastRequestTime = 0;
static String lastClientIP = "";

// Security helper functions
bool CaptivePortal::isLockedOut() {
  if (lockoutStart > 0) {
    if (millis() - lockoutStart < LOCKOUT_DURATION) {
      return true;
    }
    // Lockout expired, reset
    lockoutStart = 0;
    failedAttempts = 0;
  }
  return false;
}

void CaptivePortal::recordFailedAttempt() {
  failedAttempts++;
  if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
    lockoutStart = millis();
  }
}

void CaptivePortal::resetFailedAttempts() {
  failedAttempts = 0;
  lockoutStart = 0;
}

bool CaptivePortal::isRateLimited() {
  unsigned long now = millis();
  if (now - lastRequestTime < MIN_REQUEST_INTERVAL) {
    return true;
  }
  lastRequestTime = now;
  return false;
}

void CaptivePortal::addSecurityHeaders() {
  server.sendHeader("X-Content-Type-Options", "nosniff");
  server.sendHeader("X-Frame-Options", "DENY");
  server.sendHeader("X-XSS-Protection", "1; mode=block");
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
}

String CaptivePortal::sanitizeInput(const String &input, size_t maxLen) {
  String result = input;
  // Truncate to max length
  if (result.length() > maxLen) {
    result = result.substring(0, maxLen);
  }
  // Remove potentially dangerous characters
  result.replace("<", "&lt;");
  result.replace(">", "&gt;");
  result.replace("\"", "&quot;");
  result.replace("'", "&#39;");
  result.replace("\\", "\\\\");
  return result;
}

bool CaptivePortal::authenticateAdmin() {
  // Check if locked out
  if (isLockedOut()) {
    addSecurityHeaders();
    server.send(429, "text/plain",
                "Too many failed attempts. Try again later.");
    return false;
  }

  // Check rate limiting
  if (isRateLimited()) {
    addSecurityHeaders();
    server.send(429, "text/plain", "Too many requests. Slow down.");
    return false;
  }

  // Attempt authentication
  if (!server.authenticate(storage.config.adminUser,
                           storage.config.adminPass)) {
    recordFailedAttempt();
    return false;
  }

  // Success - reset failed attempts
  resetFailedAttempts();
  return true;
}

void CaptivePortal::setup() {
  // start WiFi
  WiFi.mode(WIFI_AP_STA); // AP for Portal, STA for Beacon Spam

  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  WiFi.softAP(storage.config.apName, NULL, 1, storage.config.hideAP);

  // Setup DNS for Captive Portal - redirect ALL domains
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

  // ===== Captive Portal Detection URLs =====
  // Each handler returns appropriate response to trigger sign-in popup

  // Apple iOS/macOS detection (all versions)
  server.on("/hotspot-detect.html", handleCaptiveApple);
  server.on("/library/test/success.html", handleCaptiveApple);
  server.on("/captive.apple.com", handleCaptiveApple);
  server.on("/captive.apple.com/hotspot-detect.html", handleCaptiveApple);

  // Google Android detection (all versions including newer)
  server.on("/generate_204", handleCaptiveAndroid);
  server.on("/gen_204", handleCaptiveAndroid);
  server.on("/mobile/status.php", handleCaptiveAndroid);
  server.on("/connectivity-check.html", handleCaptiveAndroid);
  server.on("/connectivitycheck/gstatic/generate_204", handleCaptiveAndroid);
  server.on("/clients3.google.com/generate_204", handleCaptiveAndroid);

  // Samsung devices
  server.on("/check_network.html", handleCaptiveAndroid);
  server.on("/connectivitycheck.android.com/generate_204",
            handleCaptiveAndroid);

  // Xiaomi devices
  server.on("/connect/wifi_portal.html", handleCaptiveAndroid);

  // Microsoft Windows 10/11 detection
  server.on("/ncsi.txt", handleCaptiveWindows);
  server.on("/connecttest.txt", handleCaptiveWindows);
  server.on("/redirect", handleCaptiveWindows);
  server.on("/fwlink", handleCaptiveWindows);
  server.on("/msftconnecttest.com/connecttest.txt", handleCaptiveWindows);
  server.on("/www.msftconnecttest.com/connecttest.txt", handleCaptiveWindows);
  server.on("/ipv6.msftconnecttest.com/connecttest.txt", handleCaptiveWindows);
  server.on("/www.msftncsi.com/ncsi.txt", handleCaptiveWindows);

  // Firefox browser
  server.on("/success.txt", handleCaptiveFirefox);
  server.on("/canonical.html", handleCaptiveFirefox);
  server.on("/detectportal.firefox.com/success.txt", handleCaptiveFirefox);

  // Chrome browser
  server.on("/chrome/generate_204", handleCaptiveAndroid);

  // Main portal
  server.on("/", handleRoot);

  // Admin Routes - Auth handled in callbacks
  server.on("/admin", handleAdmin);
  server.on("/api/data", handleGetData);
  server.on("/api/public_data", handlePublicData);
  server.on("/api/save_config", HTTP_POST, handleSaveConfig);
  server.on("/api/save_ssids", HTTP_POST, handleSaveSSIDs);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/redirect", handleRedirect);
  server.on("/api/clients", handleGetClients);
  server.on("/api/export", handleExportConfig);
  server.on("/api/import", HTTP_POST, handleImportConfig);

  server.onNotFound(handleNotFound);
  server.begin();
}

void CaptivePortal::update() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void CaptivePortal::handleRoot() {
  if (server.hostHeader() != "192.168.4.1") {
    server.sendHeader("Location", String("http://192.168.4.1/"), true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(200, "text/html", portal_html);
}

void CaptivePortal::handleNotFound() {
  // Redirect everything unknown to trigger portal
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.send(302, "text/plain", "");
}

// ===== Helper for Captive Portal Headers =====
// Adds headers that signal to devices this is a captive portal,
// forcing the sign-in popup to open automatically
void CaptivePortal::addCaptivePortalHeaders() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.sendHeader("X-Captive-Portal", "true");
  server.sendHeader("Connection", "close");
}

// ===== Platform-Specific Captive Portal Handlers =====
// All handlers serve the portal HTML directly to trigger the sign-in popup

// Apple iOS/macOS - expects specific success page
// Returning our portal page triggers the sign-in popup
void CaptivePortal::handleCaptiveApple() {
  addCaptivePortalHeaders();
  server.send(200, "text/html", portal_html);
}

// Android - expects HTTP 204 No Content
// Returning our portal page triggers the sign-in popup
void CaptivePortal::handleCaptiveAndroid() {
  addCaptivePortalHeaders();
  server.send(200, "text/html", portal_html);
}

// Windows 10/11 - expects specific test response
// Returning our portal page triggers the sign-in popup
void CaptivePortal::handleCaptiveWindows() {
  addCaptivePortalHeaders();
  server.send(200, "text/html", portal_html);
}

// Firefox - expects "success\n"
// Returning our portal page triggers captive portal notification
void CaptivePortal::handleCaptiveFirefox() {
  addCaptivePortalHeaders();
  server.send(200, "text/html", portal_html);
}

// Legacy handler kept for compatibility
void CaptivePortal::handleCaptivePortal() {
  addCaptivePortalHeaders();
  server.send(200, "text/html", portal_html);
}

// Admin Handlers

void CaptivePortal::handleAdmin() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();
  server.send(200, "text/html", admin_html);
}

void CaptivePortal::handleGetData() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();
  String json = "{";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"clientCount\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"wpa2\":" + String(storage.config.wpa2 ? "true" : "false") + ",";
  json += "\"appendSpaces\":" +
          String(storage.config.appendSpaces ? "true" : "false") + ",";
  json += json +=
      "\"enableBLE\":" + String(storage.config.enableBLE ? "true" : "false") +
      ",";
  json += "\"beaconInterval\":" + String(storage.config.beaconInterval) + ",";

  // New Fields
  json += "\"adminUser\":\"" + String(storage.config.adminUser) + "\",";
  json += "\"adminPass\":\"" + String(storage.config.adminPass) + "\",";
  json += "\"redirectUrl\":\"" + String(storage.config.redirectUrl) + "\",";
  json += "\"advertisingHeadline\":\"" +
          String(storage.config.advertisingHeadline) + "\",";
  json += "\"advertisingDescription\":\"" +
          String(storage.config.advertisingDescription) + "\",";
  json += "\"buttonText\":\"" + String(storage.config.buttonText) + "\",";
  json +=
      "\"autoRedirectDelay\":" + String(storage.config.autoRedirectDelay) + ",";
  json += "\"apName\":\"" + String(storage.config.apName) + "\",";
  json +=
      "\"hideAP\":" + String(storage.config.hideAP ? "true" : "false") + ",";

  // Advanced settings
  json += "\"wifiChannel\":" + String(storage.config.wifiChannel) + ",";
  json += "\"randomizeMAC\":" +
          String(storage.config.randomizeMAC ? "true" : "false") + ",";

  json += "\"ssidCount\":" + String(storage.ssids.size()) + ",";

  json += "\"ssids\":\"";
  for (size_t i = 0; i < storage.ssids.size(); i++) {
    String s = storage.ssids[i];
    s.replace("\n", "\\n");  // Escape newlines
    s.replace("\"", "\\\""); // Escape quotes
    json += s;
    if (i < storage.ssids.size() - 1)
      json += "\\n";
  }
  json += "\"";

  json += "}";
  server.send(200, "application/json", json);
}

void CaptivePortal::handlePublicData() {
  String json = "{";
  json += "\"advertisingHeadline\":\"" +
          String(storage.config.advertisingHeadline) + "\",";
  json += "\"advertisingDescription\":\"" +
          String(storage.config.advertisingDescription) + "\",";
  json += "\"buttonText\":\"" + String(storage.config.buttonText) + "\",";
  json +=
      "\"autoRedirectDelay\":" + String(storage.config.autoRedirectDelay) + ",";

  String rUrl = String(storage.config.redirectUrl);
  if (rUrl.length() == 0)
    rUrl = "https://google.com";
  json += "\"redirectUrl\":\"" + rUrl + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void CaptivePortal::handleSaveConfig() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "Missing data");
    return;
  }

  String data = server.arg("data");

  // Simple manual parsing to avoid JSON library dependency
  storage.config.wpa2 = data.indexOf("\"wpa2\":true") > 0;
  storage.config.appendSpaces = data.indexOf("\"appendSpaces\":true") > 0;
  storage.config.enableBLE = data.indexOf("\"enableBLE\":true") > 0;

  int intervalIdx = data.indexOf("\"beaconInterval\":");
  if (intervalIdx > 0) {
    int endIdx = data.indexOf("}", intervalIdx);
    if (endIdx < 0)
      endIdx = data.indexOf(",", intervalIdx);
    String val = data.substring(intervalIdx + 17, endIdx);
    storage.config.beaconInterval = val.toInt();
  }

  // Manual string parsing for new fields strings (simplified)
  // Logic: find key, find next quote, find closing quote
  auto extractStr = [&](const char *key, char *dest, size_t len) {
    String k = "\"" + String(key) + "\":\"";
    int start = data.indexOf(k);
    if (start > 0) {
      start += k.length();
      int end = data.indexOf("\"", start);
      if (end > start) {
        String val = data.substring(start, end);
        strlcpy(dest, val.c_str(), len);
      }
    }
  };

  extractStr("adminUser", storage.config.adminUser,
             sizeof(storage.config.adminUser));
  extractStr("adminPass", storage.config.adminPass,
             sizeof(storage.config.adminPass));
  extractStr("redirectUrl", storage.config.redirectUrl,
             sizeof(storage.config.redirectUrl));
  extractStr("advertisingHeadline", storage.config.advertisingHeadline,
             sizeof(storage.config.advertisingHeadline));
  extractStr("advertisingDescription", storage.config.advertisingDescription,
             sizeof(storage.config.advertisingDescription));
  extractStr("buttonText", storage.config.buttonText,
             sizeof(storage.config.buttonText));
  extractStr("apName", storage.config.apName, sizeof(storage.config.apName));

  // Parse integer: autoRedirectDelay
  int delayIdx = data.indexOf("\"autoRedirectDelay\":");
  if (delayIdx > 0) {
    int endIdx = data.indexOf(",", delayIdx);
    if (endIdx < 0)
      endIdx = data.indexOf("}", delayIdx);
    String val = data.substring(delayIdx + 20, endIdx);
    storage.config.autoRedirectDelay = val.toInt();
  }

  // Parse boolean: hideAP
  storage.config.hideAP = data.indexOf("\"hideAP\":true") > 0;

  // Parse integer: wifiChannel
  int channelIdx = data.indexOf("\"wifiChannel\":");
  if (channelIdx > 0) {
    int endIdx = data.indexOf(",", channelIdx);
    if (endIdx < 0)
      endIdx = data.indexOf("}", channelIdx);
    String val = data.substring(channelIdx + 14, endIdx);
    int ch = val.toInt();
    if (ch >= 1 && ch <= 14)
      storage.config.wifiChannel = ch;
  }

  // Parse boolean: randomizeMAC
  storage.config.randomizeMAC = data.indexOf("\"randomizeMAC\":true") > 0;

  storage.saveConfig();
  server.send(200, "text/plain", "Config Saved! Reboot to apply changes.");
}

void CaptivePortal::handleSaveSSIDs() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  if (!server.hasArg("ssids")) {
    server.send(400, "text/plain", "Missing ssids");
    return;
  }

  String ssids = server.arg("ssids");
  storage.saveSSIDs(ssids);
  server.send(200, "text/plain",
              "SSIDs Saved (" + String(storage.ssids.size()) + ")");
}

void CaptivePortal::handleReboot() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();
  server.send(200, "text/plain", "Rebooting...");
  delay(500);
  ESP.restart();
}

void CaptivePortal::handleRedirect() {
  // Redirect to the configured URL
  // We use a meta-refresh or JS redirect to be safe
  String url = String(storage.config.redirectUrl);
  if (url.length() == 0)
    url = "http://google.com";

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv=\"refresh\" content=\"0; url=" + url + "\" />";
  html += "<script>window.location.href='" + url + "';</script>";
  html += "</head><body>Redirecting to <a href=\"" + url + "\">" + url +
          "</a>...</body></html>";

  server.send(200, "text/html", html);
}

void CaptivePortal::handleGetClients() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  String json = "{\"clients\":[";

#ifdef ESP32
  wifi_sta_list_t stationList;
  tcpip_adapter_sta_list_t adapterList;
  esp_wifi_ap_get_sta_list(&stationList);
  tcpip_adapter_get_sta_list(&stationList, &adapterList);

  for (int i = 0; i < adapterList.num; i++) {
    if (i > 0)
      json += ",";
    tcpip_adapter_sta_info_t station = adapterList.sta[i];
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0],
            station.mac[1], station.mac[2], station.mac[3], station.mac[4],
            station.mac[5]);
    char ipStr[16];
    sprintf(ipStr, IPSTR, IP2STR(&station.ip));
    json +=
        "{\"mac\":\"" + String(macStr) + "\",\"ip\":\"" + String(ipStr) + "\"}";
  }
#else
  struct station_info *stat_info = wifi_softap_get_station_info();
  int i = 0;
  while (stat_info != NULL) {
    if (i > 0)
      json += ",";
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", stat_info->bssid[0],
            stat_info->bssid[1], stat_info->bssid[2], stat_info->bssid[3],
            stat_info->bssid[4], stat_info->bssid[5]);
    IPAddress ip = stat_info->ip;
    json +=
        "{\"mac\":\"" + String(macStr) + "\",\"ip\":\"" + ip.toString() + "\"}";
    stat_info = STAILQ_NEXT(stat_info, next);
    i++;
  }
  wifi_softap_free_station_info();
#endif

  json += "],\"total\":" + String(WiFi.softAPgetStationNum()) + "}";
  server.send(200, "application/json", json);
}

void CaptivePortal::handleExportConfig() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  String json = "{";
  json += "\"wpa2\":" + String(storage.config.wpa2 ? "true" : "false") + ",";
  json += "\"appendSpaces\":" +
          String(storage.config.appendSpaces ? "true" : "false") + ",";
  json +=
      "\"enableBLE\":" + String(storage.config.enableBLE ? "true" : "false") +
      ",";
  json += "\"beaconInterval\":" + String(storage.config.beaconInterval) + ",";
  json += "\"adminUser\":\"" + String(storage.config.adminUser) + "\",";
  json += "\"adminPass\":\"" + String(storage.config.adminPass) + "\",";
  json += "\"advertisingHeadline\":\"" +
          String(storage.config.advertisingHeadline) + "\",";
  json += "\"advertisingDescription\":\"" +
          String(storage.config.advertisingDescription) + "\",";
  json += "\"buttonText\":\"" + String(storage.config.buttonText) + "\",";
  json += "\"redirectUrl\":\"" + String(storage.config.redirectUrl) + "\",";
  json +=
      "\"autoRedirectDelay\":" + String(storage.config.autoRedirectDelay) + ",";
  json += "\"apName\":\"" + String(storage.config.apName) + "\",";
  json +=
      "\"hideAP\":" + String(storage.config.hideAP ? "true" : "false") + ",";
  json += "\"wifiChannel\":" + String(storage.config.wifiChannel) + ",";
  json += "\"randomizeMAC\":" +
          String(storage.config.randomizeMAC ? "true" : "false") + ",";

  // Include SSIDs
  json += "\"ssids\":\"";
  for (size_t i = 0; i < storage.ssids.size(); i++) {
    String s = storage.ssids[i];
    s.replace("\n", "\\n");
    s.replace("\"", "\\\"");
    json += s;
    if (i < storage.ssids.size() - 1)
      json += "\\n";
  }
  json += "\"}";

  server.sendHeader("Content-Disposition",
                    "attachment; filename=\"beacon_config.json\"");
  server.send(200, "application/json", json);
}

void CaptivePortal::handleImportConfig() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "Missing data");
    return;
  }

  // This uses the same parsing logic as saveConfig
  String data = server.arg("data");

  // Reuse the extractStr lambda pattern
  auto extractStr = [&](const char *key, char *dest, size_t len) {
    String k = "\"" + String(key) + "\":\"";
    int start = data.indexOf(k);
    if (start > 0) {
      start += k.length();
      int end = data.indexOf("\"", start);
      if (end > start) {
        String val = data.substring(start, end);
        strlcpy(dest, val.c_str(), len);
      }
    }
  };

  // Parse all fields
  storage.config.wpa2 = data.indexOf("\"wpa2\":true") > 0;
  storage.config.appendSpaces = data.indexOf("\"appendSpaces\":true") > 0;
  storage.config.enableBLE = data.indexOf("\"enableBLE\":true") > 0;

  int intervalIdx = data.indexOf("\"beaconInterval\":");
  if (intervalIdx > 0) {
    int endIdx = data.indexOf(",", intervalIdx);
    if (endIdx < 0)
      endIdx = data.indexOf("}", intervalIdx);
    String val = data.substring(intervalIdx + 17, endIdx);
    storage.config.beaconInterval = val.toInt();
  }

  extractStr("adminUser", storage.config.adminUser,
             sizeof(storage.config.adminUser));
  extractStr("adminPass", storage.config.adminPass,
             sizeof(storage.config.adminPass));
  extractStr("advertisingHeadline", storage.config.advertisingHeadline,
             sizeof(storage.config.advertisingHeadline));
  extractStr("advertisingDescription", storage.config.advertisingDescription,
             sizeof(storage.config.advertisingDescription));
  extractStr("buttonText", storage.config.buttonText,
             sizeof(storage.config.buttonText));
  extractStr("redirectUrl", storage.config.redirectUrl,
             sizeof(storage.config.redirectUrl));
  extractStr("apName", storage.config.apName, sizeof(storage.config.apName));

  int delayIdx = data.indexOf("\"autoRedirectDelay\":");
  if (delayIdx > 0) {
    int endIdx = data.indexOf(",", delayIdx);
    if (endIdx < 0)
      endIdx = data.indexOf("}", delayIdx);
    storage.config.autoRedirectDelay =
        data.substring(delayIdx + 20, endIdx).toInt();
  }

  storage.config.hideAP = data.indexOf("\"hideAP\":true") > 0;

  int channelIdx = data.indexOf("\"wifiChannel\":");
  if (channelIdx > 0) {
    int endIdx = data.indexOf(",", channelIdx);
    if (endIdx < 0)
      endIdx = data.indexOf("}", channelIdx);
    int ch = data.substring(channelIdx + 14, endIdx).toInt();
    if (ch >= 1 && ch <= 14)
      storage.config.wifiChannel = ch;
  }

  storage.config.randomizeMAC = data.indexOf("\"randomizeMAC\":true") > 0;

  // Parse and save SSIDs
  int ssidIdx = data.indexOf("\"ssids\":\"");
  if (ssidIdx > 0) {
    int start = ssidIdx + 9;
    int end = data.indexOf("\"}", start);
    if (end > start) {
      String ssidData = data.substring(start, end);
      ssidData.replace("\\n", "\n");
      storage.saveSSIDs(ssidData);
    }
  }

  storage.saveConfig();
  server.send(200, "text/plain", "Config Imported! Reboot to apply changes.");
}
