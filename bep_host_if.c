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
 * @file    bep_host_if.c
 * @brief   BEP Host Interface implementation.
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>


#include "pins_arduino.h"

#include "fpc_bep_types.h"
#include "fpc_hcp_common.h"
#include "fpc_com_result.h"
#include "fpc_hcp.h"
#include "platform.h"
#include "com_common.h"

#include "bep_host_if.h"

/** Returns the number of elements in an array. */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define RECEIVE_TIMEOUT 10

#ifdef DEBUG
#define log_debug(format, ...) WRITE(format, ##__VA_ARGS__)
#else
#define log_debug(format, ...)
#endif
#define log_info(format, ...) WRITE(format, ##__VA_ARGS__)
#define log_error(format, ...) WRITE(format, ##__VA_ARGS__)

/** Maximum attempts for capture image */
static const uint8_t MAX_CAPTURE_ATTEMPTS = 15U;
static const uint16_t CAPTURE_TIMEOUT = 3000;


/**
 * @brief Helper function for sending HCP commands
 *
 * @param chain HCP communication chain
 * @param command_id command to send
 * @param arg_key1 first key to add to the command
 * @param arg_data1 first argument data to add
 * @param arg_data1_length first data length of argument data
 * @param arg_key2 second key to add to the command
 * @param arg_data2 second argument data to add
 * @param arg_data2_length second data length of argument data
 * @param arg_key3 third key to add to the command
 * @param arg_data3 third argument data to add
 * @param arg_data3_length third data length of argument data
 * @return ::fpc_bep_result_t
 */

 void (*new_ws_callback)(char *ch);

 
static fpc_bep_result_t send_command_args3(fpc_com_chain_t *chain, fpc_hcp_cmd_t command_id,
        fpc_hcp_arg_t arg_key1, void *arg_data1, uint16_t arg_data1_length,
        fpc_hcp_arg_t arg_key2, void *arg_data2, uint16_t arg_data2_length,
        fpc_hcp_arg_t arg_key3, void *arg_data3, uint16_t arg_data3_length)
{
    fpc_hcp_packet_t command;
    fpc_bep_result_t bep_result;
    fpc_com_result_t com_result;
    fpc_hcp_arg_data_t args_tx[10] = {{ 0 }};

    memset(&command, 0x0, sizeof(command));
    command.arguments = args_tx;
    command.num_args = ARRAY_SIZE(args_tx);
    command.id = command_id;

    if (arg_key1 != ARG_NONE) {
        if (!fpc_hcp_arg_add(&command, arg_key1, arg_data1_length, false, arg_data1)) {
            log_error("%s:%u Could not add arg:%u\n", __func__, __LINE__, arg_key1);
            bep_result = FPC_BEP_RESULT_NO_MEMORY;
            goto exit;
        }
    }

    if (arg_key2 != ARG_NONE) {
        if (!fpc_hcp_arg_add(&command, arg_key2, arg_data2_length, false, arg_data2)) {
            log_error("%s:%u Could not add arg:%u\n", __func__, __LINE__, arg_key2);
            bep_result = FPC_BEP_RESULT_NO_MEMORY;
            goto exit;
        }
    }

    if (arg_key3 != ARG_NONE) {
        if (!fpc_hcp_arg_add(&command, arg_key3, arg_data3_length, false, arg_data3)) {
            log_error("%s:%u Could not add arg:%u\n", __func__, __LINE__, arg_key3);
            bep_result = FPC_BEP_RESULT_NO_MEMORY;
            goto exit;
        }
    }

    com_result = fpc_hcp_transmit(&command, chain);
    bep_result = com_to_bep_result(com_result);

    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u ERROR %d\n", __func__, __LINE__, bep_result);
    }

exit:
    fpc_hcp_free(chain, &command);

    return bep_result;
}

static fpc_bep_result_t send_command_args2(fpc_com_chain_t *chain, fpc_hcp_cmd_t command_id,
        fpc_hcp_arg_t arg_key1, void *arg_data1, uint16_t arg_data1_length,
        fpc_hcp_arg_t arg_key2, void *arg_data2, uint16_t arg_data2_length)
{
    return send_command_args3(chain, command_id, arg_key1, arg_data1, arg_data1_length,
        arg_key2, arg_data2, arg_data2_length, ARG_NONE, NULL, 0);
}

static fpc_bep_result_t send_command_no_args(fpc_com_chain_t *chain, fpc_hcp_cmd_t command_id)
{
    return send_command_args2(chain, command_id, ARG_NONE, NULL, 0, ARG_NONE, NULL, 0);
}

