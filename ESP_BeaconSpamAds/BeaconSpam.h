#ifndef BEACON_SPAM_H
#define BEACON_SPAM_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class BeaconSpam {
public:
    void setup();
    void update();

private:
    void nextChannel();
    void randomMac();
    
    uint8_t channelIndex = 0;
    uint8_t macAddr[6];
    uint8_t wifi_channel = 1;
    uint32_t currentTime = 0;
    uint32_t packetSize = 0;
    uint32_t packetCounter = 0;
    uint32_t attackTime = 0;
    uint32_t packetRateTime = 0;
};

#endif
