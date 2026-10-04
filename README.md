# PATHIK-project_yatra-
PATHIK is a smart, IoT-based tourist guide designed to make travel more interactive and informative. It helps travelers explore destinations by providing location-based information, historical insights, heritage sites, hidden gems, and local food recommendations through an easy-to-use portable device.

## 🧰 Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| ESP32 Dev Board | 1 | |
| SSD1306 OLED 128x64 (I2C, address 0x3C) | 1 | |
| GPS module (e.g. NEO-6M, 9600 baud) | 1 | |
| MAX98357A I2S amplifier | 1 | |
| Speaker (4–8 Ω) | 1 | |
| Push buttons | 5 | History, Tourist Spot, Hidden Gem, Food, Exit |
| Breadboard + jumper wires | 1 set | |
| USB data cable | 1 | |
| Laptop or Raspberry Pi | 1 | Runs `backend.py` |

## 🔌 Hardware Assembly

Mount everything on a breadboard and wire it as follows.

**OLED display (I2C)**

| OLED | ESP32 |
|------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 23 |

**GPS module** (TX and RX are crossed)

| GPS | ESP32 |
|-----|-------|
| VCC | 3.3V (or 5V, check your module) |
| GND | GND |
| TX | GPIO 4 |
| RX | GPIO 2 |

**MAX98357A amplifier + speaker**

| MAX98357A | ESP32 |
|-----------|-------|
| VIN | 5V (VIN) |
| GND | GND |
| DIN | GPIO 27 |
| BCLK | GPIO 26 |
| LRC | GPIO 25 |
| SPK + / – | Speaker terminals |

**Buttons** (internal pull-ups are used, so no resistors are needed). Connect one leg of each button to the GPIO and the other leg to GND.

| Button | ESP32 Pin |
|--------|-----------|
| History | GPIO 14 |
| Tourist Spot | GPIO 33 |
| Hidden Gem | GPIO 13 |
| Food | GPIO 18 |
| Exit | GPIO 19 |

> Double-check VCC/GND on every module before powering on. A GPS module needs a clear view of the sky to get a fix, so test it near a window or outdoors.

## ⚙️ ESP32 Software Setup

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Add ESP32 board support:
   - **File → Preferences** → *Additional Boards Manager URLs*, add:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - **Tools → Board → Boards Manager** → search **esp32** → install.
3. Install these libraries (**Sketch → Include Library → Manage Libraries**):
   - `Adafruit SSD1306`
   - `Adafruit GFX Library`
   - `TinyGPSPlus` (by Mikal Hart)
   - `ArduinoJson` (version 6.x recommended)
   - `ESP8266Audio` (by Earle F. Philhower III). It provides the `AudioFileSourceHTTPStream`, `AudioGeneratorMP3` and `AudioOutputI2S` classes, and it works on ESP32.

## 🔧 Changing the 3 Parameters in `esp32_code.ino`

Edit these lines near the top of the file (lines 18, 19 and 26):

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER        = "YOUR_IP_ADDRESS(changes every time you change wifi)";
```

### 1. `WIFI_SSID`
Your WiFi or hotspot name, exactly as it appears (case-sensitive).
```cpp
const char* WIFI_SSID = "MyHotspot";
```
> The ESP32 only supports **2.4 GHz** WiFi. On a phone hotspot, enable 2.4 GHz / "Maximize compatibility".

### 2. `WIFI_PASSWORD`
The password of that network.
```cpp
const char* WIFI_PASSWORD = "mypassword123";
```

### 3. `SERVER`
This is the address of the laptop/Raspberry Pi running `backend.py`. It must include **`http://`**, the **IP address**, and the **port `:5000`**.
```cpp
const char* SERVER = "http://192.168.1.5:5000";
```

**The IP changes every time you connect to a different WiFi, so update it each time.** The laptop/Pi and the ESP32 must be on the **same network**.

Find your laptop's IP:
- **Windows:** run `ipconfig` and look for *IPv4 Address* under the WiFi adapter.
- **Linux / Raspberry Pi:** run `hostname -I`.
- **macOS:** run `ipconfig getifaddr en0`.

