# ArtNet HUB75 Controller (ESP-IDF)

Production-grade Art-Net (DMX over IP) receiver for ESP32 that drives HUB75 LED matrix panels via I2S DMA. Configurable via web interface with OTA firmware updates.

## Features

- Art-Net 4 compliant receiver (ArtDmx, ArtPoll, ArtSync)
- 64x32 HUB75 LED matrix via I2S DMA (flicker-free, no CPU involvement)
- Double-buffered frame assembly with 20ms timeout (never freezes on lost packets)
- Web-based configuration and OTA firmware updates
- WiFi provisioning: auto SoftAP captive portal on first boot
- NVS persistent configuration
- FreeRTOS dual-core: Core 0 for networking, Core 1 for display
- Configurable panel size, brightness, Art-Net universe, and scan type

## Hardware

### Required

- ESP32 DevKit (any variant with 4MB+ flash)
- 64x32 HUB75 LED matrix panel (1/16 scan)
- 5V power supply (panel: 2-4A depending on brightness)

### Pin Connections

| Signal | GPIO | Description |
|--------|------|-------------|
| R1 | 25 | Red upper half |
| G1 | 26 | Green upper half |
| B1 | 27 | Blue upper half |
| R2 | 14 | Red lower half |
| G2 | 12 | Green lower half |
| B2 | 13 | Blue lower half |
| A | 23 | Row address A |
| B | 19 | Row address B |
| C | 5 | Row address C |
| D | 17 | Row address D |
| CLK | 16 | Clock |
| LAT | 4 | Latch |
| OE | 15 | Output enable |
| GND | GND | Common ground |

**Optional OLED** (I2C, SSD1306):
| Signal | GPIO |
|--------|------|
| SDA | 21 |
| SCL | 22 |

**Note:** GPIO 12 is a strapping pin. Add a 10K pulldown resistor to ensure it is LOW at boot.

## Build & Flash

Requires [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

```bash
cd artnet-hub75-controller
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## First Boot

1. The ESP32 starts a WiFi access point: **ArtNet-XXXX** (XXXX = last 4 hex digits of MAC)
2. Connect to it with your phone/laptop
3. Open **http://192.168.4.1** in a browser
4. Enter your WiFi credentials and Art-Net settings
5. Click **Save Configuration** — the ESP32 restarts and connects to your network

## Art-Net Configuration

| Setting | Default | Description |
|---------|---------|-------------|
| Start Universe | 0 | First Art-Net universe for this panel |
| Panel Width | 64 | Panel width in pixels |
| Panel Height | 32 | Panel height in pixels |
| Brightness | 128 | Display brightness (1-255) |
| Scan Type | 1/16 | Panel scan rate |

**Channel mapping:** RGB sequential, row-major, top-left origin.
- Pixel (0,0) = channels 0,1,2
- Pixel (1,0) = channels 3,4,5
- A 64x32 panel uses 6144 channels = 12 Art-Net universes

## Web Interface

Access at `http://<device-ip>/` after WiFi connection:

- **Status**: IP, RSSI, FPS, free heap
- **Configuration**: WiFi, Art-Net, display settings
- **OTA Update**: Upload new firmware `.bin` file
- **Restart**: Reboot the controller

## OTA Updates

1. Build new firmware: `idf.py build`
2. Open the web interface
3. Under "Firmware Update", select `build/artnet-hub75-controller.bin`
4. Click **Upload Firmware** — wait for progress bar to complete
5. Controller restarts automatically with new firmware

## Architecture

```
Core 0                          Core 1
┌──────────────────┐           ┌──────────────────┐
│ WiFi stack       │           │ HUB75 I2S DMA    │
│ (system, pri 23) │           │ (autonomous)     │
├──────────────────┤           ├──────────────────┤
│ Art-Net recv     │  notify   │ Display render   │
│ (task, pri 12)   │ ───────> │ (task, pri 20)   │
├──────────────────┤           └──────────────────┘
│ Web server       │
│ (task, pri 5)    │
└──────────────────┘
```

## Supported Art-Net Software

Any Art-Net controller should work:
- Resolume Arena/Avenue
- MadMapper
- Jinx!
- QLC+
- xLights
- DMXIS
- Lightkey

## License

MIT
