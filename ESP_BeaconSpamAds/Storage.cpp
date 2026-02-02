#include "Storage.h"
#include "Settings.h"

Storage storage;

const char *CONFIG_FILE = "/config.bin";
const char *SSIDS_FILE = "/ssids.txt";

void Storage::setup() {
  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS Mount Failed. Formatting..."));
    LittleFS.format();
    LittleFS.begin();
  }

  // Check if files exist, else create defaults
  if (!LittleFS.exists(CONFIG_FILE)) {
    createDefaultConfig();
  } else {
    loadConfig();
  }

  if (!LittleFS.exists(SSIDS_FILE)) {
    createDefaultSSIDs();
  }
  loadSSIDs();
}

void Storage::createDefaultConfig() {
  config.wpa2 = false;
  config.appendSpaces = true;
  config.beaconInterval = DEFAULT_BEACON_INTERVAL;
  config.enableBLE = DEFAULT_ENABLE_BLE;

  // Admin Defaults
  strlcpy(config.adminUser, DEFAULT_ADMIN_USER, sizeof(config.adminUser));
  strlcpy(config.adminPass, DEFAULT_ADMIN_PASS, sizeof(config.adminPass));

  // Portal Advertising Defaults
  strlcpy(config.advertisingHeadline, DEFAULT_HEADLINE,
          sizeof(config.advertisingHeadline));
  strlcpy(config.advertisingDescription, DEFAULT_DESCRIPTION,
          sizeof(config.advertisingDescription));
  strlcpy(config.buttonText, DEFAULT_BUTTON_TEXT, sizeof(config.buttonText));
  strlcpy(config.redirectUrl, DEFAULT_REDIRECT_URL, sizeof(config.redirectUrl));
  config.autoRedirectDelay = DEFAULT_AUTO_REDIRECT_DELAY;

  // AP Defaults
  strlcpy(config.apName, DEFAULT_AP_NAME, sizeof(config.apName));
  config.hideAP = DEFAULT_HIDE_AP;

  // Advanced Beacon Defaults
  config.wifiChannel = 1; // Default channel 1
  config.randomizeMAC = false;

  saveConfig();
}

void Storage::loadConfig() {
  File f = LittleFS.open(CONFIG_FILE, "r");
  if (f) {
    f.read((uint8_t *)&config, sizeof(Config));
    f.close();
  }
}

void Storage::saveConfig() {
  File f = LittleFS.open(CONFIG_FILE, "w");
  if (f) {
    f.write((uint8_t *)&config, sizeof(Config));
    f.close();
  }
}

void Storage::createDefaultSSIDs() {
  // Default list
  String defaults =
      "Mom Use This One\nAbraham Linksys\nBenjamin FrankLAN\nMartin Router "
      "King\nJohn Wilkes Bluetooth\nPretty Fly for a Wi-Fi\nBill Wi the "
      "Science Fi\nI Believe Wi Can Fi\nTell My Wi-Fi Love Her\nNo More Mister "
      "Wi-Fi\nLAN Solo\nThe LAN Before Time\nSilence of the LANs\nHouse "
      "LANister\nWinternet Is Coming\nPing's Landing\nThe Ping in the "
      "North\nThis LAN Is My LAN\nGet Off My LAN\nThe Promised LAN\nThe LAN "
      "Down Under\nFBI Surveillance Van 4\nArea 51 Test Site\nDrive-By "
      "Wi-Fi\nPlanet Express\nWu Tang LAN\nDarude LANstorm\nNever Gonna Give "
      "You Up\nHide Yo Kids, Hide Yo "
      "Wi-Fi\nLoading...\nSearching...\nVIRUS.EXE\nVirus-Infected "
      "Wi-Fi\nStarbucks Wi-Fi\nText ###-#### for Password\nYell ____ for "
      "Password\nThe Password Is 1234\nFree Public Wi-Fi\nNo Free Wi-Fi "
      "Here\nGet Your Own Damn Wi-Fi\nIt Hurts When IP\nDora the Internet "
      "Explorer\n404 Wi-Fi Unavailable\nPorque-Fi\nTitanic Syncing\nTest Wi-Fi "
      "Please Ignore\nDrop It Like It's Hotspot\nLife in the Fast LAN\nThe "
      "Creep Next Door\nYe Olde Internet";

  File f = LittleFS.open(SSIDS_FILE, "w");
  if (f) {
    f.print(defaults);
    f.close();
  }
}

void Storage::loadSSIDs() {
  ssids.clear();
  File f = LittleFS.open(SSIDS_FILE, "r");
  if (!f)
    return;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.length() <= 32) {
      ssids.push_back(line);
    }
  }
  f.close();
}

void Storage::saveSSIDs(const String &ssidListContent) {
  File f = LittleFS.open(SSIDS_FILE, "w");
  if (f) {
    f.print(ssidListContent);
    f.close();
  }
  loadSSIDs(); // Reload vector
}
