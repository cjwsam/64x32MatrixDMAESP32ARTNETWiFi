# 64x32 LED Matrix DMA - ESP32 Art-Net WiFi Controller

An ESP32-based controller that drives a 64x32 HUB75 LED matrix panel via DMA, receiving pixel data over WiFi using the Art-Net (DMX over IP) protocol. This allows any Art-Net compatible software (such as MadMapper, Resolume, Jinx!, or GLEDIATOR) to control the LED matrix wirelessly in real time.

## Features

- Receives Art-Net DMX data over WiFi and displays it on a 64x32 HUB75 LED matrix
- Uses DMA (Direct Memory Access) via the SmartMatrix library for flicker-free output
- Supports multi-universe Art-Net (up to 13 universes for full 64x32 RGB)
- Two firmware versions included:
  - **64x32MatrixDMAESP32ARTNETWiFi.ino** -- Simple version with hard-coded WiFi credentials
  - **2020final.ino** -- Advanced version with WiFi Manager captive portal, OLED status display, EEPROM-saved settings, and configurable universe mapping

## Hardware Requirements

| Component | Details |
|---|---|
| Microcontroller | ESP32 (ESP32-DevKitC or similar) |
| LED Matrix Panel | 64x32 HUB75 LED matrix (1/16 scan, MOD16SCAN) |
| Power Supply | 5V, 4A minimum (depends on brightness and panel) |
| OLED Display (advanced version only) | SSD1306 128x64 I2C OLED |
| Wiring | Dupont jumper wires or custom PCB |

## HUB75 Pin Connections (ESP32 to LED Matrix)

The SmartMatrix library uses specific ESP32 GPIO pins for the HUB75 interface. Default pin mapping for ESP32 with SmartMatrix3:

| HUB75 Pin | ESP32 GPIO | Function |
|---|---|---|
| R1 | GPIO 2 | Red data (upper half) |
| G1 | GPIO 15 | Green data (upper half) |
| B1 | GPIO 4 | Blue data (upper half) |
| R2 | GPIO 16 | Red data (lower half) |
| G2 | GPIO 27 | Green data (lower half) |
| B2 | GPIO 17 | Blue data (lower half) |
| A | GPIO 5 | Row select A |
| B | GPIO 18 | Row select B |
| C | GPIO 19 | Row select C |
| D | GPIO 21 | Row select D |
| CLK | GPIO 22 | Clock |
| LAT | GPIO 26 | Latch |
| OE | GPIO 25 | Output enable |
| GND | GND | Ground (connect at least one) |

> **Note:** Pin assignments may vary depending on your SmartMatrix library version. Check the SmartMatrix documentation or your `MatrixHardware_ESP32_V0.h` file for exact pin mappings.

### OLED Display Wiring (Advanced Version)

| SSD1306 Pin | ESP32 GPIO |
|---|---|
| SDA | GPIO 5 |
| SCL | GPIO 4 |
| VCC | 3.3V |
| GND | GND |

## Art-Net Configuration

### Channel Layout

- **Total pixels:** 2048 (64 wide x 32 tall)
- **Channels per pixel:** 3 (Red, Green, Blue)
- **Total channels:** 6144
- **Universes required:** 13 (512 channels per universe, with the last universe partially filled)

### Simple Version Settings

In `64x32MatrixDMAESP32ARTNETWiFi.ino`, configure:

```cpp
const int startUniverse = 0;  // First Art-Net universe (some software uses 0, others use 1)
const int numberOfChannels = 6144;  // 64 * 32 * 3 = 6144 RGB channels
```

### Advanced Version Settings

In `2020final.ino`, Art-Net universe settings are configurable at runtime via the WiFi Manager captive portal:

- **Start Universe** -- The first Art-Net universe to listen for
- **End Universe** -- The last Art-Net universe to listen for
- **Process Priority** -- Task priority for the Art-Net reading loop (adjust if experiencing dropped frames)

These settings are stored in EEPROM and persist across reboots.

## Network Setup

### Simple Version

1. Edit `64x32MatrixDMAESP32ARTNETWiFi.ino` and set your WiFi credentials:
   ```cpp
   const char ssid[] = "YourWiFiName";
   const char pwd[]  = "YourWiFiPassword";
   ```
