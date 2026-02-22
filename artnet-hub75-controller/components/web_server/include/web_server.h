#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the HTTP web server.
 * @param provisioning_mode  If true, unmatched URIs redirect to config page.
 */
esp_err_t web_server_start(bool provisioning_mode);

/** Stop the web server. */
esp_err_t web_server_stop(void);

/** Update stats displayed on the status endpoint. */
void web_server_update_stats(uint32_t fps, uint32_t frames_ok, uint32_t frames_partial);

#ifdef __cplusplus
}
#endif
