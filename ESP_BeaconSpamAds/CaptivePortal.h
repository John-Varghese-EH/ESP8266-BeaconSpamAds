#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include <DNSServer.h>
#ifdef ESP32
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

#else
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#endif

class CaptivePortal {
public:
  void setup();
  void update();

private:
  static void handleRoot();
  static void handleNotFound();
  static void handleCaptivePortal();
  // Platform-specific captive portal handlers
  static void handleCaptiveApple();
  static void handleCaptiveAndroid();
  static void handleCaptiveWindows();
  static void handleCaptiveFirefox();

  // Admin
  static void handleAdmin();
  static void handleSaveSSIDs();
  static void handleSaveConfig();
  static void handleGetData();
  static void handlePublicData();
  static void handleReboot();
  static void handleRedirect();
  static void handleGetClients();
  static void handleExportConfig();
  static void handleImportConfig();

  // Security functions
  static bool isLockedOut();
  static void recordFailedAttempt();
  static void resetFailedAttempts();
  static bool isRateLimited();
  static void addSecurityHeaders();
  static String sanitizeInput(const String &input, size_t maxLen);
  static bool authenticateAdmin();

  DNSServer dnsServer;
};

#endif
