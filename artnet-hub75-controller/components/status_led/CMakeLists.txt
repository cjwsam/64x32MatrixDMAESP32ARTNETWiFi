idf_component_register(
    SRCS "status_led.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer
    PRIV_REQUIRES esp_log
)
