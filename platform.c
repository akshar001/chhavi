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

/**
   @file    platform
   @brief
*/
#include "Arduino.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include "fpc_com_result.h"
#include "platform.h"

void (*function_send)(uint16_t size, const uint8_t *data, uint32_t timeout, void *session);
void (*function2_send)(uint16_t size,  uint8_t *data, uint32_t timeout, void *session);
void (*new_delay_serial)();


bool platform_com_init(char *port, int *baudrate, int timeout)
{
  //  Serial.println("platform_com_init");
  //     uint32_t status = ftdi_spi_open(baudrate);
  //     if (status) {
  //         WRITE("FTDI open failed with status %d\r\n", status);
  // #if !defined(_WIN32) && (defined(__unix__) || defined(__unix)) && !defined(__CYGWIN__)
  //         WRITE("Try sudo rmmod ftdi_sio and sudo rmmod usb_serial");
  // #endif
  //         return false;
  //     }

  return true;
}

void assign_function(void *ptr) {
  function_send = ptr;
}

void assign2_function(void *ptr) {
  function2_send = ptr;
}

void assign3_function(void *ptr) {
  new_delay_serial = ptr;
}

fpc_com_result_t platform_com_send(uint16_t size, const uint8_t *data, uint32_t timeout,
                                   void *session)
{

//  printf("fn::platform_com_send \n");
  fpc_com_result_t res = FPC_COM_RESULT_OK;
  function_send(size, data, timeout, session);
  return res;
}

fpc_com_result_t platform_com_receive(uint16_t size, uint8_t *data, uint32_t timeout,
                                      void *session)
{
//  printf("fn::platform_com_receive \n");
  fpc_com_result_t res = FPC_COM_RESULT_OK;
  function2_send(size, data, timeout, session);
  return res;
}

uint64_t platform_get_time(void)
{
  struct timeval current_time;
  uint64_t time_in_ms;

  gettimeofday(&current_time, NULL);
  /* Seconds and microseconds are converted to milliseconds */
  time_in_ms = current_time.tv_usec / 1000 + current_time.tv_sec * 1000;

  return time_in_ms;
}

void platform_delay_us()
{
  new_delay_serial();
}

void platform_clear_screen(void)
{
  // #ifdef _WIN32
  //     system("cls");
  // #else
  //     system("clear");
  // #endif
}

bool platform_check_irq(void)
{
  // int irq;

  // (void)ftdi_spi_check_irq(&irq);

  // return !(irq == 0);
}

void platform_reset(void)
{
  // ftdi_set_reset_level(0);
  // platform_delay_us(1000);
  // ftdi_set_reset_level(1);
}
