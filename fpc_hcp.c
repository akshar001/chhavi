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
 * @file    fpc_hcp.c
 * @brief   Host Communication Protocol implementation.
 */

#include <stdlib.h>
#include <string.h>

#include "fpc_hcp.h"
//#include "platform.h"

/**
 * @name HCP Packet Member Sizes
 *
 * Macros for packet member sizes.
 * @{
 */
#define PACKET_ID_SIZE sizeof(((fpc_hcp_packet_t*)0)->id)
#define PACKET_NUM_ARGS_SIZE sizeof(((fpc_hcp_packet_t*)0)->num_args)
#define PACKET_HEADER_SIZE (PACKET_ID_SIZE + PACKET_NUM_ARGS_SIZE)
/** @} */

/**
 * @name HCP Argument Member Sizes
 *
 * Macros for argument member sizes.
 * @{
 */
#define ARGUMENT_ARG_SIZE sizeof(((fpc_hcp_arg_data_t*)0)->arg)
#define ARGUMENT_SIZE_SIZE sizeof(((fpc_hcp_arg_data_t*)0)->size)
#define ARGUMENT_HEADER_SIZE (ARGUMENT_ARG_SIZE + ARGUMENT_SIZE_SIZE)
/** @} */

/**
 * @brief Handle receive chunks.
 *
 * @param chain Comminucation chain.
 * return ::fpc_com_result_t
 */
static fpc_com_result_t recieve_chunks(fpc_com_chain_t *chain);
/**
 * @brief Handle transmit chunks.
 *
 * @param chain Comminucation chain.
 * return ::fpc_com_result_t
 */
static fpc_com_result_t transmit_chunks(fpc_com_chain_t *chain);

uint16_t fpc_hcp_get_size(fpc_hcp_packet_t *packet, uint16_t *num_args)
{
    uint16_t size = 0;
    uint16_t args = 0;

    if (packet == NULL) {
        goto exit;
    }
    size = PACKET_HEADER_SIZE;

    for (uint8_t i = 0; i < packet->num_args; i++) {
        if (packet->arguments[i].arg != ARG_NONE) {
            size += ARGUMENT_HEADER_SIZE;
            size += packet->arguments[i].size;
            args++;
        }
    }
    if (num_args) {
        *num_args = args;
    }

exit:
    return size;
}

fpc_com_result_t fpc_hcp_transmit(fpc_hcp_packet_t *packet, fpc_com_chain_t *chain)
{
    fpc_com_result_t result;

    if (chain == NULL || packet == NULL || chain->initialized == false ||
            chain->phy_mtu_buffer[FPC_COM_CHAIN_TX] == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }
    chain->private_vars.hcp_packet = packet;

    if (chain->app_mtu_size[FPC_COM_CHAIN_TX] == 0 ||
            chain->app_mtu_buffer[FPC_COM_CHAIN_TX] == NULL) {
        uint16_t tsp_offset;
        uint16_t link_offset;
        uint16_t overhead;

        overhead = chain->tsp_overhead_get(&tsp_offset) + chain->link_overhead_get(&link_offset);
        chain->app_mtu_buffer[FPC_COM_CHAIN_TX] = chain->phy_mtu_buffer[FPC_COM_CHAIN_TX]
                + tsp_offset + link_offset;
        chain->app_mtu_size[FPC_COM_CHAIN_TX] = chain->phy_mtu_size[FPC_COM_CHAIN_TX] - overhead;
    }
    result = transmit_chunks(chain);
//    printf("fn:fpc_hcp_transmit \n");

exit:
    return result;
}

