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
 * @file    com_app_cleartext.c
 * @brief   Support functions for HCP.
 */

#include <stddef.h>
#include <string.h>

#include "com_app_cleartext.h"
#include "fpc_hcp.h"

fpc_com_result_t com_app_clr_transmit(fpc_com_chain_t *chain)
{
    fpc_com_packet_tsp_t tsp_packet = { 0 };
    uint16_t tsp_offset;
    uint16_t link_offset;

    /* Get offsets */
    chain->link_overhead_get(&link_offset);
    chain->tsp_overhead_get(&tsp_offset);

    /* Create header */
    tsp_packet.data = chain->phy_mtu_buffer[FPC_COM_CHAIN_TX] + link_offset + tsp_offset;
    tsp_packet.size = chain->app_packet_size[FPC_COM_CHAIN_TX];
    tsp_packet.seq_len = chain->private_vars.hcp_seq_len;
    tsp_packet.seq_nr = chain->private_vars.hcp_seq_nr;

    return chain->tsp_tx(&tsp_packet, chain);
}

fpc_com_result_t com_app_clr_receive(fpc_com_chain_t *chain)
{
    fpc_com_result_t result;
    fpc_com_packet_tsp_t tsp_packet = { 0 };

    result = chain->tsp_rx(&tsp_packet, chain);

    chain->app_packet_size[FPC_COM_CHAIN_RX] = tsp_packet.size;

    return result;
}

uint16_t com_app_clr_get_overhead(uint16_t *offset)
{
    if (offset) {
        *offset = 0;
    }

    return 0;
}
