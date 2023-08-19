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
 * @file    fpc_com_chain.h
 * @brief   Communication chain type definitions.
 */

#ifndef FPC_COM_CHAIN_H
#define FPC_COM_CHAIN_H

#include <stdint.h>

#include "fpc_com_result.h"
#include "fpc_hcp_common.h"
#include "fpc_com_packets.h"

/**
 * @brief Communication chain private variables.
 */
typedef struct fpc_com_chain_private fpc_com_chain_private_t;
/** Communication chain private struct */
struct fpc_com_chain_private {
    /** HCP packet */
    fpc_hcp_packet_t *hcp_packet;
    /** HCP sequence length */
    uint16_t hcp_seq_len;
    /** HCP sequence number */
    uint16_t hcp_seq_nr;
};

/**
 * @brief Communication chain.
 */
typedef struct fpc_com_chain fpc_com_chain_t;
/** Communication chain struct */
struct fpc_com_chain {
    /** Initialization status */
    bool initialized;

    /**
     * @name HCP Layer
     * @{
     */
    /** Argument allocator interface function */
    void *(*argument_allocator)(fpc_hcp_cmd_t cmd, fpc_hcp_arg_t arg, uint16_t size,
            bool *free_data, void *context);
    /** Argument free interface function */
    void (*argument_free)(fpc_hcp_cmd_t cmd, fpc_hcp_arg_data_t *arg_data, void *context);
    /** @} */

    /** CRC calculation interface function */
    uint32_t (*crc_calc)(uint32_t start, const void *data, uint32_t size);

    /**
     * @name Application Layer
     * @{
     */
    /** Application layer transmit interface function */
    fpc_com_result_t (*app_tx)(fpc_com_chain_t *chain);
    /** Application layer receive interface function */
    fpc_com_result_t (*app_rx)(fpc_com_chain_t *chain);
    /** Application layer overhead get interface function */
    uint16_t (*app_overhead_get)(uint16_t *offset);
    /** Application packet sizes */
    uint16_t app_packet_size[2];
    /** Application MTU sizes */
    uint16_t app_mtu_size[2];
    /** Application MTU buffers */
    uint8_t *app_mtu_buffer[2];
    /** @} */

    /**
     * @name Transport Layer
     * @{
     */
    /** Transport layer transmit interface function */
    fpc_com_result_t (*tsp_tx)(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain);
    /** Transport layer receive interface function */
    fpc_com_result_t (*tsp_rx)(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain);
    /** Transport layer overhead get interface function */
    uint16_t (*tsp_overhead_get)(uint16_t *offset);
    /** @} */

    /**
     * @name Link Layer
     * @{
     */
    /** Link layer overhead get interface function */
    uint16_t (*link_overhead_get)(uint16_t *offset);
    /** Communication channel */
    fpc_com_channel_t channel;
    /** @} */

    /**
     * @name Physical Layer
     * @{
     */
    /** Physical layer transmit interface function */
    fpc_com_result_t (*phy_tx)(uint16_t size, const uint8_t *buffer, uint32_t timeout,
            void *session);
    /** Physical layer receive interface function */
    fpc_com_result_t (*phy_rx)(uint16_t size, uint8_t *buffer, uint32_t timeout,
            void *session);
    /** Physical MTU sizes */
    uint16_t phy_mtu_size[2];
    /** Physical MTU buffers */
    uint8_t *phy_mtu_buffer[2];
    /** Physical transmit timeout */
    uint32_t phy_timeout_tx;
    /** Physical receive timeout */
    uint32_t phy_timeout_rx;
    /** @} */

    /** Communication change private variables */
    fpc_com_chain_private_t private_vars;

    /**
     * @brief User session pointer.
     * User private stuff, to be able to pass necessary info from the layer that
     * calls hcp down to the user's TX and RX functions (phy_tx/rx), to enable
     * multi threaded applications at the host side.
     */
    void *session;
    /**
     * @brief User context pointer.
     * User private stuff, to be able to pass nessecary context to argument_allocator and
     * argument_free.
     */
    void *context;
};

/**
 * @brief Communication chain direction type.
 */
typedef enum {
    FPC_COM_CHAIN_TX = 0,
    FPC_COM_CHAIN_RX = 1,
} fpc_com_chain_dir_t;

#endif /* FPC_COM_CHAIN_H */
