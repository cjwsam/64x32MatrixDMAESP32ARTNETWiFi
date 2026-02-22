#pragma once

#include "app_config.h"
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the HUB75 display driver and allocate double framebuffers.
 */
esp_err_t hub75_display_init(const app_config_t *cfg);

/**
 * Get pointer to the back buffer for writing pixel data.
 * Format: sequential RGB bytes, row-major, top-left origin.
 */
uint8_t *hub75_display_get_back_buffer(void);

/** Get the frame buffer size in bytes. */
size_t hub75_display_get_frame_size(void);

/**
 * Swap front/back buffers and push the new front buffer to the display.
 * Thread-safe (uses critical section for pointer swap).
 */
void hub75_display_swap_and_render(void);

/** Set display brightness (1-255). */
void hub75_display_set_brightness(uint8_t brightness);

/** Run a self-test color sequence (R/G/B/W, 500ms each). */
void hub75_display_self_test(void);

/** Start the display render task on DISPLAY_TASK_CORE. */
void hub75_display_start_render_task(void);

/** Get the render task handle for xTaskNotify. */
void *hub75_display_get_task_handle(void);

#ifdef __cplusplus
}
#endif
