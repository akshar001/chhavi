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

#ifndef COM_COMMON_H
#define COM_COMMON_H

/**
 * @file    com_common.h
 * @brief   Communication common interface.
 */

#include <stdint.h>
#include <stdbool.h>

#include "fpc_bep_types.h"
#include "fpc_hcp_common.h"
#include "fpc_com_chain.h"

/**
 * @brief Allocates or assigns memory for arguments during HCP packet reassembly.

 * @param[in] cmd HCP command.
 * @param[in] arg HCP argument.
 * @param[in] size size of allocation.
 * @param[out] free_data Should data be freed by caller.
 * @param[in] context User defined context pointer.
 *
 * @return Pointer to allocation or NULL if allocation failed.
 */
void *argument_allocator(fpc_hcp_cmd_t cmd, fpc_hcp_arg_t arg, uint16_t size, bool *free_data,
        void *context);

/**
 * @brief Frees memory for arguments during HCP packet destruction.
 *
 * @param[in] cmd HCP Command.
 * @param[in] arg_data Argument data.
 * @param[in] context User defined context pointer.
 */
void argument_free(fpc_hcp_cmd_t cmd, fpc_hcp_arg_data_t *arg_data, void *context);

/**
 * @brief Prints packet information to the log.
 *
 * @param[in] packet Packet to print.
 * @param[in] msg Optional message to print. Set NULL if not needed.
 */
void print_packet(fpc_hcp_packet_t *packet, const char *msg);

/**
 * @brief Initialize the chain used during communication with the host.
 *
 * @param[in, out] chain The chain structure to populate.
 * @param[in] buffer The buffer to use during communication.
 * @param[in] size An array of two sizes, one for the TX buffer size and one for the RX.
 * @param[in] context Application context.
 */
void init_com_chain(fpc_com_chain_t *chain, uint8_t *buffer, uint16_t size[2], void *context);

/**
 * @brief Convert a COM result from HCP to a BEP result.
 *
 * @param[in] result COM result to convert.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t com_to_bep_result(fpc_com_result_t result);

/**
 * @brief Allocate and add data to an argument.
 *
 * @param[in, out] cmd Command to add argument to.
 * @param[in] arg Argument to add.
 * @param[in] data Data to copy to allocated memory.
 * @param[in] size Size of data.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t arg_add(fpc_hcp_packet_t *cmd, fpc_hcp_arg_t arg, const void *data, uint16_t size);

#endif /* COM_COMMON_H */
