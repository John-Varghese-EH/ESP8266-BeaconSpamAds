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

  // Admin
  static void handleAdmin();
  static void handleSaveSSIDs();
  static void handleSaveConfig();
  static void handleGetData();
  static void handleReboot();

  DNSServer dnsServer;
};

#endif
