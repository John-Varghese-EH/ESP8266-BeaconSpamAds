#include "CaptivePortal.h"
#include "Storage.h"
#include "web_index.h"

#ifdef ESP32
#include <esp_wifi.h>
WebServer server(80);
#else
extern "C" {
#include <user_interface.h>
void wifi_softap_deauth(uint8 mac[6]); // Explicit prototype
}
ESP8266WebServer server(80);
#endif

// Init static members
IPAddress CaptivePortal::pendingDeauthIP;
unsigned long CaptivePortal::deauthTime = 0;
bool CaptivePortal::deauthActive = false;

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

// Deep Link: Find custom URL for SSID (format: "SSID|https://url.com")
String CaptivePortal::getDeepLinkUrl(const String &ssidName) {
  for (size_t i = 0; i < storage.ssids.size(); i++) {
    String entry = storage.ssids[i];
    int pipeIdx = entry.indexOf('|');
    if (pipeIdx > 0) {
      String entrySSID = entry.substring(0, pipeIdx);
      entrySSID.trim();
      if (entrySSID.equals(ssidName)) {
        String url = entry.substring(pipeIdx + 1);
        url.trim();
        return url;
      }
    }
  }
  return ""; // No deep link found
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

  // Portal HTML Editor API
  server.on("/api/portal_html", handleGetPortalHTML);
  server.on("/api/save_portal_html", HTTP_POST, handleSavePortalHTML);
  server.on("/api/reset_portal_html", HTTP_POST, handleResetPortalHTML);
  server.on("/content", handleCustomContent);

  // CNA Breakout Routes
  server.on("/hotspot-detect.html", handleAppleCaptivePortal);
  server.on("/library/test/success.html", handleAppleCaptivePortal);
  server.on("/success.txt",
            []() { server.send(200, "text/plain", "success"); });
  server.on("/disconnect", handleDisconnect);

  server.onNotFound(handleNotFound);
  server.begin();
}

void CaptivePortal::update() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (deauthActive && millis() > deauthTime) {
    deauthClient(pendingDeauthIP);
    deauthActive = false;
  }
}

void CaptivePortal::handleAppleCaptivePortal() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(
      200, "text/html",
      "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
}

void CaptivePortal::handleDisconnect() {
  pendingDeauthIP = server.client().remoteIP();
  deauthTime = millis() + 1500; // Wait 1.5s for redirect to start
  deauthActive = true;
  server.send(200, "text/plain", "Disconnecting...");
}