fpc_com_result_t fpc_hcp_receive(fpc_hcp_packet_t *packet, fpc_com_chain_t *chain)
{
    fpc_com_result_t result;

//    printf("fn:: fpc_hcp_receive\n");
    if (chain == NULL || packet == NULL || chain->initialized == false ||
            chain->phy_mtu_buffer[FPC_COM_CHAIN_RX] == NULL) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }
    chain->private_vars.hcp_packet = packet;

    if (chain->app_mtu_size[FPC_COM_CHAIN_RX] == 0 ||
            chain->app_mtu_buffer[FPC_COM_CHAIN_RX] == NULL) {
        uint16_t tsp_offset;
        uint16_t link_offset;
        uint16_t overhead;

        overhead = chain->tsp_overhead_get(&tsp_offset) + chain->link_overhead_get(&link_offset);
        chain->app_mtu_buffer[FPC_COM_CHAIN_RX] = chain->phy_mtu_buffer[FPC_COM_CHAIN_RX]
                + tsp_offset + link_offset;
        chain->app_mtu_size[FPC_COM_CHAIN_RX] = chain->phy_mtu_size[FPC_COM_CHAIN_RX] - overhead;
    }
    result = recieve_chunks(chain);

exit:
    return result;
}

bool fpc_hcp_arg_add(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg, uint16_t size, bool free_data,
        void *data)
{
    bool result = false;

    if (packet == NULL || (size > 0 && data == NULL)) {
        goto exit;
    }

    for (uint8_t i = 0; i < packet->num_args; i++) {
        //
        if (packet->arguments[i].arg == ARG_NONE) {
            packet->arguments[i].arg = arg;
            packet->arguments[i].size = size;
            packet->arguments[i].free_data = free_data;
            packet->arguments[i].data = data;
            result = true;
            break;
        }
    }

exit:
    return result;
}

bool fpc_hcp_arg_check(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg) {
    return fpc_hcp_arg_get(packet, arg) != NULL;
}

fpc_hcp_arg_data_t *fpc_hcp_arg_get(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg)
{
    for (uint8_t i = 0; i < packet->num_args; i++) {
//        printf("%4x %4x\n",packet->arguments[i].arg,arg);
        if (packet->arguments[i].arg == arg) {
            return &(packet->arguments[i]);
        }
    }
    return NULL;
}

bool fpc_hcp_arg_copy_data(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg, uint16_t data_size,
        uint8_t *data)
{
    fpc_hcp_arg_data_t *arg_data = fpc_hcp_arg_get(packet, arg);

    if (arg_data == NULL || data == NULL || data_size == 0 || arg_data->size == 0
            || data_size > arg_data->size) {
        return false;
    }

    memcpy(data, arg_data->data, data_size);

    return true;
}

void fpc_hcp_free(fpc_com_chain_t *chain, fpc_hcp_packet_t *packet)
{
    if (chain && packet) {
        for (uint8_t i = 0; i < packet->num_args; i++) {
            if (packet->arguments[i].arg != ARG_NONE) {
                chain->argument_free(packet->id, &packet->arguments[i], chain->context);
            }
        }
        memset(packet->arguments, 0x0, sizeof(*packet->arguments) * packet->num_args);
        packet->id = CMD_NONE;
    }
}

