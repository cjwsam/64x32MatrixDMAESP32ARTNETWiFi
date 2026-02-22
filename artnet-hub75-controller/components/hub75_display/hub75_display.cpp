/*
 * HUB75 Display Driver — wraps ESP32-HUB75-MatrixPanel-DMA
 *
 * Manages double-buffered framebuffers and provides a C-callable API.
 * The library handles I2S DMA refresh autonomously on its own task.
 */

#include "hub75_display.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

/* ESP32-HUB75-MatrixPanel-DMA library */
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

static const char *TAG = "hub75_display";

static MatrixPanel_I2S_DMA *s_matrix = nullptr;
static uint8_t *s_buf_a = nullptr;     /* framebuffer A */
static uint8_t *s_buf_b = nullptr;     /* framebuffer B */
static uint8_t *s_back  = nullptr;     /* current back buffer (artnet writes here) */
static uint8_t *s_front = nullptr;     /* current front buffer (display reads from here) */
static size_t   s_frame_size = 0;
static uint16_t s_width = 0;
static uint16_t s_height = 0;
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static TaskHandle_t s_render_task = nullptr;

/* Push the front buffer contents to the display hardware. */
static void render_front_to_matrix(void)
{
    const uint8_t *p = s_front;
    for (int y = 0; y < s_height; y++) {
        for (int x = 0; x < s_width; x++) {
            s_matrix->drawPixelRGB888(x, y, p[0], p[1], p[2]);
            p += 3;
        }
    }
}

/* Render task: waits for notification from artnet, then pushes pixels. */
static void display_render_task(void *arg)
{
    ESP_LOGI(TAG, "Render task started on core %d", xPortGetCoreID());

    for (;;) {
        /* Wait up to 100ms for a frame notification */
        uint32_t val = 0;
        if (xTaskNotifyWait(0, ULONG_MAX, &val, pdMS_TO_TICKS(100)) == pdTRUE) {
            render_front_to_matrix();
        }
    }
}

extern "C" esp_err_t hub75_display_init(const app_config_t *cfg)
{
    s_width  = cfg->panel_width;
    s_height = cfg->panel_height;
    s_frame_size = (size_t)s_width * s_height * 3;

    /* Configure the matrix panel */
    HUB75_I2S_CFG::i2s_pins pins = {
        HUB75_R1, HUB75_G1, HUB75_B1,
        HUB75_R2, HUB75_G2, HUB75_B2,
        HUB75_A, HUB75_B_PIN, HUB75_C, HUB75_D, HUB75_E,
        HUB75_LAT, HUB75_OE, HUB75_CLK
    };

    HUB75_I2S_CFG mxconfig(
        s_width,    /* panel width */
        s_height,   /* panel height */
        1,          /* chain length */
        pins        /* pin mapping */
    );

    mxconfig.double_buff = true;
    mxconfig.clkphase = false;

    /* Select driver based on scan type */
    switch (cfg->scan_type) {
    case 8:
        mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
        break;
    case 32:
        mxconfig.driver = HUB75_I2S_CFG::FM6126A;
        break;
    default:
        mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
        break;
    }

    s_matrix = new MatrixPanel_I2S_DMA(mxconfig);
    if (!s_matrix->begin()) {
        ESP_LOGE(TAG, "MatrixPanel_I2S_DMA::begin() failed");
        return ESP_FAIL;
    }

    s_matrix->setBrightness8(cfg->brightness);
    s_matrix->clearScreen();

    /* Allocate double framebuffers in DMA-capable internal SRAM */
    s_buf_a = (uint8_t *)heap_caps_calloc(1, s_frame_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    s_buf_b = (uint8_t *)heap_caps_calloc(1, s_frame_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!s_buf_a || !s_buf_b) {
        ESP_LOGE(TAG, "Failed to allocate framebuffers (%u bytes each)", (unsigned)s_frame_size);
        return ESP_ERR_NO_MEM;
    }

    s_back  = s_buf_a;
    s_front = s_buf_b;

    ESP_LOGI(TAG, "Display initialized: %ux%u, %u bytes/frame, brightness=%u",
             s_width, s_height, (unsigned)s_frame_size, cfg->brightness);
    return ESP_OK;
}

extern "C" uint8_t *hub75_display_get_back_buffer(void)
{
    return s_back;
}

extern "C" size_t hub75_display_get_frame_size(void)
{
    return s_frame_size;
}

extern "C" void hub75_display_swap_and_render(void)
{
    portENTER_CRITICAL(&s_lock);
    uint8_t *tmp = s_back;
    s_back = s_front;
    s_front = tmp;
    portEXIT_CRITICAL(&s_lock);

    /* Notify render task that a new frame is ready */
    if (s_render_task) {
        xTaskNotify(s_render_task, 1, eSetValueWithOverwrite);
    }
}

extern "C" void hub75_display_set_brightness(uint8_t brightness)
{
    if (s_matrix) {
        s_matrix->setBrightness8(brightness);
    }
}

extern "C" void hub75_display_self_test(void)
{
    if (!s_matrix) return;

    struct { uint8_t r, g, b; } colors[] = {
        {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}
    };

    for (int c = 0; c < 4; c++) {
        s_matrix->fillScreenRGB888(colors[c].r, colors[c].g, colors[c].b);
        s_matrix->flipDMABuffer();
        vTaskDelay(pdMS_TO_TICKS(400));
    }

    s_matrix->clearScreen();
    s_matrix->flipDMABuffer();
    ESP_LOGI(TAG, "Self-test complete");
}

extern "C" void hub75_display_start_render_task(void)
{
    xTaskCreatePinnedToCore(
        display_render_task,
        "display",
        DISPLAY_TASK_STACK,
        nullptr,
        DISPLAY_TASK_PRIORITY,
        &s_render_task,
        DISPLAY_TASK_CORE
    );
}

extern "C" void *hub75_display_get_task_handle(void)
{
    return (void *)s_render_task;
}
