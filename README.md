# ESP8266 Beacon Spam & Captive Portal Ads

## Overview
This project combines the best features of previous iterations to turn an ESP8266 into a powerful tool that:
1.  **Broadcasts fake WiFi SSIDs** (Beacon Spam) to attract attention.
2.  **Creates a "Connect me!" Access Point** with a Captive Portal.
3.  **Serves an Ad/Message Page** to anyone who connects.

**Improved & Modularized**: This version uses a clean, modular structure (`BeaconSpam` and `CaptivePortal` classes) and includes a comprehensive list of creative SSIDs from the original project.

## Features
-   **Web Admin Panel**: Manage SSIDs and settings wirelessly from your phone or browser.
-   **Dynamic Storage**: SSIDs are saved to internal memory (LittleFS), so they persist after reboot.
-   **Modular Codebase**: Easier to read, maintain, and expand.
-   **Extensive SSID List**: Over 50+ creative and funny SSIDs pre-loaded.
-   **WPA2 Support**: Toggleable option to make fake networks appear encrypted.
-   **Captive Portal**: Automatic redirect for connected users to your custom HTML page.
-   **Status LED**: Blinks to indicate the system is running and spamming.

## Quick Customization (SSIDs)
You can easily change the list of fake networks without writing code:
1.  Open `ESP_BeaconSpamAds/data/ssids.txt`.
2.  Add or remove names (one per line).
3.  **Upload the changes**:
    *   **Arduino IDE**: Use "LittleFS Data Upload" tool.
    *   **PlatformIO**: Run "Upload Filesystem Image" task.
    *   **Web Admin**: Go to `/admin` after flashing and paste your list there.

## Installation
### Method 1: Arduino IDE
1.  Open `ESP_BeaconSpamAds/ESP_BeaconSpamAds.ino`.
2.  (Optional) Run `python utils/build_web.py` if you modified the web interface.
3.  Select Board (NodeMCU 1.0) and Upload.
4.  **Important**: Upload the filesystem data using the "ESP8266 LittleFS Data Upload" plugin to load the default SSIDs.

### Method 2: PlatformIO (Recommended)
1.  Open this project folder in VS Code with PlatformIO installed.
2.  Click the **PlatformIO Alien icon** on the left.
3.  Under `nodemcuv2`, click **Upload Filesystem Image** (to load SSIDs).
4.  Click **Upload** (to flash firmware).

## Configuration
**Runtime Config**: Connect to the **"Connect me!"** network and visit `http://192.168.4.1/admin`.
**Manual Config**: Edit `data/ssids.txt` before uploading via LittleFS.

## Usage
1.  Power on the ESP8266.
2.  **LED Status**: The onboard LED will blink to show it is active.
3.  **Beacon Spam**: Check your phone's WiFi list to see the fake networks.
4.  **Captive Portal**: Connect to **"Connect me!"** to see the Ad page.
5.  **Admin**: Visit `/admin` to configure the device on the fly.

## Web Development (Advanced)
If you want to customize the look of the Admin Panel or the Captive Portal page:
1.  Navigate to `ESP_BeaconSpamAds/web/`.
2.  Edit `index.html` (Admin), `portal.html` (Ad Page), `style.css`, or `app.js`.
3.  **Compile the changes**:
    -   Run: `python utils/build_web.py` from the project root.
    -   This updates `web_index.h` automatically.
4.  Re-upload the firmware.

## Legal Disclaimer
**Use responsibly.** This tool is for educational and testing purposes only. Ensure you have permission to broadcast on the frequencies and environment you are in. Do not use this to disrupt legitimate services.

## Credits
- Based on the original concepts by [Spacehuhn](https://github.com/spacehuhn) and [John-Varghese-EH](https://github.com/John-Varghese-EH).
- Merged and refactored by Google Deepmind's Antigravity.
