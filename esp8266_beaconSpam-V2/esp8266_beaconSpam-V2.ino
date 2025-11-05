/*
  ESP8266 Beacon Spam
  version: 2.0
  github.com/John-Varghese-EH
*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Web server on port 80
ESP8266WebServer server(80);

// Define your SSIDs here (max 32 chars, no duplicates)
const char* ssid_list[] = {
  "Mom Use This One",
  "Abraham Linksys",
  "Benjamin FrankLAN",
  "Martin Router King",
  "John Wilkes Bluetooth",
  "Pretty Fly for a Wi-Fi",
  "Bill Wi the Science Fi",
  "I Believe Wi Can Fi",
  "Tell My Wi-Fi Love Her",
  "No More Mister Wi-Fi"
  // add more here as needed
};
const int total_ssids = sizeof(ssid_list) / sizeof(ssid_list[0]);

const uint8_t channels[] = {1, 6, 11};  // WiFi channels used
const int num_channels = sizeof(channels) / sizeof(channels[0]);

const bool appendSpaces = true;

// HTML ad page as C string literal
const char* adPageHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Special Offer</title>
<style>
  body { font-family: Arial, sans-serif; text-align: center; margin: 20px; }
  h1 { color: #2c3e50; }
  img { max-width: 100%; height: auto; margin-top: 20px; }
  a { display: inline-block; margin-top: 20px; padding: 10px 20px; background: #2980b9; color: white; text-decoration: none; border-radius: 5px; }
  a:hover { background: #3498db; }
</style>
</head>
<body>
  <h1>Welcome to Free Wi-Fi!</h1>
  <p>Don't miss our limited-time special offer:</p>
  <img src="https://your-ad-image-url.com/ad-banner.jpg" alt="Awesome Product" />
  <p><a href="https://your-product-page.com" target="_blank" rel="noopener">Click here to learn more!</a></p>
</body>
</html>
)rawliteral";

// Beacon packet template (modify as per your original)
uint8_t beaconPacket[109] = {
  0x80, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x00, 0x00,
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,
  0xe8, 0x03,
  0x31, 0x00,
  0x00, 0x20,
  // SSID placeholder (filled later)
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20,
  0x01, 0x08,
  0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
  0x03, 0x01,
  0x01,
  0x30, 0x18,
  0x01, 0x00,
  0x00, 0x0f, 0xac, 0x02,
  0x02, 0x00,
  0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
  0x01, 0x00,
  0x00, 0x0f, 0xac, 0x02,
  0x00, 0x00
};

char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = sizeof(beaconPacket);
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

// Random MAC generator
void randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
}

// Channel cycling
void nextChannel() {
  channelIndex++;
  if (channelIndex >= num_channels) channelIndex = 0;
  wifi_channel = channels[channelIndex];
  wifi_set_channel(wifi_channel);
}

void handleCaptivePortal() {
  server.send(200, "text/html", adPageHtml);
}

void handleNotFound() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  randomSeed(os_random());

  for (int i = 0; i < 32; i++) emptySSID[i] = ' ';

  randomMac();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("Connect Me");  // AP for web clients

  wifi_set_opmode(STATION_MODE);  // For beacon spam mode
  wifi_set_channel(channels[0]);

  server.on("/generate_204", handleCaptivePortal);
  server.on("/hotspot-detect.html", handleCaptivePortal);
  server.on("/connecttest.txt", handleCaptivePortal);
  server.on("/", handleCaptivePortal);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Setup complete. Waiting to send beacon packets...");
}

void loop() {
  server.handleClient();
  currentTime = millis();

  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    nextChannel();

    static int ssidNum = 0;
    // Cycle through one SSID per attack cycle for pacing
    const char* ssid = ssid_list[ssidNum % total_ssids];
    ssidNum++;

    uint8_t ssidLen = strlen(ssid);
    macAddr[5] = ssidNum;

    // Update MAC in beacon frame
    memcpy(&beaconPacket[10], macAddr, 6);
    memcpy(&beaconPacket[16], macAddr, 6);

    // Reset SSID area
    memcpy(&beaconPacket[38], emptySSID, 32);
    // Write new SSID
    memcpy(&beaconPacket[38], ssid, ssidLen);

    // Set channel in beacon frame
    beaconPacket[82] = wifi_channel;

    if (appendSpaces) {
      for (int k = 0; k < 3; k++) {
        packetCounter += wifi_send_pkt_freedom(beaconPacket, packetSize, 0) == 0;
        delay(1);
      }
    } else {
      // Optimization: skipping dynamic allocation here for simplicity
      packetCounter += wifi_send_pkt_freedom(beaconPacket, packetSize, 0) == 0;
    }
  }

  if (currentTime - packetRateTime > 1000) {
    packetRateTime = currentTime;
    Serial.printf("Packets/s: %d\n", packetCounter);
    packetCounter = 0;
  }
}
