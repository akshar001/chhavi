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
 * @file    fpc_hcp_common.h
 * @brief   Host Communication Protocol common type definitions.
 */

#ifndef FPC_HCP_COMMON_H
#define FPC_HCP_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Returns the smallest of two values. */
#define HCP_MIN(x, y) (((x) < (y)) ? (x) : (y))

/** Program specific commands base number */
#define CMD_APP_BASE_VAL 0xE000

/** Program specific arguments base number */
#define ARG_APP_BASE_VAL 0x7000

/** HCP Command definitions */
enum fpc_hcp_cmd {
    CMD_NONE                = 0x0000,

    /* Biometry */
    CMD_CAPTURE             = 0x0001,
    CMD_ENROLL              = 0x0002,
    CMD_IDENTIFY            = 0x0003,
    CMD_MATCH               = 0x0004,
    CMD_IMAGE               = 0x0005,
    CMD_TEMPLATE            = 0x0006,
    CMD_WAIT                = 0x0007,
    CMD_SETTINGS            = 0x0008,

    /* Sensor */
    CMD_NAVIGATE            = 0x1001,
    CMD_SENSOR              = 0x1002,
    CMD_DEADPIXELS          = 0x1003,

    /* Security */
    CMD_CONNECT             = 0x2001,
    CMD_RECONNECT           = 0x2002,

    /* Firmware */
    CMD_RESET               = 0x3002,
    CMD_CANCEL              = 0x3003,
    CMD_INFO                = 0x3004,

    /* Storage */
    CMD_STORAGE_TEMPLATE    = 0x4002,
    CMD_STORAGE_CALIBRATION = 0x4003,
    CMD_STORAGE_LOG         = 0x4004,
    CMD_STORAGE_SETTINGS    = 0x4005,

    /* Hardware */
    CMD_TEST                = 0x5001,
    CMD_MCU                 = 0x5002,
    CMD_GPIO                = 0x5003,

    /* Communication */
    CMD_COMMUNICATION       = 0x6001,

    /* Application specific commands */
    CMD_APP_BASE            = CMD_APP_BASE_VAL,

    /* Debug */
    CMD_DIAG                = 0xF003,

    CMD_FFFF                = 0xFFFF,
};
/** HCP Command type */
typedef uint16_t fpc_hcp_cmd_t;

/** HCP Argument definitions */
enum fpc_hcp_arg {
    ARG_NONE            = 0x0000,

    /* Biometry */
    ARG_FINGER_DOWN     = 0x0001,
    ARG_FINGER_UP       = 0x0002,
    ARG_START           = 0x0003,
    ARG_ADD             = 0x0004,
    ARG_FINISH          = 0x0005,
    ARG_ID              = 0x0006,
    ARG_ALL             = 0x0007,
    ARG_EXTRACT         = 0x0008,
    ARG_MATCH_IMAGE     = 0x0009,
    ARG_MATCH           = 0x000A,

    /* Data */
    ARG_ACQUIRE         = 0x1001,
    ARG_RELEASE         = 0x1002,
    ARG_SET             = 0x1003,
    ARG_GET             = 0x1004,
    ARG_UPLOAD          = 0x1005,
    ARG_DOWNLOAD        = 0x1006,
    ARG_CREATE          = 0x1007,
    ARG_SAVE            = 0x1008,
    ARG_DELETE          = 0x1009,
    ARG_DATA            = 0x100A,
    ARG_UPDATE          = 0x100B,
    ARG_SEQ_NR          = 0x100C,
    ARG_SEQ_LEN         = 0x100D,

    /* Results */
    ARG_RESULT          = 0x2001,
    ARG_COUNT           = 0x2002,
    ARG_SIZE            = 0x2003,
    ARG_LEVEL           = 0x2004,
    ARG_FORMAT          = 0x2005,
    ARG_FLAG            = 0x2006,
    ARG_PROPERTIES      = 0x2007,
    ARG_SPEED           = 0x2008,
    ARG_PROD_TEST       = 0x2009,

    /* Sensor */
    ARG_SENSOR_TYPE     = 0x3001,
    ARG_WIDTH           = 0x3002,
    ARG_HEIGHT          = 0x3003,
    ARG_RESET           = 0x3004,
    ARG_DPI             = 0x3005,
    ARG_MAX_SPI_CLOCK   = 0x3006,
    ARG_NUM_SUB_AREAS_WIDTH = 0x3007,
    ARG_NUM_SUB_AREAS_HEIGHT = 0x3008,
    ARG_IRQ_STATUS      = 0x3009,
    ARG_RESET_HARD      = 0x300A,

    /* MCU */
    ARG_IDLE            = 0x4001,
    ARG_SLEEP           = 0x4002,
    ARG_DEEP_SLEEP      = 0x4003,
    ARG_POWER_MODE      = 0x4004,
    ARG_BUSY_WAIT       = 0x4005,

    /* Misc */
    ARG_TIMEOUT         = 0x5001,
    ARG_DONE            = 0x5002,

    /* Info */
    ARG_BOOT            = 0x6001,
    ARG_STATUS          = 0x6002,
    ARG_VERSION         = 0x6003,
    ARG_UNIQUE_ID       = 0x6004,

    /* Application specific arguments */
    ARG_APP_BASE        = ARG_APP_BASE_VAL,

    /* VSM */
    ARG_NONCE           = 0x8001,
    ARG_MAC             = 0x8002,
    ARG_RANDOM          = 0x8003,
    ARG_CLAIM           = 0x8004,
    ARG_PUBLIC_KEY      = 0x8005,
    ARG_CIPHERTEXT      = 0x8006,

    /* Communication */
    ARG_MTU             = 0x9001,

    /* Debug */
    ARG_STACK           = 0xE001,
    ARG_FILL            = 0xE002,
    ARG_HEAP            = 0xE003,

    /* Log */
    ARG_MODE            = 0xF001,
    ARG_DEBUG           = 0xF002,

    ARG_FFFF            = 0xFFFF,
};
/** HCP Argument type */
typedef uint16_t fpc_hcp_arg_t;

/**
 * @brief Command Argument
 */
typedef struct fpc_hcp_arg_data {
    /** Argument */
    fpc_hcp_arg_t arg;
    /** Size of data */
    uint16_t size;
    /** Free data inside HCP */
    bool free_data;
    /** Pointer to data */
    uint8_t *data;
} fpc_hcp_arg_data_t;

/**
 * @brief Application Command Packet
 */
typedef struct fpc_hcp_packet {
    /** Command ID */
    fpc_hcp_cmd_t id;
    /** Number of arguments */
    uint16_t num_args;
    /** Pointer to argument data */
    fpc_hcp_arg_data_t *arguments;
} fpc_hcp_packet_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FPC_HCP_COMMON_H */
