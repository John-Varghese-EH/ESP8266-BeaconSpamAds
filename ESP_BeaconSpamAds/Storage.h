#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <vector>
#ifdef ESP32
#include <FS.h>
#endif
#include <LittleFS.h>

struct Config {
  // Config version - increment when struct changes to invalidate old configs
  uint32_t configVersion; // Must be CONFIG_MAGIC to be valid

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
  bool disableButton;    // Hide button (Full screen iframe)
  char redirectUrl[128]; // Where to redirect on button click
  int autoRedirectDelay; // Seconds to wait before auto-redirecting (0=disabled)

  // Access Point Settings
  char apName[32]; // Name of the captive portal AP
  bool hideAP;     // Hide the AP from SSID lists

  // Advanced Beacon Settings
  uint8_t wifiChannel; // Channel for beacons (1-14)
  bool randomizeMAC;   // Randomize source MAC addresses

  // Custom Portal
  bool useCustomPortal; // Use custom HTML instead of default
};

// Magic number to validate config - change when struct changes
#define CONFIG_MAGIC 0xBEACF005

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

  // Custom Portal HTML
  String loadCustomPortalHTML();
  void saveCustomPortalHTML(const String &html);
  void deleteCustomPortalHTML();
  bool hasCustomPortalHTML();

private:
  void createDefaultSSIDs();
  void createDefaultConfig();
};

extern Storage storage;

#endif
