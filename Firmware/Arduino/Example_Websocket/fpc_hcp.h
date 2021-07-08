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
 * @file    fpc_hcp.h
 * @brief   Host Communication Protocol interface.
 */

#ifndef FPC_HCP_H
#define FPC_HCP_H

#include <stdbool.h>
#include <stdint.h>

#include "fpc_hcp_common.h"
#include "fpc_com_chain.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Transmits an application packet through the supplied transmit chain.
 *
 * @param[in] packet Application packet to send.
 * @param[in] chain The chain to use.
 * @return fpc_com_result_t
 */
fpc_com_result_t fpc_hcp_transmit(fpc_hcp_packet_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Receives an application packet through the supplied transmit chain.
 *
 * @param[in, out] packet Pointer to pre-allocated packet struct.
 * @param[in] chain The chain to use.
 * @return ::fpc_com_result_t
 */
fpc_com_result_t fpc_hcp_receive(fpc_hcp_packet_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Add argument to packet.
 *
 * @note This function does not allocate any memory, it will only set the argument variables.
 *
 * @param[in] packet Packet to add to.
 * @param[in] arg Argument id.
 * @param[in] size Size of argument data.
 * @param[in] free_data Set to true if data should be owned by the argument, false if user still owns
 *                      data.
 * @param[in] data Pointer to argument data.
 * @return true = success, false = failure.
 */
bool fpc_hcp_arg_add(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg, uint16_t size, bool free_data,
        void *data);

/**
 * @brief Check if command contains selected argument key.
 *
 * @param[in] packet The packet to scan.
 * @param[in] arg Argument to look for.
 * @return true if found, false if not found.
 */
bool fpc_hcp_arg_check(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg);

/**
 * @brief Get Argument with specified key.
 *
 * @param[in] packet The packet to operate on.
 * @param[in] arg The arg to retrieve.
 * @return Pointer to ::fpc_hcp_arg_data_t is successful, otherwise NULL.
 */
fpc_hcp_arg_data_t *fpc_hcp_arg_get(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg);

/**
 * @brief Copy data from an argument with specified key.
 *
 * Argument data will be copied to specified data buffer.
 * Remaining bytes in data will be cleared if the argument data size is less
 * than data size when the argument contains data.
 *
 * @param[in] packet The packet to operate on.
 * @param[in] arg The arg to retrieve data from.
 * @param[in] data_size Number of bytes to copy.
 * @param[in, out] data Pointer to data buffer.
 *
 * @return True if argument found, false if not found.
 */
bool fpc_hcp_arg_copy_data(fpc_hcp_packet_t *packet, fpc_hcp_arg_t arg, uint16_t data_size,
        uint8_t *data);

/**
 * @brief Frees the resources held by the packet i.e. the dynamic data held in the arguments.
 *
 * @param[in] chain Pointer to the communication chain used to retrieve the packet.
 * @param[in] packet Pointer to packet.
 */
void fpc_hcp_free(fpc_com_chain_t *chain, fpc_hcp_packet_t *packet);

/**
 * @brief Calculate serialized packet size.
 *
 * @param[in] packet Packet to calculate.
 * @param[in, out] num_args Will return number of arguments held by the command can be set to NULL.
 * @return Serialized size.
 */
uint16_t fpc_hcp_get_size(fpc_hcp_packet_t *packet, uint16_t *num_args);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_HCP_H */
