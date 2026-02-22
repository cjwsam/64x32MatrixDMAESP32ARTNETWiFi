#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATUS_BOOTING,          /* Steady fast blink (100ms) */
    STATUS_WIFI_CONNECTING,  /* Fast blink (200ms) */
    STATUS_WIFI_CONNECTED,   /* Slow blink (1s) — no Art-Net yet */
    STATUS_AP_MODE,          /* Double flash pattern */
    STATUS_ARTNET_ACTIVE,    /* Solid on */
    STATUS_ERROR,            /* Triple fast flash */
} status_led_state_t;

/** Initialize the status LED GPIO. */
void status_led_init(void);

/** Set the current status LED pattern. */
void status_led_set_state(status_led_state_t state);

#ifdef __cplusplus
}
#endif
