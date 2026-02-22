/*
 * Art-Net Protocol Handler
 *
 * Receives Art-Net DMX packets via UDP, assembles complete frames from
 * multiple universes, and signals the display task when a frame is ready.
 *
 * Key features:
 *   - Single blocking recvfrom() with SO_RCVTIMEO for frame timeout
 *   - 20ms frame timeout: renders partial frame if not all universes arrive
 *   - ArtPoll reply for controller discovery
 *   - ArtSync support for synchronized multi-universe output
 *   - Double-buffered: writes to back buffer, atomic swap on complete frame
 */

#include "artnet.h"
#include "hub75_display.h"
#include "config_store.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>

static const char *TAG = "artnet";

static int s_socket = -1;
static TaskHandle_t s_recv_task = NULL;
static artnet_frame_state_t s_frame;
static artnet_stats_t s_stats;

/* FPS calculation */
static uint32_t s_fps_frame_count = 0;
static int64_t  s_fps_last_time = 0;

static void update_fps(void)
{
    s_fps_frame_count++;
    int64_t now = esp_timer_get_time();
    int64_t elapsed = now - s_fps_last_time;
    if (elapsed >= 1000000) { /* 1 second */
        s_stats.fps = (uint32_t)((int64_t)s_fps_frame_count * 1000000 / elapsed);
        s_fps_frame_count = 0;
        s_fps_last_time = now;
    }
}

static void commit_frame(bool is_partial)
{
    hub75_display_swap_and_render();
    s_frame.universe_mask = 0;

    if (is_partial) {
        s_frame.frames_partial++;
        s_stats.frames_partial = s_frame.frames_partial;
    } else {
        s_frame.frames_complete++;
        s_stats.frames_complete = s_frame.frames_complete;
    }

    update_fps();

    /* Copy front buffer content into new back buffer for continuity
       (so partial frames show last-known data for missing universes) */
    size_t fsize = hub75_display_get_frame_size();
    uint8_t *back = hub75_display_get_back_buffer();
    /* The old front is now the back after swap, so it already has the last frame.
       No copy needed — the swap means back now points to the previous front. */
    s_frame.back_buffer = back;
}

static void handle_dmx(const uint8_t *data, int len)
{
    if (len < ARTNET_DMX_START) return;

    const artnet_dmx_packet_t *pkt = (const artnet_dmx_packet_t *)data;

    uint16_t universe = pkt->sub_uni | ((uint16_t)pkt->net << 8);
    uint16_t dmx_len  = ((uint16_t)pkt->length_hi << 8) | pkt->length_lo;

    uint16_t start_uni = config_store_get_start_universe();
    uint16_t num_uni   = config_store_get_num_universes();

    if (universe < start_uni) return;
    int uni_index = universe - start_uni;
    if (uni_index >= num_uni || uni_index >= ARTNET_MAX_UNIVERSES) return;
    if (dmx_len > ARTNET_MAX_DMX_LEN) dmx_len = ARTNET_MAX_DMX_LEN;

    s_frame.total_packets++;
    s_stats.packets_received = s_frame.total_packets;

    /* First universe in a new frame? Record timestamp. */
    if (s_frame.universe_mask == 0) {
        s_frame.frame_start_us = esp_timer_get_time();
    }

    /* Copy DMX data into back buffer at correct offset */
    int buf_offset = uni_index * ARTNET_MAX_DMX_LEN;
    uint16_t total_channels = config_store_get_total_channels();
    int copy_len = dmx_len;
    if (buf_offset + copy_len > total_channels) {
        copy_len = total_channels - buf_offset;
    }
    if (copy_len > 0 && s_frame.back_buffer) {
        memcpy(s_frame.back_buffer + buf_offset, pkt->data, copy_len);
    }

    /* Mark universe received */
    s_frame.universe_mask |= (1 << uni_index);
    s_frame.last_sequence = pkt->sequence;

    /* Check if frame is complete */
    if (s_frame.universe_mask == s_frame.universe_target) {
        commit_frame(false);
    }
}