2. Flash the ESP32. It will connect to the specified network automatically.
3. Check the Serial Monitor (115200 baud) for the assigned IP address.

### Advanced Version

1. On first boot (or after a reset), the ESP32 creates an access point named **"Chris's Config XXXX"** (where XXXX is a chip ID).
2. Connect to this access point with your phone or computer.
3. A captive portal will open. Select your WiFi network, enter the password, and configure the Art-Net universe settings.
4. The ESP32 will save the settings to EEPROM and reboot into normal operation.
5. The OLED display shows boot status (checkmark for success, X for failure).

### Art-Net Software Setup

1. In your Art-Net software, set the target IP to the ESP32's IP address (shown on Serial Monitor).
2. Configure the output to send the correct number of universes starting from your configured start universe.
3. Map your content to a 64x32 pixel grid with RGB color order.

## Build and Flash Instructions

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x)
- [ESP32 Board Support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) installed via Boards Manager

### Required Libraries

Install the following libraries via the Arduino Library Manager or manually:

| Library | Purpose |
|---|---|
| [SmartMatrix](https://github.com/pixelmatix/SmartMatrix) | HUB75 LED matrix DMA driver (edited version included/required) |
| [FastLED](https://github.com/FastLED/FastLED) | RGB color type conversions |
| [WiFiManager](https://github.com/tzapu/WiFiManager) (development branch) | Captive portal WiFi setup (advanced version) |
| [SSD1306Wire / ESP8266 and ESP32 OLED driver](https://github.com/ThingPulse/esp8266-oled-ssd1306) | OLED display (advanced version) |
| [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) | Graphics primitives (advanced version) |

> **Note:** This project includes edited third-party libraries (`ArtnetWifiAsync`, `AsyncUDPCLEAN`) that are customized for async Art-Net reception. These are included in the repository and do not need to be installed separately.

### Flashing

1. Open the desired `.ino` file in Arduino IDE.
2. Select your ESP32 board from **Tools > Board**.
3. Select the correct COM port from **Tools > Port**.
4. Set **Upload Speed** to 921600 (or 115200 if you have issues).
5. Set **Partition Scheme** to "Default 4MB with spiffs" or similar.
6. Click **Upload**.

## Compatible LED Matrix Panels

This project is configured for **64x32 HUB75 panels with 1/16 scan (MOD16SCAN)**. The panel type is set with:

```cpp
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;
```

Compatible panels include:

- P3 64x32 HUB75 indoor panels (1/16 scan)
- P4 64x32 HUB75 indoor panels (1/16 scan)
- P5 64x32 HUB75 indoor panels (1/16 scan)

For **32x16 panels** (1/8 scan), change the panel type to:

```cpp
const uint8_t kPanelType = SMARTMATRIX_HUB75_16ROW_MOD8SCAN;
```

And update `kMatrixWidth` and `kMatrixHeight` accordingly.

## Troubleshooting

- **No display output:** Double-check HUB75 wiring, ensure GND is connected, verify 5V power supply is adequate.
- **Flickering or garbled display:** Ensure the correct panel type / scan rate is configured. Try reducing `kRefreshDepth` or `kDmaBufferRows`.
- **WiFi not connecting:** Check credentials (simple version) or reset the ESP32 to re-enter the captive portal (advanced version).
- **Art-Net data not received:** Confirm the ESP32 and the Art-Net source are on the same network/subnet. Check that the universe numbers match. Verify with Serial Monitor output.
- **Watchdog resets:** The advanced version feeds the watchdog timer manually. If you experience resets, try adjusting the process priority value.

## File Overview

| File | Description |
|---|---|
| `64x32MatrixDMAESP32ARTNETWiFi.ino` | Simple standalone sketch with hard-coded WiFi |
| `2020final.ino` | Advanced sketch with WiFi Manager, OLED, EEPROM config |
| `ArtnetWifiAsync.h` / `ArtnetWifiAsync.cpp` | Custom async Art-Net protocol parser |
| `AsyncUDPCLEAN.h` / `AsyncUDPCLEAN.cpp` | Custom async UDP implementation for ESP32 |
| `images.h` | XBM images for OLED pass/fail display |

## License

This code is for personal and educational use only, except by 2Entertain on Colorline Fantasy.

## Author

Created by Christopher Willis ([cjwsam](https://github.com/cjwsam))
