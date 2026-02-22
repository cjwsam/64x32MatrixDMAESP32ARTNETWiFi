idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        artnet
        hub75_display
        wifi_prov
        config_store
        web_server
        status_led
        esp_wifi
        esp_event
        nvs_flash
        esp_http_server
        esp_timer
        esp_app_format
)
