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

#endif
