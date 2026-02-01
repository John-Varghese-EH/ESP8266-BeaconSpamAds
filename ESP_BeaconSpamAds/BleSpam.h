#ifndef BLE_SPAM_H
#define BLE_SPAM_H

#ifdef ESP32

#include <Arduino.h>
#include <BLEBeacon.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>


class BleSpam {
public:
  void setup();
  void update();

private:
  void setBeacon();
  BLEAdvertising *pAdvertising;
  uint32_t lastRun = 0;
};

#endif
#endif
