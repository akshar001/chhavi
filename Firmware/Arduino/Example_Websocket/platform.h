/*
   Copyright (c) 2020 Fingerprint Cards AB

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef PLATFORM_H
#define PLATFORM_H

/**
   @file    platform.h
   @brief   Platform specific function interface
*/

#include <stdint.h>
#include <stdbool.h>
#include "fpc_com_result.h"


void assign_function(void *ptr);
void assign2_function(void *ptr);

/**
   @brief Custom printf with flush.
*/
#define WRITE(fmt, ...) { printf(fmt, ##__VA_ARGS__); fflush(stdout); }

/**
   @brief Initializes Physical layer.

   @param[in]       port        tty port to use.
   @param[in]       baudrate    Baudrate.
   @param[in]       timeout     Timeout in ms. Use 0 for infinity.
*/
bool platform_com_init(char *port, int *baudrate, int timeout);

/**
   @brief Sends data over communication port in blocking mode.

   @param[in]       size        Number of bytes to send.
   @param[in]       data        Data buffer to send.
   @param[in]       timeout     Timeout in ms. Use 0 for infinity.

   @return ::fpc_com_result_t
*/
fpc_com_result_t platform_com_send(uint16_t size, const uint8_t *data, uint32_t timeout,
                                   void *session);

/**
   @brief Receives data from communication port in blocking mode.

   @param[in]       size        Number of bytes to receive.
   @param[in, out]  data        Data buffer to fill.
   @param[in]       timeout     Timeout in ms. Use 0 for infinity.

   @return ::fpc_com_result_t
*/
fpc_com_result_t platform_com_receive(uint16_t size, uint8_t *data, uint32_t timeout,
                                      void *session);

/**
   @brief Get time in micro seconds

   @return time in us.
*/
uint64_t platform_get_time(void);

/**
   @brief Clear console screen
*/
void platform_clear_screen(void);

/**
   @brief Check IRQ
*/
bool platform_check_irq(void);

/**
   @brief Send reset
*/
void platform_reset(void);

void platform_delay_us();

void assign3_function(void *ptr);

#endif /* PLATFORM_H */