void CaptivePortal::deauthClient(IPAddress ip) {
  // Find MAC from IP and deauth
  // This requires iterating the station list
  uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
  bool found = false;

#ifdef ESP32
  wifi_sta_list_t stationList;
  tcpip_adapter_sta_list_t adapterList;
  esp_wifi_ap_get_sta_list(&stationList);
  tcpip_adapter_get_sta_list(&stationList, &adapterList);
  for (int i = 0; i < adapterList.num; i++) {
    tcpip_adapter_sta_info_t station = adapterList.sta[i];
    if (station.ip.addr == ip) {
      memcpy(mac, station.mac, 6);
      found = true;
      break;
    }
  }
  if (found)
    esp_wifi_deauth_sta(mac);

#else
  struct station_info *stat_info = wifi_softap_get_station_info();
  struct station_info *head = stat_info; // Keep head to free later
  while (stat_info != NULL) {
    if (stat_info->ip.addr == (uint32_t)ip) {
      memcpy(mac, stat_info->bssid, 6);
      found = true;
      // Don't break yet, need to finish for cleanup? No, we can just use head.
      break;
    }
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  wifi_softap_free_station_info(); // Always free the list head!

  if (found)
    wifi_softap_deauth(mac);
#endif
}

void CaptivePortal::handleRoot() {
  if (server.hostHeader() != "192.168.4.1") {
    server.sendHeader("Location", String("http://192.168.4.1/"), true);
    server.send(302, "text/html", "");
    return;
  }

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  auto send = [&](String s) { server.sendContent(s); };

  // 1. Header & Styles (Refactored for Full Screen Flex)
  send(F(
      "<!DOCTYPE html><html lang='en'><head>"
      "<meta charset='UTF-8'><meta name='viewport' "
      "content='width=device-width,initial-scale=1'>"
      "<title>ESP Beacon Spam</title><style>"
      ":root{--bg:#000;--card:#161b22;--border:#30363d;--text:#c9d1d9;"
      "--dim:#8b949e;--accent:#58a6ff;--btn:linear-gradient(135deg,#238636,#"
      "2ea043)}"
      "*{box-sizing:border-box;margin:0;padding:0}"
      "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe "
      "UI',Roboto,sans-serif;"
      "background:var(--bg);color:var(--text);height:100vh;width:100vw;"
      "overflow:hidden;"
      "display:flex;flex-direction:column}"
      "iframe{border:none;width:100%;flex:1;background:#fff}" // Iframe takes
                                                              // remaining space
      ".foot-bar{background:var(--card);border-top:1px solid "
      "var(--border);padding:15px;"
      "display:flex;flex-direction:column;align-items:center;justify-content:"
      "center;z-index:99}"
      ".btn{display:block;width:100%;max-width:400px;padding:14px 20px;"
      "background:var(--btn);color:#fff;border-radius:10px;font-weight:600;"
      "font-size:1rem;border:none;cursor:pointer;text-decoration:none;text-"
      "align:center;"
      "transition:transform .2s,box-shadow .2s}"
      ".btn:hover{transform:translateY(-2px);box-shadow:0 6px 20px "
      "rgba(35,134,54,.4)}"
      ".attr{margin-top:8px;font-size:10px;color:var(--dim)}"
      ".attr a{color:var(--dim);text-decoration:none}"
      "</style></head><body>"));

  // 2. Iframe Content
  send(F("<iframe src='/content' title='Portal Content'></iframe>"));

  // 3. Footer / Button (Conditional) - disableButton check
  bool showFooter = (!storage.config.disableButton);

  if (showFooter) {
    String currentSSID = String(storage.config.apName);
    String deepLink = getDeepLinkUrl(currentSSID);
    String rUrl =
        (deepLink.length() > 0) ? deepLink : String(storage.config.redirectUrl);
    if (rUrl.length() == 0)
      rUrl = "https://google.com";

    String safeUrl = rUrl;
    safeUrl.replace("\"", "&quot;");
    safeUrl.replace("'", "\\'");

    send(F("<div class='foot-bar' id='fb'>"));
    send("<a href='#' class='btn' id='btn' onclick=\"connect('" + safeUrl +
         "')\">" + String(storage.config.buttonText) + "</a>");
    send(F("<div class='attr'>Made with ❤️ by <a "
           "href='https://github.com/John-Varghese-EH'>John-Varghese-EH</a></"
           "div>"));
    send(F("</div>"));
  }

  // Script
  send(F("<script>"
         "function copyToClipboard(text){"
         "if(navigator.clipboard){"
         "navigator.clipboard.writeText(text).catch(function(err){"
         "fallbackCopyTextToClipboard(text);});"
         "}else{fallbackCopyTextToClipboard(text);}"
         "}"
         "function fallbackCopyTextToClipboard(text){"
         "var textArea=document.createElement('textarea');"
         "textArea.value=text;"
         "textArea.style.top='0';textArea.style.left='0';textArea.style."
         "position='fixed';"
         "document.body.appendChild(textArea);"
         "textArea.focus();textArea.select();"
         "try{document.execCommand('copy');}catch(err){}"
         "document.body.removeChild(textArea);"
         "}"
         "function connect(url){"
         // Copy to clipboard with fallback
         "copyToClipboard(url);"
         // Trigger Disconnect (Switch to Data)
         "fetch('/disconnect').catch(e=>{});"
         // Change button text temporarily
         "document.getElementById('btn').innerText='Opening...';"
         // Redirect with small delay
         "setTimeout(function(){window.location.href=url;},500);"
         "}"
         "</script></body></html>"));

  server.sendContent("");
}

void CaptivePortal::handleCustomContent() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");

  if (storage.config.useCustomPortal && storage.hasCustomPortalHTML()) {
    File f = LittleFS.open("/portal_custom.html", "r");
    if (f) {
      server.streamFile(f, "text/html");
      f.close();
    } else {
      server.send(500, "text/plain", "File Open Error");
    }
  } else {
    // Default Content (Served inside Iframe)
    String html =
        F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
          "<meta name='viewport' content='width=device-width,initial-scale=1'>"
          "<style>"
          ":root{--bg:#0d1117;--card:#161b22;--text:#c9d1d9;--dim:#8b949e;--"
          "accent:#58a6ff}"
          "body{font-family:-apple-system,BlinkMacSystemFont,Roboto,sans-serif;"
          "background:var(--bg);color:var(--text);display:flex;justify-content:"
          "center;align-items:center;min-height:100vh;margin:0;padding:20px}"
          ".c{background:var(--card);border:1px solid "
          "#30363d;border-radius:16px;padding:30px;max-width:380px;width:100%;"
          "text-align:center}"
          ".logo{font-size:40px;margin-bottom:10px}"
          "h1{font-size:1.4rem;margin-bottom:8px}"
          ".desc{color:var(--dim);font-size:.9rem;line-height:1.5;margin-"
          "bottom:18px}"
          ".f{text-align:left;background:rgba(88,166,255,.08);border-radius:"
          "8px;padding:10px;border:1px solid rgba(88,166,255,.15)}"
          ".f div{padding:4px 0;font-size:.85rem;color:var(--dim)}"
          "</style></head><body>"
          "<div class='c'>"
          "<div class='logo'>⚡</div><h1>");

    html += String(storage.config.advertisingHeadline);
    html += F("</h1><p class='desc'>");
    html += String(storage.config.advertisingDescription);
    html += F("</p><div class='f'>"
              "<div><span>📡</span> 50+ fake WiFi networks</div>"
              "<div><span>🌐</span> Customizable captive portal</div>"
              "<div><span>⚙️</span> Web-based admin panel</div>"
              "</div></div></body></html>");

    server.send(200, "text/html", html);
  }
}

