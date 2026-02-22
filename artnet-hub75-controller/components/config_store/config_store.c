#include "config_store.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "config_store";
static app_config_t s_config;
static nvs_handle_t s_nvs;

static void load_str(const char *key, char *dst, size_t max, const char *def)
{
    size_t len = max;
    if (nvs_get_str(s_nvs, key, dst, &len) != ESP_OK) {
        strncpy(dst, def, max - 1);
        dst[max - 1] = '\0';
    }
}

static void load_u16(const char *key, uint16_t *dst, uint16_t def)
{
    if (nvs_get_u16(s_nvs, key, dst) != ESP_OK) {
        *dst = def;
    }
}

static void load_u8(const char *key, uint8_t *dst, uint8_t def)
{
    if (nvs_get_u8(s_nvs, key, dst) != ESP_OK) {
        *dst = def;
    }
}

esp_err_t config_store_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition issue, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &s_nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    memset(&s_config, 0, sizeof(s_config));

    load_str("wifi_ssid", s_config.wifi_ssid, sizeof(s_config.wifi_ssid), "");
    load_str("wifi_pass", s_config.wifi_pass, sizeof(s_config.wifi_pass), "");
    load_u16("start_uni", &s_config.start_universe, DEFAULT_START_UNIVERSE);
    load_u8("brightness", &s_config.brightness, DEFAULT_BRIGHTNESS);
    load_u16("panel_w",   &s_config.panel_width, DEFAULT_PANEL_WIDTH);
    load_u16("panel_h",   &s_config.panel_height, DEFAULT_PANEL_HEIGHT);
    load_str("node_name", s_config.node_name, sizeof(s_config.node_name), "ArtNet-HUB75");
    load_u8("scan_type",  &s_config.scan_type, DEFAULT_SCAN_TYPE);
    load_u8("gamma_en",   &s_config.gamma_enabled, 1);

    /* Validate ranges */
    if (s_config.brightness == 0) s_config.brightness = 1;
    if (s_config.panel_width < 16 || s_config.panel_width > 256) s_config.panel_width = DEFAULT_PANEL_WIDTH;
    if (s_config.panel_height < 8 || s_config.panel_height > 128) s_config.panel_height = DEFAULT_PANEL_HEIGHT;
    if (s_config.scan_type != 4 && s_config.scan_type != 8 &&
        s_config.scan_type != 16 && s_config.scan_type != 32) {
        s_config.scan_type = DEFAULT_SCAN_TYPE;
    }

    ESP_LOGI(TAG, "Config loaded: %dx%d panel, universe %u, brightness %u, name '%s'",
             s_config.panel_width, s_config.panel_height,
             s_config.start_universe, s_config.brightness, s_config.node_name);

    return ESP_OK;
}

const app_config_t *config_store_get(void)
{
    return &s_config;
}

esp_err_t config_store_save(const app_config_t *cfg)
{
    nvs_set_str(s_nvs, "wifi_ssid", cfg->wifi_ssid);
    nvs_set_str(s_nvs, "wifi_pass", cfg->wifi_pass);
    nvs_set_u16(s_nvs, "start_uni", cfg->start_universe);
    nvs_set_u8(s_nvs,  "brightness", cfg->brightness);
    nvs_set_u16(s_nvs, "panel_w",   cfg->panel_width);
    nvs_set_u16(s_nvs, "panel_h",   cfg->panel_height);
    nvs_set_str(s_nvs, "node_name", cfg->node_name);
    nvs_set_u8(s_nvs,  "scan_type", cfg->scan_type);
    nvs_set_u8(s_nvs,  "gamma_en",  cfg->gamma_enabled);

    esp_err_t err = nvs_commit(s_nvs);
    if (err == ESP_OK) {
        memcpy(&s_config, cfg, sizeof(app_config_t));
        ESP_LOGI(TAG, "Config saved");
    }
    return err;
}

esp_err_t config_store_save_wifi(const char *ssid, const char *pass)
{
    strncpy(s_config.wifi_ssid, ssid, sizeof(s_config.wifi_ssid) - 1);
    s_config.wifi_ssid[sizeof(s_config.wifi_ssid) - 1] = '\0';
    strncpy(s_config.wifi_pass, pass, sizeof(s_config.wifi_pass) - 1);
    s_config.wifi_pass[sizeof(s_config.wifi_pass) - 1] = '\0';

    nvs_set_str(s_nvs, "wifi_ssid", s_config.wifi_ssid);
    nvs_set_str(s_nvs, "wifi_pass", s_config.wifi_pass);
    return nvs_commit(s_nvs);
}

uint16_t config_store_get_start_universe(void)   { return s_config.start_universe; }
uint8_t  config_store_get_brightness(void)        { return s_config.brightness; }
uint16_t config_store_get_panel_width(void)       { return s_config.panel_width; }
uint16_t config_store_get_panel_height(void)      { return s_config.panel_height; }
const char *config_store_get_node_name(void)      { return s_config.node_name; }

uint16_t config_store_get_total_channels(void)
{
    return s_config.panel_width * s_config.panel_height * 3;
}

uint16_t config_store_get_num_universes(void)
{
    uint16_t total = config_store_get_total_channels();
    return (total + ARTNET_CHANNELS_PER_UNI - 1) / ARTNET_CHANNELS_PER_UNI;
}
