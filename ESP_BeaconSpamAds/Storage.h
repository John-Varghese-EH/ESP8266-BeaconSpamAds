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
                      // We can add channel selection bitmask later if needed
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