void CaptivePortal::handleNotFound() {
  // Redirect everything unknown to trigger portal
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.send(302, "text/plain", "");
}

// ===== Platform-Specific Captive Portal Handlers =====
// Return 302 redirect to trigger sign-in popup on all platforms

// Apple iOS/macOS - expects specific success page
// Returning redirect triggers the sign-in popup
void CaptivePortal::handleCaptiveApple() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(302, "text/html", "");
}

// Android - expects HTTP 204 No Content
// Returning redirect triggers the sign-in popup
void CaptivePortal::handleCaptiveAndroid() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(302, "text/plain", "");
}

// Windows 10/11 - expects "Microsoft Connect Test" or "Microsoft NCSI"
// Returning redirect triggers the sign-in popup
void CaptivePortal::handleCaptiveWindows() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(302, "text/plain", "");
}

// Firefox - expects "success\n"
// Returning redirect triggers captive portal notification
void CaptivePortal::handleCaptiveFirefox() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.send(302, "text/plain", "");
}

// Legacy handler kept for compatibility
void CaptivePortal::handleCaptivePortal() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(302, "text/plain", "");
}

// Admin Handlers

void CaptivePortal::handleAdmin() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();
  // Prevent browser caching to fix blank page after login
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(200, "text/html", admin_html);
}

void CaptivePortal::handleGetData() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");

  auto send = [&](String s) { server.sendContent(s); };

  send("{");
  send("\"uptime\":");
  send(String(millis() / 1000));
  send(",");
  send("\"freeHeap\":");
  send(String(ESP.getFreeHeap()));
  send(",");
#ifdef ESP32
  send("\"clientCount\":");
  send(String(WiFi.softAPgetStationNum()));
  send(",");
#else
  send("\"clientCount\":");
  send(String(WiFi.softAPgetStationNum()));
  send(",");
