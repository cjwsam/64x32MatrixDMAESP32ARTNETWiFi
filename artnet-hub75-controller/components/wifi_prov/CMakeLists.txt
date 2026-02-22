idf_component_register(
    SRCS "wifi_prov.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_event esp_netif config_store
    PRIV_REQUIRES esp_log lwip
)
