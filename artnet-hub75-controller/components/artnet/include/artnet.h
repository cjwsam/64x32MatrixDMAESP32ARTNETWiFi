#pragma once

#include "app_config.h"
#include "artnet_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the Art-Net receiver.
 * Creates a UDP socket on port 6454 and spawns the receive task on Core 0.
 */
esp_err_t artnet_start(const app_config_t *cfg);

/** Stop the Art-Net receiver and close the socket. */
void artnet_stop(void);

/** Get current statistics. */
void artnet_get_stats(artnet_stats_t *stats);

/**
 * Send an ArtPollReply to the given address (called internally on ART_POLL).
 * Exposed for testing.
 */
void artnet_send_poll_reply(int sock, const struct sockaddr_in *requester);

#ifdef __cplusplus
}
#endif
