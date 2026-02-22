#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Firmware version */
#define FW_VERSION_MAJOR  2
#define FW_VERSION_MINOR  0

/* Panel defaults */
#define DEFAULT_PANEL_WIDTH   64
#define DEFAULT_PANEL_HEIGHT  32
#define DEFAULT_SCAN_TYPE     16
#define DEFAULT_BRIGHTNESS    128

/* Art-Net defaults */
#define DEFAULT_START_UNIVERSE  0
#define ARTNET_PORT             6454
#define ARTNET_MAX_UNIVERSES    16
#define ARTNET_CHANNELS_PER_UNI 512
#define ARTNET_MAX_PACKET_SIZE  530

/* Frame assembly */
#define FRAME_TIMEOUT_MS   20
#define RECV_TIMEOUT_MS    5

/* WiFi provisioning */
#define STA_CONNECT_TIMEOUT_MS  10000
#define AP_SSID_PREFIX          "ArtNet-"
#define AP_MAX_CONNECTIONS      4

/* Task configuration */
#define ARTNET_TASK_STACK    4096
#define ARTNET_TASK_PRIORITY 12
#define ARTNET_TASK_CORE     0

#define DISPLAY_TASK_STACK    4096
#define DISPLAY_TASK_PRIORITY 20
#define DISPLAY_TASK_CORE     1

#define WEB_SERVER_STACK      8192

/* Status LED */
#define STATUS_LED_GPIO  2

/* HUB75 pin assignment (conflict-free) */
#define HUB75_R1   25
#define HUB75_G1   26
#define HUB75_B1   27
#define HUB75_R2   14
#define HUB75_G2   12
#define HUB75_B2   13
#define HUB75_A    23
#define HUB75_B_PIN 19
#define HUB75_C    5
#define HUB75_D    17
#define HUB75_E    -1   /* Not used for 32-row panels */
#define HUB75_CLK  16
#define HUB75_LAT  4
#define HUB75_OE   15

/* I2C for optional OLED (conflict-free) */
#define I2C_SDA_GPIO  21
#define I2C_SCL_GPIO  22

/* NVS namespace */
#define NVS_NAMESPACE "artnet_cfg"

/* Configuration structure shared across components */
typedef struct {
    char     wifi_ssid[33];
    char     wifi_pass[65];
    uint16_t start_universe;
    uint8_t  brightness;
    uint16_t panel_width;
    uint16_t panel_height;
    char     node_name[18];
    uint8_t  scan_type;
    uint8_t  gamma_enabled;
} app_config_t;
