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
 * @file    com_common.c
 * @brief   Communication common interface.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fpc_com_transport.h"
#include "fpc_com_link.h"
#include "fpc_hcp.h"
#include "fpc_crc.h"

#include "platform.h"
#include "com_app_cleartext.h"
#include "com_common.h"

#ifdef _WIN32
#define __attribute__(x)
#endif

#define UNUSED(x) (void)(x)

__attribute__((weak)) void *argument_allocator(fpc_hcp_cmd_t cmd, fpc_hcp_arg_t arg, uint16_t size,
        bool *free_data, void *context)
{
    void *pointer = NULL;

    UNUSED(cmd);
    UNUSED(arg);
    UNUSED(context);

    /* Default behavior */
    pointer = malloc(size);

    if (free_data != NULL) {
        *free_data = true;
    }

    return pointer;
}

__attribute__((weak)) void argument_free(fpc_hcp_cmd_t cmd, fpc_hcp_arg_data_t *arg_data,
        void *context)
{
    UNUSED(cmd);
    UNUSED(context);

    if (arg_data->free_data) {
        free(arg_data->data);
        arg_data->data = NULL;
    }
}

__attribute__((weak)) void print_packet(fpc_hcp_packet_t *packet, const char* msg)
{
#if (FPC_LOG_INCLUDE == 1)
    if (!packet) {
        fpc_log_app_var(FPC_LOG_LEVEL_INFO, "Invalid argument");
        return;
    }

    fpc_log_app_var(FPC_LOG_LEVEL_DEBUG, "%sPacket cmd id=0x%04X", msg != NULL ? msg : "",
                    packet->id);

    for (int i = 0; i < packet->num_args; ++i) {
        if (packet->arguments[i].arg != ARG_NONE) {
            fpc_log_app_var(FPC_LOG_LEVEL_DEBUG, "\tArg[%02d]: key=0x%04X, size=%d free=%d", i,
                            packet->arguments[i].arg, packet->arguments[i].size,
                            packet->arguments[i].free_data);
        }
    }
#else
    UNUSED(packet);
    UNUSED(msg);
#endif
}

void init_com_chain(fpc_com_chain_t *chain, uint8_t *buffer, uint16_t size[2],
        void *context)
{
    chain->initialized = true;

    /* CRC */
    chain->crc_calc = fpc_crc;

    /* HCP */
    chain->app_tx = com_app_clr_transmit;
    chain->app_rx = com_app_clr_receive;
    chain->app_overhead_get = com_app_clr_get_overhead;
    chain->argument_allocator = argument_allocator;
    chain->argument_free = argument_free;
    chain->app_packet_size[FPC_COM_CHAIN_TX] = 0;
    chain->app_packet_size[FPC_COM_CHAIN_RX] = 0;
    chain->app_mtu_buffer[FPC_COM_CHAIN_TX] = NULL;
    chain->app_mtu_buffer[FPC_COM_CHAIN_RX] = NULL;
    chain->app_mtu_size[FPC_COM_CHAIN_TX] = 0;
    chain->app_mtu_size[FPC_COM_CHAIN_RX] = 0;

    /* Transport */
    chain->tsp_tx = fpc_com_transport_transmit;
    chain->tsp_rx = fpc_com_transport_receive;
    chain->tsp_overhead_get = fpc_com_transport_get_overhead;

    /* Link */
    chain->link_overhead_get = fpc_com_link_get_overhead;

    /* Phy */
    chain->phy_tx = platform_com_send;
    chain->phy_rx = platform_com_receive;
    chain->phy_mtu_size[FPC_COM_CHAIN_TX] = size[FPC_COM_CHAIN_TX];
    chain->phy_mtu_size[FPC_COM_CHAIN_RX] = size[FPC_COM_CHAIN_RX];
    chain->phy_mtu_buffer[FPC_COM_CHAIN_TX] = buffer;
    chain->phy_mtu_buffer[FPC_COM_CHAIN_RX] = buffer + size[FPC_COM_CHAIN_TX];
    chain->phy_timeout_tx = 2000;
    chain->phy_timeout_rx = 2000;

    chain->session = NULL;
    chain->context = context;
}

fpc_bep_result_t com_to_bep_result(fpc_com_result_t result)
{
    fpc_bep_result_t bep_result;
    switch (result) {
        case FPC_COM_RESULT_OK :
            bep_result = FPC_BEP_RESULT_OK;
            break;
        case FPC_COM_RESULT_NO_MEMORY:
            bep_result = FPC_BEP_RESULT_NO_MEMORY;
            break;
        case FPC_COM_RESULT_INVALID_ARGUMENT:
            bep_result = FPC_BEP_RESULT_INVALID_ARGUMENT;
            break;
        case FPC_COM_RESULT_NOT_IMPLEMENTED:
            bep_result = FPC_BEP_RESULT_NOT_IMPLEMENTED;
            break;
        case FPC_COM_RESULT_IO_ERROR:
            bep_result = FPC_BEP_RESULT_IO_ERROR;
            break;
        case FPC_COM_RESULT_TIMEOUT:
            bep_result = FPC_BEP_RESULT_TIMEOUT;
            break;
        default:
            bep_result = FPC_BEP_RESULT_GENERAL_ERROR;
            break;
    }
    return bep_result;
}

fpc_bep_result_t arg_add(fpc_hcp_packet_t *cmd, fpc_hcp_arg_t arg, const void *data, uint16_t size)
{
    fpc_bep_result_t result = FPC_BEP_RESULT_OK;
    uint32_t *pointer;

    if (cmd == NULL || data == NULL || size == 0) {
        result = FPC_BEP_RESULT_INVALID_ARGUMENT;
        goto exit;
    }

    pointer = malloc(size);
    if (pointer == NULL) {
        result = FPC_BEP_RESULT_NO_MEMORY;
        goto exit;
    }

    memcpy(pointer, data, size);
    if (fpc_hcp_arg_add(cmd, arg, size, true, pointer) == false) {
        free(pointer);
        result = FPC_BEP_RESULT_NO_RESOURCE;
    }

exit:
    return result;
}
