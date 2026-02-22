/*
 * Art-Net HUB75 LED Matrix Controller — ESP-IDF
 *
 * Receives Art-Net DMX data over WiFi and drives a 64x32 (or configurable)
 * HUB75 LED matrix panel via I2S DMA. Production-grade rewrite of the
 * original Arduino-based project.
 *
 * Architecture:
 *   Core 0: WiFi stack + Art-Net UDP receiver + Web server
 *   Core 1: HUB75 DMA refresh (autonomous) + Display render task
 */

#include "app_config.h"
#include "config_store.h"
#include "status_led.h"
#include "hub75_display.h"
#include "wifi_prov.h"
#include "web_server.h"
#include "artnet.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "=== ArtNet HUB75 Controller v%d.%d ===",
             FW_VERSION_MAJOR, FW_VERSION_MINOR);
    ESP_LOGI(TAG, "Free heap at start: %lu bytes", (unsigned long)esp_get_free_heap_size());

    /* Phase 1: NVS + configuration */
    ESP_ERROR_CHECK(config_store_init());
    const app_config_t *cfg = config_store_get();

    /* Phase 2: Status LED */
    status_led_init();
    status_led_set_state(STATUS_BOOTING);

    /* Phase 3: HUB75 display */
    esp_err_t disp_err = hub75_display_init(cfg);
    if (disp_err != ESP_OK) {
        ESP_LOGE(TAG, "Display init failed: %s", esp_err_to_name(disp_err));
        status_led_set_state(STATUS_ERROR);
        return;
    }
    hub75_display_self_test();

    /* Phase 4: WiFi */
    status_led_set_state(STATUS_WIFI_CONNECTING);
    esp_err_t wifi_err = wifi_prov_start();
    if (wifi_err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi connected: %s", wifi_prov_get_ip());
        status_led_set_state(STATUS_WIFI_CONNECTED);
    } else {
        ESP_LOGW(TAG, "AP mode: %s", wifi_prov_get_ip());
        status_led_set_state(STATUS_AP_MODE);
    }

    /* Phase 5: Web server (always — for config and OTA) */
    web_server_start(wifi_err != ESP_OK);

    /* Phase 6: Start display render task */
    hub75_display_start_render_task();

    /* Phase 7: Start Art-Net receiver (only if WiFi connected) */
    if (wifi_err == ESP_OK) {
        artnet_start(cfg);
        status_led_set_state(STATUS_ARTNET_ACTIVE);
    }

    ESP_LOGI(TAG, "Initialization complete. Free heap: %lu bytes",
             (unsigned long)esp_get_free_heap_size());

    /* app_main returns — its task is deleted automatically */
}