static fpc_com_result_t recieve_chunks(fpc_com_chain_t *chain)
{
    uint8_t *data;
    uint16_t rem_size;
    uint16_t id_rem_size;
    uint16_t size_rem_size;
    uint16_t arg_rem_size = 0;
    uint16_t arg_size;
    uint8_t *arg_data = NULL;
    uint16_t overhead;
    uint16_t arg_nr = 0;
    uint16_t num_args = 0;
    fpc_com_result_t result;
    bool first_packet = true;
    fpc_hcp_arg_t arg;
    uint16_t app_offset;

    /* Calculate application packet data MTU size */
    overhead = chain->app_overhead_get(&app_offset);
    if (overhead > chain->app_mtu_size[FPC_COM_CHAIN_RX]) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

    /* Retrieve application packets */
    do {
        /* Get packet from application layer */
        result = chain->app_rx(chain);
        if (result != FPC_COM_RESULT_OK) {
//            printf("chain->app_rx  result != FPC_COM_RESULT_OK\n ");
            break;
        }

        /* Get packet size */
        rem_size = chain->app_packet_size[FPC_COM_CHAIN_RX];
        data = chain->app_mtu_buffer[FPC_COM_CHAIN_RX] + app_offset;

        if (first_packet) {
            /* ID */
            chain->private_vars.hcp_packet->id = *((uint16_t *) data);
            data += PACKET_ID_SIZE;

            /* Number of arguments */
            num_args = *((uint16_t *) data);
            data += PACKET_NUM_ARGS_SIZE;

            first_packet = false;
            rem_size -= PACKET_HEADER_SIZE;
            id_rem_size = ARGUMENT_ARG_SIZE;
            size_rem_size = ARGUMENT_SIZE_SIZE;
        }

        while (rem_size && (arg_nr < num_args)) {
            uint16_t copy_size;

            if (rem_size && id_rem_size) {
                /* Set copy size */
                copy_size = HCP_MIN(id_rem_size, rem_size);

                /* Copy arg id */
                memcpy(((uint8_t *) &arg) + (ARGUMENT_ARG_SIZE - id_rem_size), data, copy_size);
                data += copy_size;

                /* Update parameters */
                id_rem_size -= copy_size;
                rem_size -= copy_size;
            }

            if (rem_size && size_rem_size) {
                bool status;

                /* Set copy size */
                copy_size = HCP_MIN(size_rem_size, rem_size);

                /* Copy arg size */
                memcpy(((uint8_t *) &arg_size) + (ARGUMENT_SIZE_SIZE - size_rem_size),
                        data, copy_size);
                data += copy_size;

                /* Update parameters */
                size_rem_size -= copy_size;
                rem_size -= copy_size;

                if (size_rem_size == 0) {
                    bool free_data = true;

                    if (arg_size) {
                        arg_data = chain->argument_allocator(chain->private_vars.hcp_packet->id,
                                arg, arg_size, &free_data, chain->context);
                        if (arg_data == NULL) {
                            result = FPC_COM_RESULT_NO_MEMORY;
                            break;
                        }
                    }

                    status = fpc_hcp_arg_add(chain->private_vars.hcp_packet, arg, arg_size,
                            free_data, arg_data);
                    if (status == false) {
                        fpc_hcp_arg_data_t arg_struct = { 0 };

                        arg_struct.arg = arg;
                        arg_struct.free_data = free_data;
                        arg_struct.size = arg_size;
                        arg_struct.data = arg_data;
                        chain->argument_free(chain->private_vars.hcp_packet->id, &arg_struct,
                                chain->context);

                        if (arg_nr < num_args) {
                            result = FPC_COM_RESULT_INVALID_ARGUMENT;
                            break;
                        } else {
                            result = FPC_COM_RESULT_NO_MEMORY;
                            break;
                        }
                    }
                    arg_rem_size = arg_size;
                }
            }

            if (rem_size && arg_rem_size) {
                /* Set copy size */
                copy_size = HCP_MIN(arg_rem_size, rem_size);

                /* Copy argument data */
                memcpy(arg_data + (arg_size - arg_rem_size), data, copy_size);
                data += copy_size;

                /* Update parameters */
                rem_size -= copy_size;
                arg_rem_size -= copy_size;
            }

            /* If data has been copied set new arg to true */
            if (id_rem_size == 0 && size_rem_size == 0 && arg_rem_size == 0) {
                id_rem_size = ARGUMENT_ARG_SIZE;
                size_rem_size = ARGUMENT_SIZE_SIZE;
                arg_nr++;
            }
        }
    } while (result == FPC_COM_RESULT_OK && arg_nr < num_args);

exit:
    return result;
}