#endif
  send("\"wpa2\":");
  send(storage.config.wpa2 ? "true" : "false");
  send(",");
  send("\"appendSpaces\":");
  send(storage.config.appendSpaces ? "true" : "false");
  send(",");
  send("\"enableBLE\":");
  send(storage.config.enableBLE ? "true" : "false");
  send(",");
  send("\"beaconInterval\":");
  send(String(storage.config.beaconInterval));
  send(",");

  // New Fields
  send("\"adminUser\":\"");
  send(storage.config.adminUser);
  send("\",");
  send("\"adminPass\":\"");
  send(storage.config.adminPass);
  send("\",");
  send("\"redirectUrl\":\"");
  send(storage.config.redirectUrl);
  send("\",");
  send("\"advertisingHeadline\":\"");
  send(storage.config.advertisingHeadline);
  send("\",");
  send("\"advertisingDescription\":\"");
  send(storage.config.advertisingDescription);
  send("\",");
  send("\"buttonText\":\"");
  send(storage.config.buttonText);
  send("\",");
  send("\"autoRedirectDelay\":");
  send(String(storage.config.autoRedirectDelay));
  send(",");
  send("\"apName\":\"");
  send(storage.config.apName);
  send("\",");
  send("\"hideAP\":");
  send(storage.config.hideAP ? "true" : "false");
  send(",");

  // Advanced settings
  send("\"wifiChannel\":");
  send(String(storage.config.wifiChannel));
  send(",");
  send("\"randomizeMAC\":");
  send(storage.config.randomizeMAC ? "true" : "false");
  send(",");
  send("\"useCustomPortal\":");
  send(storage.config.useCustomPortal ? "true" : "false");
  send(",");

  send("\"ssidCount\":");
  send(String(storage.ssids.size()));
  send(",");

  // Stream SSIDs to save RAM
  send("\"ssids\":\"");
  for (size_t i = 0; i < storage.ssids.size(); i++) {
    String s = storage.ssids[i];
    s.replace("\n", "\\n");
    s.replace("\"", "\\\"");
    s.replace("\\", "\\\\"); // Escape backslashes too

    if (i > 0)
      send("\\n");
    send(s);
  }
  send("\"");

  send("}");
  server.sendContent("");
}

// Stream public data to save RAM
// Stream public data to save RAM
void CaptivePortal::handlePublicData() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");

  auto send = [&](String s) { server.sendContent(s); };

  send("{");
  send("\"advertisingHeadline\":\"");
  send(storage.config.advertisingHeadline);
  send("\",");
  send("\"advertisingDescription\":\"");
  send(storage.config.advertisingDescription);
  send("\",");
  send("\"buttonText\":\"");
  send(storage.config.buttonText);
  send("\",");
  send("\"autoRedirectDelay\":");
  send(String(storage.config.autoRedirectDelay));
  send(",");

  // Check for Deep Link (Custom URL for current AP Name)
  String currentSSID = String(storage.config.apName);
  String deepLink = getDeepLinkUrl(currentSSID);

  String rUrl;
  if (deepLink.length() > 0) {
    rUrl = deepLink;
  } else {
    rUrl = String(storage.config.redirectUrl);
  }

  if (rUrl.length() == 0)
    rUrl = "https://google.com";

  send("\"redirectUrl\":\"");
  send(rUrl);
  send("\"");
  send("}");
  server.sendContent("");
}

void CaptivePortal::handleSaveConfig() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  // Safety check
  if (ESP.getFreeHeap() < 2000) {
    server.send(503, "text/plain", "Low memory");
    return;
  }

  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "Missing data");
    return;
  }

  const String &data = server.arg("data");

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

  // Parse boolean: useCustomPortal
  storage.config.useCustomPortal = data.indexOf("\"useCustomPortal\":true") > 0;

  storage.saveConfig();
  server.send(200, "text/plain", "Config Saved! Reboot to apply changes.");
}

