/*
 * Art-Net Poll Reply
 *
 * Constructs and sends a proper ArtPollReply (239 bytes) so Art-Net
 * controllers can discover this node on the network.
 */

#include "artnet.h"
#include "artnet_types.h"
#include "config_store.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "artnet_reply";

void artnet_send_poll_reply(int sock, const struct sockaddr_in *requester)
{
    artnet_poll_reply_t reply;
    memset(&reply, 0, sizeof(reply));

    /* Header */
    memcpy(reply.id, ARTNET_HEADER, ARTNET_HEADER_LEN);
    reply.opcode = ARTNET_OP_POLL_REPLY;

    /* Get our IP */
    esp_netif_ip_info_t ip_info = {};
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_get_ip_info(netif, &ip_info);
    }
    memcpy(reply.ip, &ip_info.ip.addr, 4);
    reply.port = htons(ARTNET_PORT);

    /* Version */
    reply.ver_hi = FW_VERSION_MAJOR;
    reply.ver_lo = FW_VERSION_MINOR;

    /* Universe mapping */
    uint16_t start_uni = config_store_get_start_universe();
    reply.net_switch = (start_uni >> 8) & 0x7F;
    reply.sub_switch = (start_uni >> 4) & 0x0F;

    /* Names */
    const char *name = config_store_get_node_name();
    strncpy(reply.short_name, name, sizeof(reply.short_name) - 1);
    snprintf(reply.long_name, sizeof(reply.long_name) - 1,
             "ArtNet HUB75 %ux%u ESP32 [v%u.%u]",
             config_store_get_panel_width(), config_store_get_panel_height(),
             FW_VERSION_MAJOR, FW_VERSION_MINOR);

    /* Node report */
    artnet_stats_t stats;
    artnet_get_stats(&stats);
    snprintf(reply.node_report, sizeof(reply.node_report) - 1,
             "#0001 [%04u] OK - %u fps, %u complete, %u partial",
             (unsigned)(stats.frames_complete & 0xFFFF),
             stats.fps, stats.frames_complete, stats.frames_partial);

    /* Port configuration */
    reply.num_ports_lo = 1;
    reply.port_types[0] = 0x80;  /* Output, DMX512 */
    reply.good_output[0] = 0x80; /* Data being transmitted */
    reply.sw_out[0] = start_uni & 0x0F;

    /* MAC address */
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    memcpy(reply.mac, mac, 6);

    /* Bind info */
    memcpy(reply.bind_ip, reply.ip, 4);
    reply.bind_index = 1;
    reply.status2 = 0x08;  /* Supports web browser configuration */
    reply.style = 0x00;    /* StNode */

    /* Send to requester */
    int ret = sendto(sock, &reply, sizeof(reply), 0,
                     (const struct sockaddr *)requester, sizeof(*requester));
    if (ret < 0) {
        ESP_LOGW(TAG, "Failed to send PollReply to requester: errno %d", errno);
    }

    /* Also broadcast (per Art-Net spec) */
    struct sockaddr_in broadcast = {
        .sin_family = AF_INET,
        .sin_port = htons(ARTNET_PORT),
        .sin_addr.s_addr = htonl(INADDR_BROADCAST),
    };
    sendto(sock, &reply, sizeof(reply), 0,
           (const struct sockaddr *)&broadcast, sizeof(broadcast));

    ESP_LOGD(TAG, "PollReply sent");
}