static fpc_com_result_t transmit_chunks(fpc_com_chain_t *chain)
{
    uint16_t serialized_size;
    uint16_t packet_data_left;
    uint16_t data_rem_size = 0;
    uint16_t app_mtu;
    uint16_t overhead;
    fpc_com_result_t result;
    uint16_t num_args = 0;
    uint16_t arg_nr = 0;
    uint16_t app_offset;
    uint8_t *data;
    fpc_hcp_arg_data_t *arg_data = NULL;


    /* Calculate transport packet data MTU size */
    overhead = chain->app_overhead_get(&app_offset);
    if (overhead > chain->phy_mtu_size[FPC_COM_CHAIN_TX]) {
        result = FPC_COM_RESULT_INVALID_ARGUMENT;
        goto exit;
    }
    app_mtu = chain->app_mtu_size[FPC_COM_CHAIN_TX] - overhead;

    /* Get application packet total serialized size */
    serialized_size = fpc_hcp_get_size(chain->private_vars.hcp_packet, &num_args);

    /* Set initial sequence length and size */
    /* Calculate sequence length (number of application packages) */
    chain->private_vars.hcp_seq_len = (serialized_size + app_mtu + 1) / app_mtu;
    chain->private_vars.hcp_seq_nr = 1;

    /* Set first packet size */
    chain->app_packet_size[FPC_COM_CHAIN_TX] = HCP_MIN(serialized_size, app_mtu);
    packet_data_left = chain->app_packet_size[FPC_COM_CHAIN_TX];

    /* Point packet data to application buffer */
    data = chain->app_mtu_buffer[FPC_COM_CHAIN_TX] + app_offset;

    /* Copy command ID to buffer */
    *((fpc_hcp_cmd_t *) data) = chain->private_vars.hcp_packet->id;
    data += PACKET_ID_SIZE;

    /* Copy number of arguments to buffer */
    *((uint16_t *) data) = num_args;
    data += PACKET_NUM_ARGS_SIZE;

    packet_data_left -= PACKET_HEADER_SIZE;

    do {
        /* Copy arguments to data blob */
        while (packet_data_left && arg_nr < num_args) {
            /* Copy arg header to buffer */
            if (!data_rem_size && packet_data_left >= ARGUMENT_HEADER_SIZE) {
                /* Get current argument */
                arg_data = &chain->private_vars.hcp_packet->arguments[arg_nr];

                /* Copy argument id */
                memcpy(data, &arg_data->arg, ARGUMENT_ARG_SIZE);
                data += ARGUMENT_ARG_SIZE;

                /* Copy argument size */
                memcpy(data, &arg_data->size, ARGUMENT_SIZE_SIZE);
                data += ARGUMENT_SIZE_SIZE;

                /* Update parameters */
                packet_data_left -= ARGUMENT_HEADER_SIZE;
                data_rem_size = arg_data->size;
            } else if (!data_rem_size && packet_data_left < ARGUMENT_HEADER_SIZE) {
                /*
                 * Special case to ensure that the argument header
                 * is not split over several packets as that will break the
                 * argument allocator.
                 */
                chain->app_packet_size[FPC_COM_CHAIN_TX] -= packet_data_left;
                break;
            }

            /* Copy argument data to transport package data blob */
            if (packet_data_left && data_rem_size) {
                uint16_t copy_size;

                /* Set copy size */
                copy_size = HCP_MIN(data_rem_size, packet_data_left);

                /* Copy argument data */
                memcpy(data, arg_data->data + (arg_data->size - data_rem_size), copy_size);
                data += copy_size;

                /* Update parameters */
                data_rem_size -= copy_size;
                packet_data_left -= copy_size;
            }
            if (!data_rem_size) {
                arg_nr++;
            }
        }
//        printf("fn:transmit_chunks \n");
        /* Transmit transport package */
        result = chain->app_tx(chain);

        chain->private_vars.hcp_seq_nr++;

        /* Reduce data left counter */
        serialized_size -= chain->app_packet_size[FPC_COM_CHAIN_TX];

        /* Set next packet size */
        packet_data_left = HCP_MIN(serialized_size, app_mtu);
        chain->app_packet_size[FPC_COM_CHAIN_TX] = packet_data_left;

        /* Set destination offset to zero for next packet */
        data = chain->app_mtu_buffer[FPC_COM_CHAIN_TX] + app_offset;
    } while (result == FPC_COM_RESULT_OK && serialized_size);

exit:
    return result;
}
