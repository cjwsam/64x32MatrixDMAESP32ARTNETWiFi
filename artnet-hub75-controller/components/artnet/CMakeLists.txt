idf_component_register(
    SRCS "artnet.c" "artnet_pollreply.c"
    INCLUDE_DIRS "include"
    REQUIRES lwip esp_timer esp_netif config_store hub75_display
    PRIV_REQUIRES esp_log esp_wifi
)