void CaptivePortal::handleSaveSSIDs() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  // Safety check: ensure we have enough RAM to process this
  if (ESP.getFreeHeap() < 5000) {
    server.send(503, "text/plain", "Device low on memory, please reboot");
    return;
  }

  if (!server.hasArg("ssids")) {
    server.send(400, "text/plain", "Missing ssids");
    return;
  }

  // Use reference to avoid copying valid data
  const String &ssids = server.arg("ssids");

  if (ssids.length() > 20000) {
    server.send(413, "text/plain", "SSID list too large");
    return;
  }

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

  server.sendHeader("Content-Disposition",
                    "attachment; filename=\"beacon_spam_config.json\"");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");

  // Helper to stream chunks
  auto send = [&](String s) { server.sendContent(s); };

  send("{");
  send("\"wpa2\":" + String(storage.config.wpa2 ? "true" : "false") + ",");
  send("\"appendSpaces\":" +
       String(storage.config.appendSpaces ? "true" : "false") + ",");
  send("\"enableBLE\":" + String(storage.config.enableBLE ? "true" : "false") +
       ",");
  send("\"beaconInterval\":" + String(storage.config.beaconInterval) + ",");

  send("\"adminUser\":\"" + String(storage.config.adminUser) + "\",");
  send("\"adminPass\":\"" + String(storage.config.adminPass) + "\",");

  send("\"advertisingHeadline\":\"" +
       String(storage.config.advertisingHeadline) + "\",");
  send("\"advertisingDescription\":\"" +
       String(storage.config.advertisingDescription) + "\",");
  send("\"buttonText\":\"" + String(storage.config.buttonText) + "\",");
  send("\"redirectUrl\":\"" + String(storage.config.redirectUrl) + "\",");
  send("\"autoRedirectDelay\":" + String(storage.config.autoRedirectDelay) +
       ",");
  send("\"apName\":\"" + String(storage.config.apName) + "\",");
  send("\"hideAP\":" + String(storage.config.hideAP ? "true" : "false") + ",");
  send("\"wifiChannel\":" + String(storage.config.wifiChannel) + ",");
  send("\"randomizeMAC\":" +
       String(storage.config.randomizeMAC ? "true" : "false") + ",");
  send("\"useCustomPortal\":" +
       String(storage.config.useCustomPortal ? "true" : "false") + ",");

  // Stream SSIDs
  send("\"ssids\":\"");
  for (size_t i = 0; i < storage.ssids.size(); i++) {
    String s = storage.ssids[i];
    // Escape quotes and backslashes in SSIDs
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    s.replace("\n", "\\n");

    if (i > 0)
      send("\\n");
    send(s);
  }
  send("\""); // End ssids string

  send("}");              // End JSON
  server.sendContent(""); // End of stream
}

void CaptivePortal::handleImportConfig() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  // Safety check
  if (ESP.getFreeHeap() < 5000) {
    server.send(503, "text/plain", "Low memory");
    return;
  }

  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "Missing data");
    return;
  }

  // This uses the same parsing logic as saveConfig
  const String &data = server.arg("data");

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
  storage.config.useCustomPortal = data.indexOf("\"useCustomPortal\":true") > 0;

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

// ===== Portal HTML Editor Handlers =====

void CaptivePortal::handleGetPortalHTML() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  // Return custom HTML if exists, otherwise return default
  String html;
  if (storage.hasCustomPortalHTML()) {
    html = storage.loadCustomPortalHTML();
  } else {
    html = portal_html;
  }
  server.send(200, "text/html", html);
}

void CaptivePortal::handleSavePortalHTML() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  if (ESP.getFreeHeap() < 5000) {
    server.send(503, "text/plain", "Low memory");
    return;
  }

  if (!server.hasArg("html")) {
    server.send(400, "text/plain", "Missing html parameter");
    return;
  }

  const String &html = server.arg("html");

  // Limit size to prevent memory issues (8KB max)
  if (html.length() > 8192) {
    server.send(400, "text/plain", "HTML too large (max 8KB)");
    return;
  }

  storage.saveCustomPortalHTML(html);
  server.send(200, "text/plain", "Portal HTML saved!");
}

void CaptivePortal::handleResetPortalHTML() {
  if (!authenticateAdmin()) {
    return server.requestAuthentication();
  }
  addSecurityHeaders();

  storage.deleteCustomPortalHTML();
  server.send(200, "text/plain", "Portal HTML reset to default!");
}
