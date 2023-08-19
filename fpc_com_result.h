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
 * @file    fpc_com_result.h
 * @brief   Communication result type definitions.
 */

#ifndef FPC_COM_RESULT_H
#define FPC_COM_RESULT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Communication result codes */
enum fpc_com_result {
    FPC_COM_RESULT_OK,
    FPC_COM_RESULT_NO_MEMORY,
    FPC_COM_RESULT_INVALID_ARGUMENT,
    FPC_COM_RESULT_NOT_IMPLEMENTED,
    FPC_COM_RESULT_IO_ERROR,
    FPC_COM_RESULT_TIMEOUT,
};
/** Communication result type */
typedef uint8_t fpc_com_result_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_COM_RESULT_H */