static fpc_bep_result_t send_command(fpc_com_chain_t *chain, fpc_hcp_cmd_t command_id,
        fpc_hcp_arg_t arg_key, void *arg_data, uint16_t arg_data_length)
{
    return send_command_args2(chain, command_id, arg_key, arg_data, arg_data_length,
        ARG_NONE, NULL, 0);
}

/**
 * @brief Helper function for receiving HCP commands

 * @param command_id command to send
 * @param arg_key1 first key to receive
 * @param arg_data1 first argument data
 * @param arg_data1_length first argument data length
 * @param arg_key2 second key to receive
 * @param arg_data2 second argument data
 * @param arg_data2_length second argument
 * @return ::fpc_bep_result_t
 */
static fpc_bep_result_t receive_result_args2(fpc_com_chain_t *chain,
        fpc_hcp_arg_t arg_key1, void *arg_data1, uint16_t arg_data1_length,
        fpc_hcp_arg_t arg_key2, void *arg_data2, uint16_t arg_data2_length)
{
    fpc_hcp_packet_t response;
    fpc_hcp_arg_data_t args_rx[10] = {{ 0 }};
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_GENERAL_ERROR;
    fpc_hcp_arg_data_t *arg_data;
    
    memset(&response, 0x0, sizeof(fpc_hcp_cmd_t));
    response.arguments = args_rx;
    response.num_args = ARRAY_SIZE(args_rx);

//    printf("fn::receive_result_args2 bep_result\n");
    
    do {
        fpc_com_result_t com_result = fpc_hcp_receive(&response, chain);
        bep_result = com_to_bep_result(com_result);
    } while (bep_result == FPC_BEP_RESULT_TIMEOUT);

    if (bep_result != FPC_BEP_RESULT_OK) {
//        printf("receive_result_args2 bep_result != FPC_BEP_RESULT_OK\n");
        goto exit;
    }

    /* Check bep result first */
    arg_data = fpc_hcp_arg_get(&response, ARG_RESULT);
    if (arg_data) {
        bep_result = *(int8_t *)arg_data->data;
//        printf("arg_data bep_result != FPC_BEP_RESULT_OK   %d  %d\n",bep_result,FPC_BEP_RESULT_OK);
    } else {
        log_error("%s Result argument missing\n", __func__);
        bep_result = FPC_BEP_RESULT_INVALID_ARGUMENT;
    }
    if (bep_result != FPC_BEP_RESULT_OK) {
//        printf("fpc_hcp_arg_get bep_result != FPC_BEP_RESULT_OK\n");
        goto exit;
    }

    /* Get first argument */
    if (arg_key1 != ARG_NONE) {
        arg_data = fpc_hcp_arg_get(&response, arg_key1);
        if (arg_data && arg_data->size <= arg_data1_length) {
            memcpy(arg_data1, arg_data->data, arg_data->size);
        } else {
            log_error("%s %d argument missing\n", __func__, arg_key1);
            bep_result = FPC_BEP_RESULT_INVALID_ARGUMENT;
            goto exit;
        }
    }

    /* Get second argument */
    if (arg_key2 != ARG_NONE) {
        arg_data = fpc_hcp_arg_get(&response, arg_key2);
        if (arg_data && arg_data->size <= arg_data2_length) {
            memcpy(arg_data2, arg_data->data, arg_data->size);
        } else {
            /* Not an error since the second argument is optional */
            log_debug("%s %d argument missing\n", __func__, arg_key2);
        }
    }

exit:
    fpc_hcp_free(chain, &response);

    return bep_result;
}

fpc_bep_result_t receive_result_no_args(fpc_com_chain_t *chain)
{
    return receive_result_args2(chain, ARG_NONE, NULL, 0, ARG_NONE, NULL, 0);
}

static fpc_bep_result_t receive_result_args1(fpc_com_chain_t *chain,
        fpc_hcp_arg_t arg_key, void *arg_data, uint16_t arg_data_length)
{
    return receive_result_args2(chain, arg_key, arg_data, arg_data_length, ARG_NONE, NULL, 0);
}


fpc_bep_result_t bep_capture(fpc_com_chain_t *chain, uint16_t timeout)
{
    fpc_bep_result_t bep_result;

    log_info("Put finger on sensor\n");
    new_ws_callback("Put finger on sensor\n");
    

    /* Capture finger down */
    bep_result = send_command(chain, CMD_CAPTURE, ARG_TIMEOUT, &timeout, sizeof(timeout));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u Error transmitting CMD_CAPTURE\n", __func__, __LINE__);
        return bep_result;
    }
   platform_delay_us();
   return receive_result_no_args(chain);
}

