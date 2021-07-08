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
 * @file    fpc_com_link.h
 * @brief   Communication link interface.
 */

#ifndef FPC_COM_LINK_H
#define FPC_COM_LINK_H

#include <stdbool.h>
#include <stdint.h>

#include "fpc_com_result.h"
#include "fpc_com_chain.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Sends a packet over the physical link in blocking mode.
 *
 * @param[in] packet Packet to transmit.
 * @param[in] chain The communication chain to use.
 * @return ::fpc_com_result_t
 */
fpc_com_result_t fpc_com_link_transmit(fpc_com_packet_link_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Receives a packet from the physical link.
 *
 * @param[in, out] packet Packet to populate.
 * @param[in] chain The communication chain to use.
 * @return ::fpc_com_result_t
 */
fpc_com_result_t fpc_com_link_receive(fpc_com_packet_link_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Returns the overhead of the layer.
 *
 * @param[out] offset The offset to the packet data.
 * @return Overhead size in bytes.
 */
uint16_t fpc_com_link_get_overhead(uint16_t *offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_COM_LINK_H */
