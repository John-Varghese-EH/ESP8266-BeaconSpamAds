# ⚡ ESP8266 Beacon Spam & Captive Portal

![ESP8266](https://img.shields.io/badge/ESP8266-Supported-green?logo=espressif)
![ESP32](https://img.shields.io/badge/ESP32-Supported-green?logo=espressif)
![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)

A powerful WiFi beacon spammer and captive portal for ESP8266/ESP32. Broadcasts fake SSIDs, captures users with a customizable captive portal, and redirects them anywhere you want.

## ✨ Features

| Feature | Description |
|---------|-------------|
| **Beacon Spam** | Broadcasts 50+ fake WiFi networks simultaneously |
| **Captive Portal** | Intercepts connections and shows your custom page |
| **Web Admin Panel** | Configure everything wirelessly at `/admin` |
| **Authentication** | Password-protected admin panel (browser popup) |
| **Custom Advertising** | Set headline, description, button text, and redirect URL |
| **Auto-Redirect** | Optional countdown timer to auto-redirect users |
| **WPA2 Toggle** | Make fake networks appear as encrypted |
| **BLE Spam (ESP32)** | Bluetooth beacon spam for iOS/Android |
| **Persistent Storage** | Settings saved to LittleFS, survive reboots |
| **Dark/Light Theme** | Toggle between dark and light mode |
| **Random SSID Generator** | Generate random realistic WiFi network names |
| **SSID Preset Packs** | One-click themed lists (Funny, Scary, Memes) |
| **Export/Import Config** | Backup and restore all settings as JSON |
| **Channel Selection** | Choose WiFi channel 1-14 for beacons |
| **MAC Randomization** | Random source MAC for each beacon |
| **Connected Clients** | View MAC & IP of connected devices |

## 🚀 Quick Start

### 1. Flash the Firmware

**Arduino IDE:**
```bash
1. Open ESP_BeaconSpamAds/ESP_BeaconSpamAds.ino
2. Select Board: NodeMCU 1.0 (ESP8266) or ESP32 Dev Module
3. Upload
```

**PlatformIO (Recommended):**
```bash
1. Open project in VS Code with PlatformIO
2. Click Upload Filesystem Image (loads default SSIDs)
3. Click Upload (flashes firmware)
```

### 2. Connect & Configure

1. **Connect** to the WiFi network: `Connect Me!!!` (or your custom AP name)
2. **Access Admin**: Navigate to `http://192.168.4.1/admin`
3. **Login**: Default credentials are `admin` / `beacon`
4. **Configure**: Set your headlines, redirect URLs, and SSIDs

## ⚙️ Configuration

### Default Settings (`Settings.h`)

Edit these before flashing to customize defaults:

```cpp
// Admin credentials
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PASS "beacon"

// Portal advertising
#define DEFAULT_HEADLINE "⚡ ESP8266 Beacon Spam"
#define DEFAULT_DESCRIPTION "Open-source WiFi beacon spammer & captive portal"
#define DEFAULT_BUTTON_TEXT "View on GitHub"
#define DEFAULT_REDIRECT_URL "https://github.com/John-Varghese-EH/ESP8266-BeaconSpamAds"

// Access point
#define DEFAULT_AP_NAME "Connect Me!!!"

// Timing
#define DEFAULT_BEACON_INTERVAL 100  // ms between beacons
#define DEFAULT_AUTO_REDIRECT_DELAY 0  // 0 = disabled
```

### Runtime Configuration

All settings can be changed at runtime via the web admin panel:

| Setting | Description |
|---------|-------------|
| **AP Name** | The WiFi network name users connect to |
| **Hide AP** | Hide from network lists (experimental) |
| **Headline** | Main title on the portal page |
| **Description** | Sub-text below the headline |
| **Button Text** | Text on the action button |
| **Redirect URL** | Where users go after clicking |
| **Auto-Redirect** | Seconds before auto-redirect (0 = disabled) |
| **WPA2** | Make beacon networks appear encrypted |
| **BLE Spam** | Enable Bluetooth spam (ESP32 only) |
| **Beacon Interval** | Delay between beacon transmissions |
| **WiFi Channel** | Channel to broadcast beacons on (1-14) |
| **MAC Randomization** | Use random source MAC for each beacon |

## 🌐 Web Interface

### Admin Panel (`/admin`)
- Dark/Light theme toggle (saved preference)
- Real-time status (uptime, connected clients, SSID count)
- Connected clients list with MAC & IP addresses
- Random SSID generator + preset packs
- Export/Import configuration backup
- SSID list editor

### Captive Portal
- Auto-displays when users connect
- Dynamic content from your settings
- Optional auto-redirect countdown
- Minimal, clean design

## 🔧 Customizing the Web UI

1. Edit files in `ESP_BeaconSpamAds/web/`:
   - `admin/index.html` - Admin page
   - `admin/style.css` - Admin styles
   - `admin/app.js` - Admin JavaScript
   - `portal.html` - Captive portal page

2. Build the header file:
   ```bash
   python utils/build.py
   ```

3. Re-upload firmware

## 📁 Project Structure

```
ESP_BeaconSpamAds/
├── ESP_BeaconSpamAds.ino    # Main entry point
├── BeaconSpam.cpp/h         # WiFi beacon broadcasting
├── CaptivePortal.cpp/h      # Web server & DNS hijacking
├── Storage.cpp/h            # LittleFS config management
├── Settings.h               # Default configuration
├── web_index.h              # Auto-generated HTML (don't edit)
├── web/                     # Source files for web UI
│   ├── admin/
│   │   ├── index.html
│   │   ├── style.css
│   │   └── app.js
│   └── portal.html
├── data/                    # LittleFS files
│   └── ssids.txt
└── utils/
    └── build.py             # Web UI compiler
```

## 🛡️ Security Notes

- **Change default credentials** before deploying
- Admin panel uses HTTP Basic Auth
- Passwords are stored in plaintext on the device
- This is designed for controlled environments only

## ⚠️ Legal Disclaimer

**Use responsibly.** This tool is for **educational and testing purposes only**.

- Do not use to disrupt legitimate WiFi services
- Ensure you have permission to broadcast in your environment
- Broadcasting fake networks may violate local regulations
- The author is not responsible for misuse

## 🙏 Credits

- Original beacon spam concept by [Spacehuhn](https://github.com/spacehuhn)
- Project by [John-Varghese-EH](https://github.com/John-Varghese-EH)

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.
