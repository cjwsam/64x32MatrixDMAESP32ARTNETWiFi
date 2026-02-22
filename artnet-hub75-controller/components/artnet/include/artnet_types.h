#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Art-Net protocol constants */
#define ARTNET_HEADER       "Art-Net"
#define ARTNET_HEADER_LEN   8
#define ARTNET_MAX_DMX_LEN  512
#define ARTNET_DMX_START    18
#define ARTNET_MAX_PACKET   (ARTNET_DMX_START + ARTNET_MAX_DMX_LEN)  /* 530 */

/* Opcodes (little-endian on wire) */
#define ARTNET_OP_POLL       0x2000
#define ARTNET_OP_POLL_REPLY 0x2100
#define ARTNET_OP_DMX        0x5000
#define ARTNET_OP_SYNC       0x5200

/* Art-Net protocol version */
#define ARTNET_PROTO_VER     14

/* ArtDmx packet header (packed, matches wire format) */
typedef struct __attribute__((packed)) {
    char     id[8];          /* "Art-Net\0" */
    uint16_t opcode;         /* little-endian */
    uint8_t  proto_ver_hi;   /* 0 */
    uint8_t  proto_ver_lo;   /* 14 */
    uint8_t  sequence;       /* 0-255 */
    uint8_t  physical;       /* input port */
    uint8_t  sub_uni;        /* universe low byte */
    uint8_t  net;            /* universe high byte */
    uint8_t  length_hi;      /* data length big-endian high */
    uint8_t  length_lo;      /* data length big-endian low */
    uint8_t  data[];         /* 2-512 bytes DMX data */
} artnet_dmx_packet_t;

/* ArtPollReply packet (239 bytes, per Art-Net 4 spec) */
typedef struct __attribute__((packed)) {
    char     id[8];          /* "Art-Net\0" */
    uint16_t opcode;         /* 0x2100 */
    uint8_t  ip[4];
    uint16_t port;           /* 0x1936 (6454) */
    uint8_t  ver_hi;
    uint8_t  ver_lo;
    uint8_t  net_switch;
    uint8_t  sub_switch;
    uint16_t oem;
    uint8_t  ubea_ver;
    uint8_t  status1;
    uint16_t esta_man;
    char     short_name[18];
    char     long_name[64];
    char     node_report[64];
    uint8_t  num_ports_hi;
    uint8_t  num_ports_lo;
    uint8_t  port_types[4];
    uint8_t  good_input[4];
    uint8_t  good_output[4];
    uint8_t  sw_in[4];
    uint8_t  sw_out[4];
    uint8_t  sw_video;
    uint8_t  sw_macro;
    uint8_t  sw_remote;
    uint8_t  spare[3];
    uint8_t  style;          /* 0x00 = StNode */
    uint8_t  mac[6];
    uint8_t  bind_ip[4];
    uint8_t  bind_index;
    uint8_t  status2;
    uint8_t  filler[26];
} artnet_poll_reply_t;

/* Frame assembly state */
typedef struct {
    uint8_t  *back_buffer;           /* points to display back buffer */
    uint16_t  universe_mask;         /* bitmask of received universes */
    uint16_t  universe_target;       /* all bits set for expected universes */
    int64_t   frame_start_us;        /* timestamp of first universe in frame */
    uint8_t   last_sequence;
    uint32_t  frames_complete;
    uint32_t  frames_partial;
    uint32_t  frames_dropped;
    uint32_t  total_packets;
} artnet_frame_state_t;

/* Runtime stats */
typedef struct {
    uint32_t fps;
    uint32_t frames_complete;
    uint32_t frames_partial;
    uint32_t packets_received;
} artnet_stats_t;

#ifdef __cplusplus
}
#endif
