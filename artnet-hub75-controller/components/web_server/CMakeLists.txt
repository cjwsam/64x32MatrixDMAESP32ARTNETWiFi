idf_component_register(
    SRCS "web_server.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server config_store esp_ota_ops esp_app_format
    PRIV_REQUIRES esp_log esp_wifi esp_timer esp_netif json
)
