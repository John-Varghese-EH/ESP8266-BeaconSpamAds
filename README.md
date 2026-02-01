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

## Installation
1.  Open `ESP8266_BeaconSpam_Merged/ESP8266_BeaconSpam_Merged.ino` in Arduino IDE.
2.  **Install LittleFS**: Ensure you have the ESP8266 LittleFS filesystem uploader installed or just allow the sketch to format it on first run (might take a few seconds).
3.  Select your ESP8266 board (e.g., NodeMCU 1.0).
4.  Click **Upload**.

## Configuration
**NEW:** You no longer need to edit code to change settings!
1.  Connect to the **"Connect me!"** WiFi network.
2.  Open a browser and go to `http://192.168.4.1/admin`.
3.  **Dashboard**:
    *   **Settings**: Toggle WPA2, Append Spaces, or change Beacon Interval.
    *   **SSID List**: Edit the text box to add/remove SSIDs (one per line). Click "Save SSIDs".
    *   **Reboot**: Restart the device to apply major changes if needed.

The SSIDs and Settings are saved to the chip's storage, so they will be there next time you turn it on.

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
    -   You need Python installed.
    -   Run: `python utils/build_web.py` from the project root.
    -   This updates `web_index.h` automatically.
4.  Re-upload the sketch to the ESP8266.

## Legal Disclaimer
**Use responsibly.** This tool is for educational and testing purposes only. Ensure you have permission to broadcast on the frequencies and environment you are in. Do not use this to disrupt legitimate services.

## Credits
- Based on the original concepts by [Spacehuhn](https://github.com/spacehuhn) and [John-Varghese-EH](https://github.com/John-Varghese-EH).
- Merged and refactored by Google Deepmind's Antigravity.
