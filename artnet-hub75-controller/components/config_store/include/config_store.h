#pragma once

#include "app_config.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize NVS and load configuration with defaults.
 * Must be called before any other config_store function.
 */
esp_err_t config_store_init(void);

/** Get pointer to the current (read-only) configuration. */
const app_config_t *config_store_get(void);

/** Save a new configuration to NVS and update the in-memory copy. */
esp_err_t config_store_save(const app_config_t *cfg);

/** Save only WiFi credentials. */
esp_err_t config_store_save_wifi(const char *ssid, const char *pass);

/** Convenience accessors */
uint16_t config_store_get_start_universe(void);
uint8_t  config_store_get_brightness(void);
uint16_t config_store_get_panel_width(void);
uint16_t config_store_get_panel_height(void);
uint16_t config_store_get_total_channels(void);
uint16_t config_store_get_num_universes(void);
const char *config_store_get_node_name(void);

#ifdef __cplusplus
}
#endif
