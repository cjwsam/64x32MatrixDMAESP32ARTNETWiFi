# 64x32 LED Matrix DMA - ESP32 Art-Net WiFi Controller

Production-grade Art-Net (DMX over IP) receiver for ESP32 that drives HUB75 LED matrix panels via I2S DMA. Fully configurable via web interface with OTA firmware updates.

## V2 — ESP-IDF Rewrite

The project has been completely rewritten in ESP-IDF (no Arduino dependency) for production use. The new code is in `artnet-hub75-controller/`.

**Key improvements over the original Arduino version:**
- Proper FreeRTOS dual-core task design (Core 0: networking, Core 1: display)
- Double-buffered frame assembly with 20ms timeout (never freezes on lost packets)
- Art-Net 4 compliant: ArtDmx + ArtPoll reply + ArtSync support
- Web-based configuration and OTA firmware updates
- WiFi provisioning via SoftAP captive portal
- NVS persistent configuration (replaces EEPROM)
- No pin conflicts (OLED on GPIO 21/22, separate from HUB75)
- Proper watchdog management via esp_task_wdt API
- BSD sockets with timeout (replaces complex AsyncUDP queue system)

### Quick Start

```bash
cd artnet-hub75-controller
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

See [`artnet-hub75-controller/README.md`](artnet-hub75-controller/README.md) for full documentation.

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 (4MB+ flash) |
| LED Matrix | 64x32 HUB75 (1/16 scan) |
| Power Supply | 5V, 4A minimum |
| Optional | SSD1306 OLED on GPIO 21/22 |

### HUB75 Pin Map (V2 — ESP-IDF)

| Signal | GPIO | Signal | GPIO |
|--------|------|--------|------|
| R1 | 25 | R2 | 14 |
| G1 | 26 | G2 | 12 |
| B1 | 27 | B2 | 13 |
| A | 23 | CLK | 16 |
| B | 19 | LAT | 4 |
| C | 5 | OE | 15 |
| D | 17 | GND | GND |

## Art-Net Configuration

- **Total pixels:** 2048 (64 x 32)
- **Channels:** 6144 (RGB)
- **Universes:** 12

Compatible with Resolume, MadMapper, Jinx!, QLC+, xLights, and any Art-Net controller.

## Legacy Arduino Code

The original Arduino sketches are preserved in the root directory for reference:
- `64x32MatrixDMAESP32ARTNETWiFi.ino` — Simple version (hardcoded WiFi)
- `2020final.ino` — Advanced version (WiFi Manager, OLED, EEPROM)

## License

MIT
