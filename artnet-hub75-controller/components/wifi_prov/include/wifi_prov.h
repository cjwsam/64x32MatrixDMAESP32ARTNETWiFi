#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start WiFi provisioning flow:
 *   1. Try STA connection with NVS credentials (10s timeout)
 *   2. On failure, start SoftAP for provisioning
 *
 * Returns ESP_OK if STA connected, ESP_ERR_WIFI_NOT_CONNECT if in AP mode.
 */
esp_err_t wifi_prov_start(void);

/** Check if WiFi is connected in STA mode. */
bool wifi_prov_is_connected(void);

/** Get the current IP address string (STA or AP). */
const char *wifi_prov_get_ip(void);

/** Get WiFi RSSI (0 if not connected). */
int8_t wifi_prov_get_rssi(void);

#ifdef __cplusplus
}
#endif
