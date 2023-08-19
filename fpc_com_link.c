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
 * @file    fpc_com_link.c
 * @brief   Communication link layer implementation.
 */

#include <string.h>
#include <stdlib.h>

#include "fpc_com_link.h"

fpc_com_result_t fpc_com_link_transmit(fpc_com_packet_link_t *packet, fpc_com_chain_t *chain)
{
    uint32_t ack;
    uint16_t size;
    fpc_com_result_t result;

    if (packet == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

    /* Calculate CRC for payload */
    packet->crc = chain->crc_calc(0, packet->data, packet->size);

    /* Get total size to be transfered over PHY layer */
    size = packet->size + fpc_com_link_get_overhead(NULL);

    *((fpc_com_channel_t *)chain->phy_mtu_buffer[FPC_COM_CHAIN_TX]) = packet->channel;
    *((uint16_t *)(chain->phy_mtu_buffer[FPC_COM_CHAIN_TX] + sizeof(packet->channel)))
            = packet->size;
    /* Copy CRC to PHY mtu buffer */
    memcpy(packet->data + packet->size, &packet->crc, sizeof(packet->crc));
//    printf("fn::chain->phy_tx \n");
    /* Send Packet */
    result = chain->phy_tx(size, chain->phy_mtu_buffer[FPC_COM_CHAIN_TX], chain->phy_timeout_tx,
            chain->session);
   
    if (result != FPC_COM_RESULT_OK) {
        goto exit;
    }
    platform_delay_us();
    /* Receive ACK */
    result = chain->phy_rx(sizeof(ack), (uint8_t *)&ack, chain->phy_timeout_rx, chain->session);
    /* Handle no ack and timeout as IO error */
    if (ack != FPC_COM_ACK || result == FPC_COM_RESULT_TIMEOUT) {
        result = FPC_COM_RESULT_IO_ERROR;
        goto exit;
    }
    /* Handle rest as normal error */
    if (result != FPC_COM_RESULT_OK) {
        goto exit;
    }

exit:
    return result;
}

fpc_com_result_t fpc_com_link_receive(fpc_com_packet_link_t *packet, fpc_com_chain_t *chain)
{
    
//    printf("\n fpc_com_link_receive");
    bool status;
    uint32_t ack = FPC_COM_ACK;
    const uint8_t header_size = sizeof(packet->channel) + sizeof(packet->size);
    fpc_com_result_t result;

    if (packet == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

//    printf("checking header \n");
  
    platform_delay_us();
    /* Receive Header */
    result = chain->phy_rx(header_size, chain->phy_mtu_buffer[FPC_COM_CHAIN_RX],
            chain->phy_timeout_rx, chain->session);
    if (result != FPC_COM_RESULT_OK) {
        goto exit;
    }

    packet->channel = *((fpc_com_channel_t *)chain->phy_mtu_buffer[FPC_COM_CHAIN_RX]);
    packet->size = *((uint16_t *) (chain->phy_mtu_buffer[FPC_COM_CHAIN_RX]
            + sizeof(fpc_com_channel_t)));
    packet->data = chain->phy_mtu_buffer[FPC_COM_CHAIN_RX] + header_size;

    /* Check if packet size is valid */
    if (chain->phy_mtu_size[FPC_COM_CHAIN_RX]
            < (header_size + packet->size + sizeof(packet->crc))) {
//        printf("packet size is showing invalid\n");
        result = FPC_COM_RESULT_IO_ERROR;
        goto exit;
    }
//    printf("final result \n");
    platform_delay_us();
    /* Receive Payload */
    result = chain->phy_rx(packet->size + sizeof(packet->crc), packet->data,
            chain->phy_timeout_rx, chain->session);
    if (result != FPC_COM_RESULT_OK) {
//        printf("chain->phy_rx error result != FPC_COM_RESULT_OK \n");
        goto exit;
    }

    /* Check incoming packet CRC */
    memcpy(&packet->crc, packet->data + packet->size, sizeof(packet->crc));
    status = (packet->crc == chain->crc_calc(0, packet->data, packet->size));
    if (!status) {
//        printf("chain->phy_rx error CRC\n");
        result =  FPC_COM_RESULT_IO_ERROR;
        goto exit;
    }

    /* Send ACK */
    result = chain->phy_tx(sizeof(ack), (uint8_t *)&ack, chain->phy_timeout_tx, chain->session);

exit:
    return result;
}

uint16_t fpc_com_link_get_overhead(uint16_t *offset)
{
    fpc_com_packet_link_t *packet;
    static const uint16_t internal_offset = sizeof(packet->channel) + sizeof(packet->size);

    if (offset) {
        *offset = internal_offset;
    }
    return internal_offset + sizeof(packet->crc);
}
