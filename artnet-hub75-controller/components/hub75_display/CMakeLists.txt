idf_component_register(
    SRCS "hub75_display.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver
    PRIV_REQUIRES esp_log
)