fpc_bep_result_t bep_enroll_finger(fpc_com_chain_t *chain)
{
   uint32_t samples_remaining = 0;
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;
    bool enroll_done = false;

    log_info("Enroll start\n");
    /* Enroll start */
    bep_result = send_command(chain, CMD_ENROLL, ARG_START, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s, ERROR line:%u\n", __func__, __LINE__);
        goto exit;
    }
    
    log_info("receive_result_no_args\n");
    delay(50);
    bep_result = receive_result_no_args(chain);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
        goto exit;
    }

    for (uint8_t i = 0; i < MAX_CAPTURE_ATTEMPTS; ++i) {

        log_info("Capture\n");
        bep_result = bep_capture(chain, CAPTURE_TIMEOUT);
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("Capture failed\n");
            new_ws_callback("Capture failed\n");
            break;
        }

        log_info("Capture done 1. Remove finger from sensor\n");
        new_ws_callback("Capture done 1. Remove finger from sensor\n");
            
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("%s:%u, ERROR receiving, result=%d\n", __func__, __LINE__, bep_result);
            new_ws_callback("ERROR receiving, result\n");
            continue;
        }
//        send_command
        /* Enroll add */
        bep_result = send_command(chain, CMD_ENROLL, ARG_ADD, NULL, 0);
        if (bep_result != FPC_BEP_RESULT_OK) {
            platform_delay_us();
            new_ws_callback("ERROR 3298 DISCLOSED\n");
            log_error("%s:%u, ERROR 3298 DISCLOSED\n", __func__, __LINE__);
            continue;
        }
        platform_delay_us();
        log_info("Finished \n\n\n\n");
       
        
        bep_result = receive_result_args1(chain, ARG_COUNT, &samples_remaining,
            sizeof(samples_remaining));
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("%s:%u, ENROLL 1238945678 ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
            continue;
        }

        log_info("Enroll samples remaining: %d\n", samples_remaining);
        char ch[100];
        sprintf(ch, "Enroll samples remaining: %d\n", samples_remaining);
        new_ws_callback(ch);

        if (samples_remaining == 0U) {
            log_info("Fingerprint Added Succesfully \n");
            char ch[100];
            sprintf(ch, "Fingerprint Added Succesfully Now Save Template \n");
            new_ws_callback(ch);
            enroll_done = true;
            break;
        }

        bep_result = send_command(chain, CMD_WAIT, ARG_FINGER_UP, NULL, 0);
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("%s:%u, ERROR\n", __func__, __LINE__);
            continue;
        }
        
        bep_result = receive_result_no_args(chain);
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("%s:%u, ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
            continue;
        }
    }

    bep_result = send_command(chain, CMD_ENROLL, ARG_FINISH, NULL, 0);
    if (bep_result == FPC_BEP_RESULT_OK) {
        bep_result = receive_result_no_args(chain);
        if (bep_result != FPC_BEP_RESULT_OK) {
            log_error("%s:%u, ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
        }
    }

exit:
    return (!enroll_done) ? FPC_BEP_RESULT_GENERAL_ERROR : bep_result;
}

fpc_bep_result_t bep_identify_finger(fpc_com_chain_t *chain, uint16_t *template_id, bool *match)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;

    *match = false;

    bep_result = bep_capture(chain, CAPTURE_TIMEOUT);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Capture failed result=%d\n", bep_result);
        return bep_result;
    }

    log_info("Capture done. Remove finger from sensor\n");

    bep_result = bep_image_extract(chain);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Extract failed\n");
        return bep_result;
    }

    bep_result = send_command(chain, CMD_IDENTIFY, ARG_NONE, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Identify failed\n");
        return bep_result;
    }
    
    bep_result = receive_result_args2(chain, ARG_MATCH, match, sizeof(bool),
        ARG_ID, template_id, sizeof(uint16_t));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Identify failed\n");
        return bep_result;
    }

    return bep_result;
}

