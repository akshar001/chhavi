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
 * @file    com_app_cleartext.h
 * @brief   TODO
 */

#ifndef COM_APP_CLEARTEXT_H
#define COM_APP_CLEARTEXT_H

#include "fpc_com_chain.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Sends a packet over the physical link in blocking mode.
 *
 * @param packet Packet to transmit.
 * @param chain The transmit chain to use.
 * @return ::fpc_com_result_t
 */
fpc_com_result_t com_app_clr_transmit(fpc_com_chain_t *chain);

/**
 * Receives a packet from the physical link.
 *
 * @param packet Packet to populate.
 * @param chain The transmit chain to use.
 * @return ::fpc_com_result_t
 */
fpc_com_result_t com_app_clr_receive(fpc_com_chain_t *chain);

/**
 * @brief Returns the overhead of the layer.
 * @return Overhead size in bytes.
 */
uint16_t com_app_clr_get_overhead(uint16_t *offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COM_APP_CLEARTEXT_H */
