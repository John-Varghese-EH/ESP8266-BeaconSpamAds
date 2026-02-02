# ⚡ ESP Beacon Spam & Captive Portal

![ESP8266](https://img.shields.io/badge/ESP8266-Supported-green?logo=espressif)
![ESP32](https://img.shields.io/badge/ESP32-Supported-green?logo=espressif)
![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)
![Version](https://img.shields.io/badge/version-2.0-blue)

A powerful WiFi beacon spammer and captive portal for ESP8266/ESP32. Broadcasts fake SSIDs, captures users with a customizable captive portal, and redirects them anywhere you want.

---

## 📑 Table of Contents

- [Features](#-features)
- [Compatible Hardware](#-compatible-hardware)
- [Quick Start](#-quick-start)
- [Configuration](#-configuration)
- [Web Interface](#-web-interface)
- [SSID Presets](#-ssid-presets)
- [Custom Portal HTML](#-custom-portal-html)
- [API Reference](#-api-reference)
- [Project Structure](#-project-structure)
- [Troubleshooting](#-troubleshooting)
- [Security Notes](#-security-notes)
- [Legal Disclaimer](#-legal-disclaimer)

---

## 🔧 Compatible Hardware

### ESP8266 Boards (Full Support)

| Board | Flash | Notes |
|-------|-------|-------|
| **NodeMCU v1.0/v2/v3** | 4MB | ⭐ Recommended - Most popular |
| **Wemos D1 Mini** | 4MB | Compact, breadboard-friendly |
| **Wemos D1 Mini Pro** | 16MB | External antenna support |
| **ESP-12E/ESP-12F** | 4MB | Module form factor |
| **ESP-07** | 4MB | Ceramic antenna + U.FL connector |
| **Adafruit Feather HUZZAH** | 4MB | LiPo battery support |
| **SparkFun ESP8266 Thing** | 4MB | Built-in battery charging |
| **ESP-01S** | 1MB | ⚠️ Limited GPIO, basic use only |

### ESP32 Boards (Full Support + BLE Spam)

| Board | Flash | Notes |
|-------|-------|-------|
| **ESP32 DevKit v1** | 4MB | ⭐ Recommended - Most common |
| **ESP32-WROOM-32** | 4MB | Standard module |
| **ESP32-S3** | 8MB+ | Better WiFi range, USB-C |
| **ESP32-C3** | 4MB | RISC-V, lower power |
| **TTGO T-Display** | 4MB | Built-in 1.14" TFT screen |
| **Wemos Lolin32** | 4MB | Compact ESP32 |
| **M5Stack Core** | 16MB | Display, battery, case included |
| **LilyGO boards** | 4MB+ | Various form factors |

### Minimum Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| **Flash** | 1MB | 4MB+ |
| **RAM** | 80KB | 320KB (ESP32) |
| **WiFi** | 802.11 b/g/n | Same |

## ✨ Features

| Feature | Description |
|---------|-------------|
| **Beacon Spam** | Broadcasts 50+ fake WiFi networks simultaneously |
| **Captive Portal** | Intercepts connections and shows your custom page |
| **Web Admin Panel** | Configure everything wirelessly at `/admin` |
| **Authentication** | Password-protected admin panel with rate limiting |
| **Custom Advertising** | Set headline, description, button text, and redirect URL |
| **Portal HTML Editor** | Upload custom HTML for the captive portal |
| **Auto-Redirect** | Optional countdown timer to auto-redirect users |
| **WPA2 Toggle** | Make fake networks appear as encrypted |
| **BLE Spam (ESP32)** | Bluetooth beacon spam for iOS/Android |
| **Persistent Storage** | Settings saved to LittleFS, survive reboots |
| **Dark/Light Theme** | Toggle between dark and light mode |
| **SSID Presets** | 6 themed packs (Random, Funny, Scary, Memes, Pop Culture, Trolling) |
| **Export/Import Config** | Backup and restore all settings as JSON |
| **Channel Selection** | Choose WiFi channel 1-14 for beacons |
| **MAC Randomization** | Random source MAC for each beacon |
| **Connected Clients** | View MAC & IP of connected devices |

---

## 🚀 Quick Start

### 1. Flash the Firmware

**Arduino IDE:**
```
1. Install ESP8266/ESP32 board package
2. Install LittleFS library
3. Open ESP_BeaconSpamAds/ESP_BeaconSpamAds.ino
4. Select Board: NodeMCU 1.0 (ESP8266) or ESP32 Dev Module
5. Upload
```

**PlatformIO (Recommended):**
```bash
pio run --target uploadfs   # Upload filesystem (SSIDs)
pio run --target upload     # Upload firmware
```

### 2. Connect & Configure

| Step | Action |
|------|--------|
| 1 | Connect to WiFi: **`Connect Me!!!`** |
| 2 | Open browser: **`http://192.168.4.1/admin`** |
| 3 | Login: **`admin`** / **`beacon`** |
| 4 | Configure settings and SSIDs |
| 5 | Click **Save** and **Reboot** |

---

## ⚙️ Configuration

### Default Settings (`Settings.h`)

Edit these before flashing to customize defaults:

```cpp
// Admin credentials
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PASS "beacon"

// Portal advertising
#define DEFAULT_HEADLINE "⚡ ESP Beacon Spam"
#define DEFAULT_DESCRIPTION "Open-source WiFi beacon spammer & captive portal"
#define DEFAULT_BUTTON_TEXT "View on GitHub"
#define DEFAULT_REDIRECT_URL "https://github.com/John-Varghese-EH/ESP8266-BeaconSpamAds"

// Access point
#define DEFAULT_AP_NAME "Connect Me!!!"
#define DEFAULT_HIDE_AP false

// Timing
#define DEFAULT_BEACON_INTERVAL 100  // ms between beacons
#define DEFAULT_AUTO_REDIRECT_DELAY 0  // 0 = disabled

// BLE (ESP32 only)
#define DEFAULT_ENABLE_BLE false
```

### Runtime Settings

All settings can be changed via the admin panel:

| Category | Setting | Description |
|----------|---------|-------------|
| **Access Point** | AP Name | WiFi network users connect to |
| | Hide AP | Hide from network lists (experimental) |
| **Portal** | Headline | Main title on portal page |
| | Description | Sub-text below headline |
| | Button Text | Action button text |
| | Redirect URL | Destination after clicking |
| | Auto-Redirect | Seconds before auto-redirect (0 = off) |
| **Beacon** | WPA2 | Show networks as encrypted |
| | Beacon Interval | Delay between transmissions (ms) |
| | WiFi Channel | Broadcast channel (1-14) |
| | MAC Randomization | Random source MAC |
| **ESP32** | BLE Spam | Enable Bluetooth spam |

---

## 🌐 Web Interface

### Admin Panel (`/admin`)

The admin panel provides full control over the device:

- **Status Bar** - Uptime, connected clients, SSID count
- **Access Point** - Configure AP name and visibility
- **Portal Advertising** - Customize the captive portal content
- **Spam Settings** - WPA2, intervals, channels, MAC randomization
- **SSID List** - Edit SSIDs with preset generator buttons
- **Backup/Restore** - Export/Import configuration as JSON
- **Portal HTML Editor** - Upload custom HTML for the portal
- **Admin Credentials** - Change username/password
- **System** - Reboot device

### Captive Portal

When users connect to your network:
1. Auto sign-in popup appears (iOS, Android, Windows, macOS)
2. Your custom portal page is displayed
3. User clicks button → Redirected to your URL
4. Optional: Auto-redirect after countdown

---

## 🎲 SSID Presets

The admin panel includes 6 preset categories:

| Preset | Examples | Count |
|--------|----------|-------|
| 🎲 **Random** | Auto-generated realistic names | 25 |
| 😂 **Funny** | "FBI Surveillance Van", "Pretty Fly for a WiFi" | 20 |
| � **Scary** | "Virus Distribution Center", "NSA Listening Post" | 16 |
| 🐸 **Memes** | "Never Gonna Give You Up", "Doge Network" | 16 |
| 📺 **Pop Culture** | "Hogwarts WiFi", "Stark Industries Guest" | 14 |
| 😈 **Trolling** | "Connecting...", "Lag Generator 3000" | 14 |

Click any preset button to instantly populate the SSID list.

---

## 📝 Custom Portal HTML

You can upload your own HTML for the captive portal:

1. Go to **Admin Panel** → **Portal HTML Editor**
2. Write or paste your HTML (max 8KB)
3. Click **Save HTML**
4. Enable **Use Custom Portal HTML** checkbox
5. Click **Save All Settings**
6. **Reboot** the device

**Tips:**
- Keep HTML under 8KB for ESP8266 memory
- Test on mobile devices
- Use inline CSS/JS (no external files)

## 🔗 Deep Link Support

You can set a specific redirect URL for each SSID. When a user connects to that network (by setting it as the AP name), they will be redirected to your custom URL.

**Format:** `SSID Name|https://custom-url.com`

**Example:**
```
Free WiFi
Starbucks Guest|https://starbucks.com
McDonalds Free|https://mcdonalds.com
```

### How to use:
1. Add SSIDs with the `|` delimiter and URL in the SSID list.
2. Change the **Access Point Name** to match one of these SSIDs.
3. Save and Reboot.
4. Users connecting to the AP will be redirected to the custom URL.

---

## 🔌 API Reference

All admin endpoints require HTTP Basic Authentication.

### Configuration API

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/data` | GET | Get all configuration data |
| `/api/save_config` | POST | Save configuration (JSON in `data` field) |
| `/api/save_ssids` | POST | Save SSID list (`ssids` field) |
| `/api/export` | GET | Download config as JSON file |
| `/api/import` | POST | Import config from JSON |
| `/api/reboot` | POST | Reboot the device |

### Client API

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/clients` | GET | List connected clients (MAC, IP) |
| `/api/public_data` | GET | Public portal data (no auth) |

### Portal HTML API

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/portal_html` | GET | Get current portal HTML |
| `/api/save_portal_html` | POST | Save custom HTML (`html` field) |
| `/api/reset_portal_html` | POST | Reset to default portal |

---

## 📁 Project Structure

```
ESP_BeaconSpamAds/
├── ESP_BeaconSpamAds.ino    # Main entry point
├── BeaconSpam.cpp/h         # WiFi beacon broadcasting
├── BleSpam.cpp/h            # Bluetooth spam (ESP32)
├── CaptivePortal.cpp/h      # Web server & DNS hijacking
├── Storage.cpp/h            # LittleFS config management
├── Settings.h               # Default configuration
├── web_index.h              # Auto-generated HTML (don't edit)
├── web/                     # Source files for web UI
│   ├── admin/
│   │   ├── index.html       # Admin page
│   │   ├── style.css        # Admin styles
│   │   └── app.js           # Admin JavaScript
│   └── portal.html          # Captive portal page
├── data/                    # LittleFS files
│   └── ssids.txt            # Default SSID list
└── utils/
    └── build.py             # Web UI compiler
```

### Rebuilding Web Assets

After editing files in `web/`:

```bash
cd ESP_BeaconSpamAds
python utils/build.py
```

This regenerates `web_index.h` with minified HTML/CSS/JS.

---

## 🔍 Troubleshooting

### SSID Not Changing After Updating `Settings.h`

The default AP name is only applied on **first boot** or when config version changes.

**To force reset:**

| Method | Command |
|--------|---------|
| Arduino IDE | Tools → Erase Flash → All Flash Contents |
| PlatformIO | `pio run --target erase && pio run --target upload` |
| esptool | `esptool.py --port COM3 erase_flash` |

### Captive Portal Popup Not Appearing

1. **Wait 5-10 seconds** after connecting
2. **Open any HTTP site** (not HTTPS) in browser
3. **Try `http://192.168.4.1`** directly
4. **Test different devices** (iOS works best)

### Can't Access Admin Panel

1. **URL**: Use `http://192.168.4.1/admin` (not HTTPS)
2. **Same network**: Must be connected to ESP's AP
3. **Credentials**: Default is `admin` / `beacon`
4. **Lockout**: Wait 5 minutes after 5 failed attempts

### SSID Generator Buttons Not Working

1. **Rebuild web assets**: Run `python utils/build.py`
2. **Re-flash firmware** after rebuilding
3. **Check browser console** for JavaScript errors

### Settings Not Saving

1. **Reboot required**: AP name, WiFi channel need reboot
2. **Flash corrupted**: Erase flash and re-upload

### Custom Portal HTML Not Showing

1. **Enable toggle**: Check "Use Custom Portal HTML"
2. **Save settings**: Click "Save All Settings"
3. **Reboot**: Required for portal changes

---

## 🛡️ Security Notes

> ⚠️ **Important Security Considerations**

- **Change default credentials** before deploying
- Admin panel uses HTTP Basic Auth (not encrypted)
- Passwords stored in plaintext on device
- Rate limiting: 5 failed attempts = 5 minute lockout
- No HTTPS support (ESP8266 memory limitations)

**Recommendations:**
- Use in controlled environments only
- Change admin password immediately after setup
- Don't store sensitive data on the device

---

## ⚠️ Legal Disclaimer

**Use responsibly.** This tool is for **educational and testing purposes only**.

- ❌ Do not use to disrupt legitimate WiFi services
- ❌ Do not use for phishing or credential theft
- ✅ Ensure you have permission to broadcast
- ✅ Use only in controlled environments

Broadcasting fake networks may violate local regulations. The author is not responsible for misuse.

---

## 🙏 Credits

- Original beacon spam concept by [Spacehuhn](https://github.com/spacehuhn)
- Project by [John-Varghese-EH](https://github.com/John-Varghese-EH)

---

## 📄 License

Apache 2.0 License - See [LICENSE](LICENSE) for details.

---

<p align="center">
  Made with ❤️ for the ESP community
</p>
