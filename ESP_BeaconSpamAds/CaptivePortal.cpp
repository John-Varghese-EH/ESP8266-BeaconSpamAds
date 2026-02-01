#include "CaptivePortal.h"
#include "Storage.h"
#include "web_index.h"

#ifdef ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif

void CaptivePortal::setup() {
  // start WiFi
  WiFi.mode(WIFI_AP_STA); // AP for Portal, STA for Beacon Spam
  WiFi.softAP(
      "Connect me!"); // We could make this configurable too if we wanted

  server.on("/generate_204", handleRoot);
  server.on("/hotspot-detect.html", handleRoot);
  server.on("/connecttest.txt", handleRoot);
  server.on("/", handleRoot);

  // Admin Routes
  server.on("/admin", handleAdmin);
  server.on("/api/data", handleGetData);
  server.on("/api/save_config", HTTP_POST, handleSaveConfig);
  server.on("/api/save_ssids", HTTP_POST, handleSaveSSIDs);
  server.on("/api/reboot", HTTP_POST, handleReboot);

  server.onNotFound(handleNotFound);
  server.begin();
}

void CaptivePortal::update() { server.handleClient(); }

void CaptivePortal::handleRoot() { server.send(200, "text/html", portal_html); }

void CaptivePortal::handleNotFound() {
  // If trying to access admin IPs directly, let it through? No, safer to just
  // redirect everything unless it matches specific routes
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(),
                    true);
  server.send(302, "text/plain", "");
}

// Admin Handlers

void CaptivePortal::handleAdmin() { server.send(200, "text/html", admin_html); }

void CaptivePortal::handleGetData() {
  String json = "{";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"clientCount\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"wpa2\":" + String(storage.config.wpa2 ? "true" : "false") + ",";
  json += "\"appendSpaces\":" +
          String(storage.config.appendSpaces ? "true" : "false") + ",";
  json +=
      "\"enableBLE\":" + String(storage.config.enableBLE ? "true" : "false") +
      ",";
  json += "\"beaconInterval\":" + String(storage.config.beaconInterval) + ",";
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

void CaptivePortal::handleSaveConfig() {
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

  storage.saveConfig();
  server.send(200, "text/plain", "Config Saved");
}

void CaptivePortal::handleSaveSSIDs() {
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
  server.send(200, "text/plain", "Rebooting...");
  delay(500);
  ESP.restart();
}
