/*
  ESP8266 Beacon Spam & Captive Portal Ads (Merged Version)
  
  Merged and improved by Google Deemind's Antigravity
  Original Concepts by:
  - V1: Spacehuhn (github.com/spacehuhn)
  - V2: John Varghese (github.com/John-Varghese-EH)
*/

#include "Settings.h"
#include "BeaconSpam.h"
#include "CaptivePortal.h"
#include "Storage.h"

BeaconSpam beaconSpam;
CaptivePortal captivePortal;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\nESP8266 Beacon Spam & Captive Portal Ads (Merged)\n"));

  storage.setup(); // Initialize FS and Config

  beaconSpam.setup();
  captivePortal.setup();

  Serial.println(F("System Started. Broadcasting beacons and hosting portal..."));
  Serial.println(F("Admin Panel available at http://192.168.4.1/admin"));
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  beaconSpam.update();
  captivePortal.update();
  
  // Blink LED to show activity
  static uint32_t lastBlink = 0;
  if(millis() - lastBlink > 500) { // Blink every 500ms
    lastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
