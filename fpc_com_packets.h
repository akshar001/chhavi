/*
 * Copyright (c) 2020 Fingerprint Cards AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file    fpc_com_packets.h
 * @brief   Communication packet type definitions.
 */

#ifndef FPC_COM_PACKETS_H
#define FPC_COM_PACKETS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Communication acknowledge definition */
#define FPC_COM_ACK 0x7f01ff7f

/**
 * Transport layer packet.
 */
typedef struct fpc_com_packet_transport {
    /** Size of packet */
    uint16_t size;
    /** Sequence length */
    uint16_t seq_len;
    /** Sequence number */
    uint16_t seq_nr;
    /** Packet data */
    uint8_t *data;
} fpc_com_packet_tsp_t;

/**
 * Transport packet channels.
 */
enum fpc_com_channel {
    FPC_COM_CHANNEL_NONE    = 0x00,
    FPC_COM_CHANNEL_CLEAR   = 0x01,
    FPC_COM_CHANNEL_TLS     = 0x02,
    FPC_COM_CHANNEL_END     = 0xFF,
};
/** Communication channel type */
typedef uint16_t fpc_com_channel_t;

/** Link layer packet */
typedef struct fpc_com_packet_link {
    /** Communication channel */
    fpc_com_channel_t channel;
    /** Size of packet */
    uint16_t size;
    /** Packet data */
    uint8_t *data;
    /** CRC of data */
    uint32_t crc;
} fpc_com_packet_link_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_COM_PACKETS_H */
