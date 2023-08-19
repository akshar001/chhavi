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
 * @file    fpc_com_transport.c
 * @brief   Communication transport layer implementation.
 */

#include <stdlib.h>
#include <string.h>

#include "fpc_com_link.h"
#include "fpc_com_transport.h"

fpc_com_result_t fpc_com_transport_transmit(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain)
{
//    printf("fn::fpc_com_transport_transmit\n");
    fpc_com_packet_link_t link_packet = { 0 };
    fpc_com_result_t result;
    uint16_t link_offset;
    uint16_t offset;

    if (packet == NULL|| chain == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

    /* Construct header */
    chain->link_overhead_get(&link_offset);
    link_packet.data = chain->phy_mtu_buffer[FPC_COM_CHAIN_TX] + link_offset;

    *((uint16_t *)(link_packet.data)) = packet->size;
    offset = sizeof(packet->size);
    *((uint16_t *)(link_packet.data + offset)) = packet->seq_nr;
    offset += sizeof(packet->seq_nr);
    *((uint16_t *)(link_packet.data + offset)) = packet->seq_len;

    link_packet.channel = chain->channel;
    link_packet.size = packet->size + chain->tsp_overhead_get(NULL);

    /* Send packet */
    result = fpc_com_link_transmit(&link_packet, chain);

exit:
    return result;
}

fpc_com_result_t fpc_com_transport_receive(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain)
{
    fpc_com_packet_link_t link_packet = { 0 };
    fpc_com_result_t result;
    uint16_t offset;

    if (packet == NULL|| chain == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

    result = fpc_com_link_receive(&link_packet, chain);
    if (result != FPC_COM_RESULT_OK) {
//        printf("fpc_com_transport_receive result != FPC_COM_RESULT_OK \n");
        goto exit;
    }

    packet->size = *((uint16_t *)link_packet.data);
    offset = sizeof(packet->size);
    packet->seq_nr = *((uint16_t *)(link_packet.data + offset));
    offset += sizeof(packet->seq_nr);
    packet->seq_len = *((uint16_t *)(link_packet.data + offset));
    offset += sizeof(packet->seq_len);
    packet->data = link_packet.data + offset;

exit:
    return result;
}

uint16_t fpc_com_transport_get_overhead(uint16_t *offset)
{
    fpc_com_packet_tsp_t *packet;
    static const uint16_t internal_offset = sizeof(packet->size) + sizeof(packet->seq_len)
            + sizeof(packet->seq_nr);

    if (offset) {
        *offset = internal_offset;
    }

    return internal_offset;
}
