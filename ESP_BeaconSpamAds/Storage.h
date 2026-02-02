#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <vector>
#ifdef ESP32
#include <FS.h>
#endif
#include <LittleFS.h>

struct Config {
  bool wpa2;
  bool appendSpaces;
  int beaconInterval; // ms, default 100
  bool enableBLE;     // ESP32 only

  // Admin Settings
  char adminUser[32];
  char adminPass[32];

  // Portal Advertising Settings
  char advertisingHeadline[64];     // Main headline
  char advertisingDescription[128]; // Sub-text/description
  char buttonText[32];   // Button label (e.g., "Connect", "Get Offer")
  char redirectUrl[128]; // Where to redirect on button click
  int autoRedirectDelay; // Seconds before auto-redirect (0 = disabled)

  // Access Point Settings
  char apName[32]; // Name of the captive portal AP
  bool hideAP;     // Hide the AP from SSID lists

  // Advanced Beacon Settings
  uint8_t wifiChannel; // Channel for beacons (1-14)
  bool randomizeMAC;   // Randomize source MAC addresses
};

class Storage {
public:
  void setup();

  // Config
  void loadConfig();
  void saveConfig();
  Config config;

  // SSIDs
  void loadSSIDs();
  void saveSSIDs(const String &ssidListContent); // content from textarea
  std::vector<String> ssids;

private:
  void createDefaultSSIDs();
  void createDefaultConfig();
};

extern Storage storage;

#endif
