#include "status_led.h"
#include "app_config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "status_led";
static esp_timer_handle_t s_timer;
static status_led_state_t s_state = STATUS_BOOTING;
static bool s_led_on = false;
static int s_phase = 0;

static void set_led(bool on)
{
    s_led_on = on;
    gpio_set_level(STATUS_LED_GPIO, on ? 1 : 0);
}

static void timer_cb(void *arg)
{
    switch (s_state) {
    case STATUS_BOOTING:
        set_led(!s_led_on);
        break;

    case STATUS_WIFI_CONNECTING:
        set_led(!s_led_on);
        break;

    case STATUS_WIFI_CONNECTED:
        set_led(!s_led_on);
        break;

    case STATUS_AP_MODE:
        /* Double flash: ON-OFF-ON-OFF-------- */
        s_phase = (s_phase + 1) % 8;
        set_led(s_phase == 0 || s_phase == 2);
        break;

    case STATUS_ARTNET_ACTIVE:
        set_led(true);
        break;

    case STATUS_ERROR:
        /* Triple fast flash */
        s_phase = (s_phase + 1) % 10;
        set_led(s_phase == 0 || s_phase == 2 || s_phase == 4);
        break;
    }
}

void status_led_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << STATUS_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    set_led(false);

    esp_timer_create_args_t timer_args = {
        .callback = timer_cb,
        .name = "status_led",
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &s_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_timer, 100 * 1000)); /* 100ms */

    ESP_LOGI(TAG, "Status LED initialized on GPIO %d", STATUS_LED_GPIO);
}

void status_led_set_state(status_led_state_t state)
{
    s_state = state;
    s_phase = 0;

    /* Adjust timer period based on state */
    uint64_t period_us;
    switch (state) {
    case STATUS_BOOTING:         period_us = 100000;  break; /* 100ms */
    case STATUS_WIFI_CONNECTING: period_us = 200000;  break; /* 200ms */
    case STATUS_WIFI_CONNECTED:  period_us = 1000000; break; /* 1s */
    case STATUS_AP_MODE:         period_us = 150000;  break; /* 150ms per phase */
    case STATUS_ARTNET_ACTIVE:   period_us = 1000000; break; /* 1s (solid) */
    case STATUS_ERROR:           period_us = 100000;  break; /* 100ms */
    default:                     period_us = 500000;  break;
    }

    esp_timer_stop(s_timer);
    esp_timer_start_periodic(s_timer, period_us);
}
