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
 * @file    fpc_com_transport.h
 * @brief   Communication transport interface.
 */

#ifndef FPC_COM_TRANSPORT_H
#define FPC_COM_TRANSPORT_H

#include <stdint.h>

#include "fpc_com_chain.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Transmit a transport layer packet.
 *
 * @param[in] packet The packet to transmit.
 * @param[in] chain The chain to use.
 * @result ::fpc_com_result_t
 */
fpc_com_result_t fpc_com_transport_transmit(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Receive a transport layer packet.
 *
 * @param[in, out] packet The packet to populate.
 * @param[in] chain The chain to use.
 * @result ::fpc_com_result_t
 */
fpc_com_result_t fpc_com_transport_receive(fpc_com_packet_tsp_t *packet, fpc_com_chain_t *chain);

/**
 * @brief Returns the overhead of the layer.
 *
 * @param[out] offset The offset to the packet data.
 * @return Overhead size in bytes.
 */
uint16_t fpc_com_transport_get_overhead(uint16_t *offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_COM_TRANSPORT_H */