fpc_bep_result_t bep_save_template(fpc_com_chain_t *chain, uint16_t template_id)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;

    bep_result = send_command_args2(chain, CMD_TEMPLATE, ARG_SAVE, NULL, 0, ARG_ID, &template_id,
        sizeof(template_id));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_delete_template(fpc_com_chain_t *chain, uint16_t template_id)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;

    if (template_id == REMOVE_ID_ALL_TEMPLATES) {
        bep_result = send_command_args2(chain, CMD_STORAGE_TEMPLATE, ARG_DELETE, NULL, 0,
            ARG_ALL, NULL, 0);
    } else {
        bep_result = send_command_args2(chain, CMD_STORAGE_TEMPLATE, ARG_DELETE, NULL, 0,
            ARG_ID, &template_id, sizeof(template_id));
    }
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_get_template_count(fpc_com_chain_t *chain, uint32_t *template_count)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;

    bep_result = send_command(chain, CMD_STORAGE_TEMPLATE, ARG_COUNT, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u ERROR sending CMD_STORAGE_TEMPLATE\n", __func__, __LINE__);
        return bep_result;
    }

    bep_result = receive_result_args1(chain, ARG_COUNT, template_count, sizeof(template_count[0]));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
        return bep_result;
    }

    return bep_result;
}

fpc_bep_result_t bep_get_template_ids(fpc_com_chain_t *chain, uint16_t *template_ids,
    uint32_t nof_templates)
{
    fpc_bep_result_t bep_result = FPC_BEP_RESULT_OK;

    bep_result = send_command(chain, CMD_STORAGE_TEMPLATE, ARG_ID, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u ERROR sending CMD_STORAGE_TEMPLATE\n", __func__, __LINE__);
        return bep_result;
    }

    bep_result = receive_result_args1(chain, ARG_DATA, template_ids, nof_templates *
            sizeof(template_ids[0]));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR receiving status=%d\n", __func__, __LINE__, bep_result);
        return bep_result;
    }

    return bep_result;
}

fpc_bep_result_t bep_image_extract(fpc_com_chain_t *chain)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command(chain, CMD_IMAGE, ARG_EXTRACT, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Extract failed\n");
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_image_get_size(fpc_com_chain_t *chain, uint32_t *size)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command(chain, CMD_IMAGE, ARG_SIZE, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Extract failed\n");
        return bep_result;
    }

    log_info("Downloading image data...\n");

    return receive_result_args1(chain, ARG_SIZE, size, sizeof(size));
}

fpc_bep_result_t bep_image_get(fpc_com_chain_t *chain, uint8_t *data, uint32_t size)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command(chain, CMD_IMAGE, ARG_UPLOAD, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("Extract failed\n");
        return bep_result;
    }

    return receive_result_args1(chain, ARG_DATA, data, size);
}

fpc_bep_result_t bep_version(fpc_com_chain_t *chain, char *version, int len)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command_args2(chain, CMD_INFO, ARG_GET, NULL, 0, ARG_VERSION, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s, ERROR line:%u\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_args1(chain, ARG_VERSION, version, len);
}

fpc_bep_result_t bep_reset(fpc_com_chain_t *chain)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command_no_args(chain, CMD_RESET);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_sensor_calibrate(fpc_com_chain_t *chain)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command_no_args(chain, CMD_STORAGE_CALIBRATION);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_sensor_calibrate_remove(fpc_com_chain_t *chain)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command(chain, CMD_STORAGE_CALIBRATION, ARG_DELETE, NULL, 0);
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    return receive_result_no_args(chain);
}

fpc_bep_result_t bep_sensor_wait_for_finger(fpc_com_chain_t *chain, uint16_t timeout,
    uint16_t sleep_counter)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command_args3(chain, CMD_WAIT, ARG_FINGER_DOWN, NULL, 0,
        ARG_TIMEOUT, &timeout, sizeof(timeout),
        ARG_SLEEP, &sleep_counter, sizeof(sleep_counter));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

  
    return receive_result_no_args(chain);
//   return bep_result;
}

fpc_bep_result_t bep_sensor_wait_finger_not_present(fpc_com_chain_t *chain, uint16_t timeout)
{
    fpc_bep_result_t bep_result;

    bep_result = send_command_args2(chain, CMD_WAIT, ARG_FINGER_UP, NULL, 0,
        ARG_TIMEOUT, &timeout, sizeof(timeout));
    if (bep_result != FPC_BEP_RESULT_OK) {
        log_error("%s:%u, ERROR\n", __func__, __LINE__);
        return bep_result;
    }

    /* Wait for finger to be lifted from sensor */
    return receive_result_no_args(chain);
}

void assign_callback_forws(void *ptr) {
  new_ws_callback = ptr;
}
