#include "BeaconSpam.h"
#include "Settings.h"
#include "Storage.h"

#ifdef ESP32
#include "esp_wifi.h"
// Wrapper for sending packets
#define SCAN_INTERVAL 100 // ms
#else
extern "C" {
#include "user_interface.h"
typedef void (*freedom_outside_cb_t)(uint8 status);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
void wifi_unregister_send_pkt_freedom_cb(void);
int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}
#endif

// Beacon packet definition
uint8_t beaconPacket[109] = {
    /*  0 - 3  */ 0x80, 0x00, 0x00,
    0x00, // Type/Subtype: managment beacon frame
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
    /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
    /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source

    // Fixed parameters
    /* 22 - 23 */ 0x00,
    0x00, // Fragment & sequence number (will be done by the SDK)
    /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
    /* 32 - 33 */ 0xe8,
    0x03, // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
    /* 34 - 35 */ 0x31, 0x00, // capabilities Tnformation

    // Tagged parameters

    // SSID parameters
    /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
    /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID

    // Supported Rates
    /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
    /* 72 */ 0x82,            // 1(B)
    /* 73 */ 0x84,            // 2(B)
    /* 74 */ 0x8b,            // 5.5(B)
    /* 75 */ 0x96,            // 11(B)
    /* 76 */ 0x24,            // 18
    /* 77 */ 0x30,            // 24
    /* 78 */ 0x48,            // 36
    /* 79 */ 0x6c,            // 54

    // Current Channel
    /* 80 - 81 */ 0x03, 0x01, // Channel set, length
    /* 82 */ 0x01,            // Current Channel

    // RSN information
    /*  83 -  84 */ 0x30, 0x18,
    /*  85 -  86 */ 0x01, 0x00,
    /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
    /*  91 -  92 */ 0x02, 0x00,
    /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
    /* 101 - 102 */ 0x01, 0x00,
    /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
    /* 107 - 108 */ 0x00, 0x00};

char emptySSID[32];

void BeaconSpam::setup() {
  // create empty SSID
  for (int i = 0; i < 32; i++)
    emptySSID[i] = ' ';

// random generator
#ifdef ESP32
  // ESP32 doesn't have os_random, use built-in random
#else
  randomSeed(os_random());
#endif

  // set packetSize
  packetSize = sizeof(beaconPacket);
  if (storage.config.wpa2) { // Use dynamic config
    beaconPacket[34] = 0x31;
  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }

  // generate random mac address
  randomMac();

// Set to default WiFi channel
#ifdef ESP32
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
#else
  wifi_set_channel(channels[0]);
#endif

  // Display all loaded SSIDs
  Serial.println(F("Loaded SSIDs from LittleFS:"));
  for (const String &ssid : storage.ssids) {
    Serial.println(ssid);
  }
  Serial.println();
}

void BeaconSpam::update() {
  currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > storage.config.beaconInterval) {
    attackTime = currentTime;

    // Go to next channel
    nextChannel();

    if (storage.ssids.empty())
      return; // Safety check

    // Cycle through SSIDs
    static size_t ssidIndex = 0;
    if (ssidIndex >= storage.ssids.size())
      ssidIndex = 0;

    // We try to send as many as possible within a small window, or just one per
    // interval Original code tried to send all, but that blocks loop and
    // webserver. Let's send a batch (e.g., 3-5 SSIDs) per update to balance
    // spam vs webserver responsiveness

    int batchSize = 5;
    for (int b = 0; b < batchSize; b++) {
      if (ssidIndex >= storage.ssids.size())
        ssidIndex = 0;

      String currentSSID = storage.ssids[ssidIndex];
      ssidIndex++;

      // Prepare packet
      uint8_t parserSsidLen = currentSSID.length();
      if (parserSsidLen > 32)
        parserSsidLen = 32;

      // static int macOffset = 0;
      // macAddr[5] = (uint8_t)(macOffset++); // Change MAC slightly for each
      // SSID so they show up as diff APs Actually, let's just randomize the
      // last byte based on index to keep it stable-ish per SSID
      macAddr[5] = (uint8_t)ssidIndex;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // reset SSID
      memcpy(&beaconPacket[38], emptySSID, 32);

      // write new SSID into beacon frame
      memcpy(&beaconPacket[38], currentSSID.c_str(), parserSsidLen);

      // set channel for beacon frame
      beaconPacket[82] = wifi_channel;

      // send packet
      if (storage.config.appendSpaces) {
        for (int k = 0; k < 3; k++) {
#ifdef ESP32
          packetCounter += esp_wifi_80211_tx(WIFI_IF_AP, beaconPacket,
                                             packetSize, true) == ESP_OK;
#else
          packetCounter +=
              wifi_send_pkt_freedom(beaconPacket, packetSize, 0) == 0;
#endif
          delay(1);
        }
      } else {
        uint16_t tmpPacketSize = (packetSize - 32) + parserSsidLen; // calc size
        // Use static buffer instead of new/delete to prevent heap fragmentation
        memcpy(&packetBuffer[0], &beaconPacket[0],
               38 + parserSsidLen); // copy first half of packet into buffer
        packetBuffer[37] = parserSsidLen; // update SSID length byte
        memcpy(&packetBuffer[38 + parserSsidLen], &beaconPacket[70],
               storage.config.wpa2 ? 39 : 13); // copy second half

        // send packet
        for (int k = 0; k < 3; k++) {
#ifdef ESP32
          packetCounter += esp_wifi_80211_tx(WIFI_IF_AP, packetBuffer,
                                             tmpPacketSize, true) == ESP_OK;
#else
          packetCounter +=
              wifi_send_pkt_freedom(packetBuffer, tmpPacketSize, 0) == 0;
#endif
          delay(1);
        }
        // No delete needed
      }
    }
  }

  // show packet-rate each second
  if (currentTime - packetRateTime > 1000) {
    packetRateTime = currentTime;
    Serial.print(F("Packets/s: "));
    Serial.println(packetCounter);
    packetCounter = 0;
  }
}

void BeaconSpam::nextChannel() {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex >= sizeof(channels))
      channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
#ifdef ESP32
      esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
#else
      wifi_set_channel(wifi_channel);
#endif
    }
  }
}

void BeaconSpam::randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
}