static void process_packet(const uint8_t *data, int len,
                           const struct sockaddr_in *source)
{
    if (len < 10) return;
    if (memcmp(data, ARTNET_HEADER, ARTNET_HEADER_LEN) != 0) return;

    uint16_t opcode = (uint16_t)data[8] | ((uint16_t)data[9] << 8);

    switch (opcode) {
    case ARTNET_OP_DMX:
        handle_dmx(data, len);
        break;

    case ARTNET_OP_POLL:
        artnet_send_poll_reply(s_socket, source);
        break;

    case ARTNET_OP_SYNC:
        /* Force render of current partial frame */
        if (s_frame.universe_mask != 0) {
            commit_frame(false);
        }
        break;

    default:
        break;
    }
}

static void artnet_recv_task(void *arg)
{
    ESP_LOGI(TAG, "Receive task started on core %d", xPortGetCoreID());

    uint8_t rx_buf[ARTNET_MAX_PACKET];
    struct sockaddr_in source_addr;

    /* Create UDP socket */
    s_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(ARTNET_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };

    if (bind(s_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed: errno %d", errno);
        close(s_socket);
        s_socket = -1;
        vTaskDelete(NULL);
        return;
    }

    /* Set receive timeout for frame timeout checking */
    struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = RECV_TIMEOUT_MS * 1000,
    };
    setsockopt(s_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* Enable broadcast reception */
    int broadcast = 1;
    setsockopt(s_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    ESP_LOGI(TAG, "Listening on UDP port %d", ARTNET_PORT);
    s_fps_last_time = esp_timer_get_time();

    while (1) {
        socklen_t addr_len = sizeof(source_addr);
        int len = recvfrom(s_socket, rx_buf, sizeof(rx_buf), 0,
                           (struct sockaddr *)&source_addr, &addr_len);

        if (len > 0) {
            process_packet(rx_buf, len, &source_addr);
        }

        /* Check frame timeout */
        if (s_frame.universe_mask != 0) {
            int64_t now = esp_timer_get_time();
            if ((now - s_frame.frame_start_us) > (FRAME_TIMEOUT_MS * 1000)) {
                ESP_LOGD(TAG, "Frame timeout (mask=0x%04x, target=0x%04x)",
                         s_frame.universe_mask, s_frame.universe_target);
                commit_frame(true);
            }
        }
    }
}

esp_err_t artnet_start(const app_config_t *cfg)
{
    memset(&s_frame, 0, sizeof(s_frame));
    memset(&s_stats, 0, sizeof(s_stats));

    uint16_t num_uni = config_store_get_num_universes();
    if (num_uni > ARTNET_MAX_UNIVERSES) num_uni = ARTNET_MAX_UNIVERSES;

    /* Build target bitmask: set bits 0..(num_uni-1) */
    s_frame.universe_target = (uint16_t)((1 << num_uni) - 1);
    s_frame.back_buffer = hub75_display_get_back_buffer();

    ESP_LOGI(TAG, "Starting Art-Net: start_uni=%u, num_uni=%u, target_mask=0x%04x, channels=%u",
             cfg->start_universe, num_uni, s_frame.universe_target,
             config_store_get_total_channels());

    xTaskCreatePinnedToCore(
        artnet_recv_task,
        "artnet",
        ARTNET_TASK_STACK,
        NULL,
        ARTNET_TASK_PRIORITY,
        &s_recv_task,
        ARTNET_TASK_CORE
    );

    return ESP_OK;
}

void artnet_stop(void)
{
    if (s_socket >= 0) {
        close(s_socket);
        s_socket = -1;
    }
    if (s_recv_task) {
        vTaskDelete(s_recv_task);
        s_recv_task = NULL;
    }
}

void artnet_get_stats(artnet_stats_t *stats)
{
    if (stats) {
        memcpy(stats, &s_stats, sizeof(artnet_stats_t));
    }
}
