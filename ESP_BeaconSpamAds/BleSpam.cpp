#include "BleSpam.h"

#ifdef ESP32

#include "esp_sleep.h"

void BleSpam::setup() {
  Serial.println("Initializing BLE Spam...");
  BLEDevice::init("");
  BLEServer *pServer = BLEDevice::createServer();
  pAdvertising = pServer->getAdvertising();

  // Initial Beacon Set
  setBeacon();
  pAdvertising->start();
}

void BleSpam::update() {
  // Periodically change the beacon data to look like new devices
  if (millis() - lastRun > 1000) {
    lastRun = millis();

    pAdvertising->stop();
    setBeacon();
    pAdvertising->start();
  }
}

void BleSpam::setBeacon() {
  BLEBeacon oBeacon = BLEBeacon();

  // Random Vendor ID
  uint16_t manufId = random(0, 0xFFFF);
  oBeacon.setManufacturerId(manufId);

  // Random UUID
  char uuid[37];
  sprintf(uuid, "%08x-%04x-%04x-%04x-%04x%08x", random(0, 0xFFFFFFFF),
          random(0, 0xFFFF), random(0, 0xFFFF), random(0, 0xFFFF),
          random(0, 0xFFFF), random(0, 0xFFFFFFFF));

  BLEUUID bleUUID = BLEUUID(std::string(uuid));
  oBeacon.setProximityUUID(bleUUID);
  oBeacon.setMajor(random(0, 0xFFFF));
  oBeacon.setMinor(random(0, 0xFFFF));

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED

  std::string strServiceData = "";
  strServiceData += (char)26;   // Len
  strServiceData += (char)0xFF; // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

void BleSpam::setEddystone() {
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(
      0x06); // General Discoverable + BR_EDR_NOT_SUPPORTED

  std::string strServiceData = "";
  strServiceData += (char)0xAA;
  strServiceData += (char)0xFE; // Service UUID 0xFEAA
  strServiceData += (char)0x10; // Frame Type: URL
  strServiceData += (char)0xEE; // TX Power

  // Compress URL Scheme
  String url = targetUrl;
  char scheme = 0x03; // Default https://
  if (url.startsWith("http://www.")) {
    scheme = 0x00;
    url.remove(0, 11);
  } else if (url.startsWith("https://www.")) {
    scheme = 0x01;
    url.remove(0, 12);
  } else if (url.startsWith("http://")) {
    scheme = 0x02;
    url.remove(0, 7);
  } else if (url.startsWith("https://")) {
    scheme = 0x03;
    url.remove(0, 8);
  }

  strServiceData += scheme;

  // Append URL (Max 17 bytes usually)
  // We can add suffix compression here if needed but simpler is just raw
  // Common suffixes: .com/ (0x00), .org/ (0x01), .edu/ (0x02), .net/ (0x03),
  // .info/ (0x04) .biz/ (0x05), .gov/ (0x06), .com (0x07), .org (0x08), .edu
  // (0x09), .net (0x0A), .info (0x0B), .biz (0x0C), .gov (0x0D)

  // Simple check for .com/ (most common)
  int comSlash = url.indexOf(".com/");
  if (comSlash > 0) {
    strServiceData += url.substring(0, comSlash).c_str();
    strServiceData += (char)0x00;
    strServiceData += url.substring(comSlash + 5).c_str();
  } else {
    int com = url.indexOf(".com");
    if (com > 0 && com == url.length() - 4) {
      strServiceData += url.substring(0, com).c_str();
      strServiceData += (char)0x07;
    } else {
      strServiceData += url.c_str();
    }
  }

  oAdvertisementData.addData(strServiceData);

  // Add 0xFEAA Service UUID
  oAdvertisementData.setCompleteServices(BLEUUID("FEAA"));

  pAdvertising->setAdvertisementData(oAdvertisementData);
}

void BleSpam::setUrl(String url) { targetUrl = url; }

#endif
