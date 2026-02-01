/*
  ESP8266 Beacon Spam & Captive Portal Ads (Merged Version)
  -John Varghese (github.com/John-Varghese-EH)
*/

#include "BeaconSpam.h"
#include "BleSpam.h"
#include "CaptivePortal.h"
#include "Settings.h"
#include "Storage.h"

BeaconSpam beaconSpam;
CaptivePortal captivePortal;
#ifdef ESP32
BleSpam bleSpam;
#endif

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\nESP8266 Beacon Spam & Captive Portal Ads (Merged)\n"));

  storage.setup(); // Initialize FS and Config

  beaconSpam.setup();
  captivePortal.setup();

#ifdef ESP32
  if (storage.config.enableBLE) {
    bleSpam.setup();
  }
#endif

  Serial.println(
      F("System Started. Broadcasting beacons and hosting portal..."));
  Serial.println(F("Admin Panel available at http://192.168.4.1/admin"));

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  beaconSpam.update();
  captivePortal.update();
#ifdef ESP32
  if (storage.config.enableBLE) {
    bleSpam.update();
  }
#endif

  // Blink LED to show activity
  static uint32_t lastBlink = 0;
  if (millis() - lastBlink > 500) { // Blink every 500ms
    lastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
