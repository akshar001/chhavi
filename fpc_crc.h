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

#ifndef FPC_CRC_H
#define FPC_CRC_H

/**
 * @file    fpc_crc.h
 * @brief   Functionality for calculating CRC-32.
 */

#include <stdint.h>

/**
 * @brief Calculates CRC-32 value for the data in the buffer.
 *
 * @param crc Accumulated CRC-32 value, must be 0 on first call.
 * @param buf Buffer with data to calculate CRC-32 for.
 * @param size Size of buffer in number of bytes.
 * @return CRC-32 value for the data in buffer.
 */
uint32_t fpc_crc(uint32_t crc, const void *buf, uint32_t size);

#endif /* FPC_CRC_H */
