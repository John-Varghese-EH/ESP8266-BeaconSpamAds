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

  // Portal HTML Editor
  static void handleGetPortalHTML();
  static void handleSavePortalHTML();
  static void handleResetPortalHTML();
  static void handleCustomContent();

  // Security functions
  static bool isLockedOut();
  static void recordFailedAttempt();
  static void resetFailedAttempts();
  static bool isRateLimited();
  static void addSecurityHeaders();
  static String sanitizeInput(const String &input, size_t maxLen);
  static String getDeepLinkUrl(const String &ssidName);
  static bool authenticateAdmin();

  // CNA Breakout & Deauth
  static void handleAppleCaptivePortal();
  static void handleDisconnect();
  static void deauthClient(IPAddress ip);

  static IPAddress pendingDeauthIP;
  static unsigned long deauthTime;
  static bool deauthActive;

  DNSServer dnsServer;
};

#endif
