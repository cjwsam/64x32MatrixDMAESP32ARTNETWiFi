#include "wifi_prov.h"
#include "config_store.h"
#include "app_config.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/inet.h"
#include <string.h>

static const char *TAG = "wifi_prov";

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static EventGroupHandle_t s_wifi_events;
static bool s_connected = false;
static char s_ip_str[16] = "0.0.0.0";
static int s_retry_count = 0;
#define MAX_RETRIES 3

static void wifi_event_handler(void *arg, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    if (base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "STA started, connecting...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            s_connected = false;
            if (s_retry_count < MAX_RETRIES) {
                s_retry_count++;
                ESP_LOGW(TAG, "Disconnected, retry %d/%d", s_retry_count, MAX_RETRIES);
                esp_wifi_connect();
            } else {
                ESP_LOGW(TAG, "Connection failed after %d retries", MAX_RETRIES);
                xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
            }
            break;

        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t *evt = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "Station connected to AP, AID=%d", evt->aid);
            break;
        }
        default:
            break;
        }
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *evt = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip_str, sizeof(s_ip_str), IPSTR, IP2STR(&evt->ip_info.ip));
        ESP_LOGI(TAG, "Connected, IP: %s", s_ip_str);
        s_connected = true;
        s_retry_count = 0;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t start_ap(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    wifi_config_t ap_cfg = {
        .ap = {
            .max_connection = AP_MAX_CONNECTIONS,
            .authmode = WIFI_AUTH_OPEN,
            .channel = 1,
        },
    };
    snprintf((char *)ap_cfg.ap.ssid, sizeof(ap_cfg.ap.ssid),
             "%s%02X%02X", AP_SSID_PREFIX, mac[4], mac[5]);
    ap_cfg.ap.ssid_len = strlen((char *)ap_cfg.ap.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    snprintf(s_ip_str, sizeof(s_ip_str), "192.168.4.1");

    ESP_LOGI(TAG, "AP mode started: SSID='%s', IP=%s", ap_cfg.ap.ssid, s_ip_str);
    return ESP_OK;
}

esp_err_t wifi_prov_start(void)
{
    s_wifi_events = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                &wifi_event_handler, NULL));

    const app_config_t *cfg = config_store_get();

    /* Try STA if credentials exist */
    if (strlen(cfg->wifi_ssid) > 0) {
        ESP_LOGI(TAG, "Attempting STA connection to '%s'...", cfg->wifi_ssid);

        wifi_config_t sta_cfg = {};
        strncpy((char *)sta_cfg.sta.ssid, cfg->wifi_ssid, sizeof(sta_cfg.sta.ssid) - 1);
        strncpy((char *)sta_cfg.sta.password, cfg->wifi_pass, sizeof(sta_cfg.sta.password) - 1);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
        ESP_ERROR_CHECK(esp_wifi_start());

        EventBits_t bits = xEventGroupWaitBits(s_wifi_events,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE,
            pdMS_TO_TICKS(STA_CONNECT_TIMEOUT_MS));

        if (bits & WIFI_CONNECTED_BIT) {
            return ESP_OK;
        }

        ESP_LOGW(TAG, "STA connection timed out, switching to AP mode");
        esp_wifi_stop();
    } else {
        ESP_LOGI(TAG, "No WiFi credentials, starting AP mode");
    }

    /* Fall back to AP mode */
    s_retry_count = 0;
    start_ap();
    return ESP_ERR_WIFI_NOT_CONNECT;
}

bool wifi_prov_is_connected(void)
{
    return s_connected;
}

const char *wifi_prov_get_ip(void)
{
    return s_ip_str;
}

int8_t wifi_prov_get_rssi(void)
{
    if (!s_connected) return 0;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    return 0;
}
