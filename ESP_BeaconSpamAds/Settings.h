#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
// ============================================= //
//           BEACON SPAM CONFIGURATION           //
// ============================================= //
// Modify these settings before flashing to      //
// customize your default device's behavior.     //
// ============================================= //

// ------- Beacon Behavior ------- //
// Enable WPA2 encryption on fake networks (makes them look more realistic)
const bool wpa2 = true;

// ------- BLE Spam (ESP32 Only) ------- //
// Enable Bluetooth Low Energy beacon spam (only works on ESP32)
#define DEFAULT_ENABLE_BLE false

// ------- Admin Panel Credentials ------- //
// Default login for the /admin web interface
// IMPORTANT: Change these before deploying!
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PASS "beacon"

// ------- Captive Portal Settings ------- //
// Where users are redirected when they click the button
#define DEFAULT_REDIRECT_URL                                                   \
  "https://github.com/John-Varghese-EH/ESP8266-BeaconSpamAds"

// The headline shown on the captive portal page
#define DEFAULT_HEADLINE "⚡ ESP Beacon Spam"

// Description text shown below the headline
#define DEFAULT_DESCRIPTION                                                    \
  "Open-source WiFi beacon spammer & captive portal. Check out the project "   \
  "on GitHub!"

// Button text on the portal page
#define DEFAULT_BUTTON_TEXT "View on GitHub"

// Auto-redirect delay in seconds (0 = disabled, user must click)
#define DEFAULT_AUTO_REDIRECT_DELAY 0

// ------- Access Point Settings ------- //
// Name of the captive portal AP (what users connect to)
#define DEFAULT_AP_NAME "Connect Me!!!"

// Hide the AP from network lists (experimental, may not work on all devices)
#define DEFAULT_HIDE_AP false

// ------- Wi-Fi Channel Settings ------- //
// Channels to broadcast beacons on (1-14 available)
const uint8_t channels[] = {1, 6, 11};

// Pad SSIDs to 32 characters with spaces (improves transmission performance)
const bool appendSpaces = true;

// ------- Beacon Timing ------- //
// Interval between beacon transmissions in milliseconds (lower = more spam)
#define DEFAULT_BEACON_INTERVAL 100

/*
  SSIDs:
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
const char ssids[] PROGMEM = {"Mom Use This One\n"
                              "Abraham Linksys\n"
                              "Benjamin FrankLAN\n"
                              "Martin Router King\n"
                              "John Wilkes Bluetooth\n"
                              "Pretty Fly for a Wi-Fi\n"
                              "Bill Wi the Science Fi\n"
                              "I Believe Wi Can Fi\n"
                              "Tell My Wi-Fi Love Her\n"
                              "No More Mister Wi-Fi\n"
                              "LAN Solo\n"
                              "The LAN Before Time\n"
                              "Silence of the LANs\n"
                              "House LANister\n"
                              "Winternet Is Coming\n"
                              "Ping’s Landing\n"
                              "The Ping in the North\n"
                              "This LAN Is My LAN\n"
                              "Get Off My LAN\n"
                              "The Promised LAN\n"
                              "The LAN Down Under\n"
                              "FBI Surveillance Van 4\n"
                              "Area 51 Test Site\n"
                              "Drive-By Wi-Fi\n"
                              "Planet Express\n"
                              "Wu Tang LAN\n"
                              "Darude LANstorm\n"
                              "Never Gonna Give You Up\n"
                              "Hide Yo Kids, Hide Yo Wi-Fi\n"
                              "Loading-J0X\n"
                              "Searching…\n"
                              "VIRUS.EXE\n"
                              "Virus-Infected Wi-Fi\n"
                              "Starbucks Wi-Fi\n"
                              "Text ###-#### for Password\n"
                              "Yell ____ for Password\n"
                              "The Password Is 1234\n"
                              "Free Public Wi-Fi\n"
                              "No Free Wi-Fi Here\n"
                              "Get Your Own Damn Wi-Fi\n"
                              "It Hurts When IP\n"
                              "Dora the Internet Explorer\n"
                              "404 Wi-Fi Unavailable\n"
                              "Porque-Fi\n"
                              "Titanic Syncing\n"
                              "Test Wi-Fi Please Ignore\n"
                              "Drop It Like It’s Hotspot\n"
                              "Life in the Fast LAN\n"
                              "The Creep Next Door\n"
                              "Ye Olde Internet\n"};

#endif