## 📤 Uploading to the ESP32

1. **Tools → Board → ESP32 Dev Module**.
2. **Tools → Port** → select your ESP32's COM port.
3. Click **Upload**. If it hangs on "Connecting...", hold the **BOOT** button on the ESP32.
4. Open **Tools → Serial Monitor** at **115200 baud** to see WiFi, GPS and server logs.

## 🖥️ Running the Backend

> Run the backend on a **laptop or Raspberry Pi on the same WiFi as the ESP32**. It will not work from GitHub Codespaces, because the ESP32 can't reach it.

1. Install [Python 3.9+](https://www.python.org/downloads/).
2. Clone the repo:
```bash
   git clone https://github.com/<your-username>/PATHIK-project_yatra-.git
   cd PATHIK-project_yatra-
```
3. Create a virtual environment:
```bash
   python -m venv venv
```
4. Activate it:
   - **Windows (CMD):** `venv\Scripts\activate`
   - **Windows (PowerShell):** `venv\Scripts\Activate.ps1`
   - **Linux / macOS / Raspberry Pi:** `source venv/bin/activate`
5. Install Flask:
```bash
   pip install flask
```
   (Or `pip install -r requirements.txt` if the repo includes one.)
6. Add the narration audio files. Create an `audios/` folder next to `backend.py` and put MP3 files named `<place>_<category>.mp3` in it:
```
   audios/
   ├── delhi_history.mp3
   ├── delhi_tourist_spot.mp3
   ├── delhi_hidden_gem.mp3
   ├── delhi_food.mp3
   ├── mumbai_history.mp3
   └── ... (same pattern for kashmir, surat, jaipur, meghalaya)
```
7. Start the server:
```bash
   python backend.py
```
   It runs on `http://0.0.0.0:5000`. Keep this terminal open.
8. **Windows only:** if the ESP32 can't connect, allow Python through the firewall, or allow inbound TCP port 5000.
9. To test, open `http://<your-ip>:5000` in a browser on your phone (same WiFi). You should see "Tourist Guide Backend is running!".

## ▶️ How to Use PATHIK

1. Start `backend.py` on your laptop/Pi.
2. Power the ESP32. The OLED shows *Starting → WiFi Connecting → Ready*.
3. The device waits for a GPS fix. If none is found within **15 seconds**, it falls back to Delhi coordinates (for demos and indoor testing).
4. The OLED shows your detected location (e.g. *Delhi*), then asks you to pick a category.
5. Press **History / Tourist Spot / Hidden Gem / Food**. The OLED shows the info and the speaker plays the narration.
6. After each narration you can pick another category, or press **Exit** to finish.

**Supported regions:** Delhi, Mumbai, Surat, Jaipur, Kashmir and Meghalaya. Anywhere else shows "Unsupported region".

## ✏️ Customizing the Content

- **Text shown on the OLED:** edit the `tourist_data` dictionary in `backend.py`.
- **Add a new city:** add an entry to `tourist_data`, add its latitude/longitude range in `detect_state()`, add it to `allowed_states` in `get_audio()`, and add its 4 MP3 files.

## 🛠️ Troubleshooting

| Problem | Fix |
|---------|-----|
| "WiFi Error" on OLED | Check SSID/password and that the network is 2.4 GHz |
| OLED blank / "OLED initialization failed" | Check SDA = 21, SCL = 23 and the I2C address (0x3C) |
| Stuck on "Looking for GPS" | Move near a window or outdoors, check GPS TX/RX wiring (they're crossed). It falls back to Delhi after 15 s |
| HTTP response `-1` in Serial Monitor | `SERVER` is wrong (missing `http://` or `:5000`), the IP changed, or the firewall is blocking the port |
| "Audio Error" | The MP3 file is missing from `audios/` (the filename must match, e.g. `delhi_history.mp3`) |
| No sound | Check the MAX98357A wiring (DIN 27, BCLK 26, LRC 25) and the speaker connection |
| Upload fails | Try another USB cable (some are charge-only) and hold the BOOT button |
| PowerShell won't activate the venv | Run `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass` first |