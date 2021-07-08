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
 * @file    main.c
 * @brief   Main file for FPC BEP Host Communication example.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "bep_host_if.h"
#include "com_common.h"
#include "platform.h"

static void help(void)
{
    fprintf(stderr, "BEP Host Communication Application\n");
    fprintf(stderr, "Syntax: bep_host_com [-s speed (khz)] [-t timeout] [-h (help)]\n");
}

static const char *result_string(fpc_bep_result_t result)
{
    switch (result) {
        case FPC_BEP_RESULT_OK:
            return "OK";
        case FPC_BEP_RESULT_GENERAL_ERROR:
            return "General Error";
        case FPC_BEP_RESULT_INTERNAL_ERROR:
            return "Internal Error";
        case FPC_BEP_RESULT_INVALID_ARGUMENT:
            return "Invalid Argument";
        case FPC_BEP_RESULT_NOT_IMPLEMENTED:
            return "Not Implemented";
        case FPC_BEP_RESULT_CANCELLED:
            return "Cancelled";
        case FPC_BEP_RESULT_NO_MEMORY:
            return "No Memory";
        case FPC_BEP_RESULT_NO_RESOURCE:
            return "No Resource";
        case FPC_BEP_RESULT_IO_ERROR:
            return "IO Error";
        case FPC_BEP_RESULT_BROKEN_SENSOR:
            return "Broken Sensor";
        case FPC_BEP_RESULT_WRONG_STATE:
            return "Wrong State";
        case FPC_BEP_RESULT_TIMEOUT:
            return "Timeout";
        case FPC_BEP_RESULT_ID_NOT_UNIQUE:
            return "Id Not Unique";
        case FPC_BEP_RESULT_ID_NOT_FOUND:
            return "Id Not Found";
        case FPC_BEP_RESULT_INVALID_FORMAT:
            return "Invalid Format";
        case FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR:
            return "Image Capture Error";
        case FPC_BEP_RESULT_SENSOR_MISMATCH:
            return "Sensor Mismatch";
        case FPC_BEP_RESULT_INVALID_PARAMETER:
            return "Invalid Parameter";
        case FPC_BEP_RESULT_MISSING_TEMPLATE:
            return "Missing Template";
        case FPC_BEP_RESULT_INVALID_CALIBRATION:
            return "Invalid Calibration";
        case FPC_BEP_RESULT_STORAGE_NOT_FORMATTED:
            return "Storage Not Formatted";
        case FPC_BEP_RESULT_SENSOR_NOT_INITIALIZED:
            return "Sensor Not Initialized";
        case FPC_BEP_RESULT_TOO_MANY_BAD_IMAGES:
            return "Too Many Bad Images";
        case FPC_BEP_RESULT_NOT_SUPPORTED:
            return "Not Supported";
        case FPC_BEP_FINGER_NOT_STABLE:
            return "Finger Not Stable";
        case FPC_BEP_RESULT_NOT_INITIALIZED:
            return "Not Initialized";
        default:
            return "Unknown Error";
    }
    return NULL;
}

int main (int argc, char **argv)
{
    char *port = NULL;
    int speed = 1000;
    int timeout = 5;
    int index;
    uint8_t buffer[512];
    uint16_t size[2] = { 256, 256 };
    fpc_com_chain_t hcp_chain;

    index = 1;
    while (index < argc) {
        if (argv[index][0] == '-' || argv[index][0] == '/') {
            if (argv[index][1] == 's' && (index + 1) < argc) {
                speed = atoi(argv[index + 1]);
                index++;
            }
            else if (argv[index][1] == 't' && (index + 1) < argc) {
                timeout = atoi(argv[index + 1]);
                index++;
            }
            else if (argv[index][1] == 'h') {
                help();
                exit(1);
            }
            else {
                WRITE("Unknown flag %c\n", argv[index][1]);
                return 1;
            }
        }
        else {
            WRITE("Unknown option %s\n", argv[index]);
            return 1;
        }
        index++;
    }

    /* Cap at 10MHz */
    if (speed > 10000) {
        speed = 10000;
    }

    if (!platform_com_init(port, &speed, timeout)) {
        WRITE("Com initialization failed\n");
        exit(1);
    }

    init_com_chain(&hcp_chain, buffer, size, NULL);

    while(1) {
        char cmd[100];
        fpc_bep_result_t res = FPC_BEP_RESULT_OK;
        uint16_t template_id;
        bool match;

        platform_clear_screen();
        WRITE("BEP Host Interface\n");
        WRITE("[speed: %d]\n", speed);
        WRITE("Timeout: %ds\n", timeout);
        WRITE("-------------------\n\n");
        WRITE("Possible options:\n");
        WRITE("a: Enroll finger\n");
        WRITE("b: Capture and identify finger\n");
        WRITE("c: Remove all templates\n");
        WRITE("d: Save template\n");
        WRITE("e: Remove template\n");
        WRITE("f: Capture image\n");
        WRITE("g: Image upload\n");
        WRITE("h: Get version\n");
        WRITE("i: Wait for finger\n");
        WRITE("q: Exit program\n");
        WRITE("\nOption>> ");
        fgets(cmd, sizeof(cmd), stdin);
        switch (cmd[0]) {
            case 'a':
                res = bep_enroll_finger(&hcp_chain);
                break;
            case 'b':
                res = bep_identify_finger(&hcp_chain, &template_id, &match);
                if (res == FPC_BEP_RESULT_OK) {
//                    if (match) {
//                      char ch[100];
//                      sprintf(ch, "Match with template id: %d\n", template_id);
//                      new_ws_callback(ch);
//                    } else {
//                        new_ws_callback("No match\n");
//                    }
                }
                break;
            case 'c':
                res = bep_delete_template(&hcp_chain, REMOVE_ID_ALL_TEMPLATES);
                break;
            case 'd':
//                WRITE("Template id: ");
//                fgets(cmd, sizeof(cmd), stdin);
                template_id = 15;
                res = bep_save_template(&hcp_chain, template_id);
                break;
            case 'e':
                WRITE("Template id: ");
                fgets(cmd, sizeof(cmd), stdin);
                template_id = atoi(cmd);
                res = bep_delete_template(&hcp_chain, template_id);
                break;
            case 'f':
                WRITE("Timeout: ");
                fgets(cmd, sizeof(cmd), stdin);
                res = bep_capture(&hcp_chain, atoi(cmd));
                break;
            case 'g': {
                uint32_t size;
                res = bep_image_get_size(&hcp_chain, &size);
                if (res == FPC_BEP_RESULT_OK) {
                    uint8_t *buf = malloc(size);
                    if (buf) {
                      res = bep_image_get(&hcp_chain, buf, size);
                      if (res == FPC_BEP_RESULT_OK) {
                          FILE *f = fopen("image.raw", "wb");
                          if (f) {
                              fwrite(buf, 1, size, f);
                              fclose(f);
                              WRITE("Image saved as image.raw\n");
                          }
                      }
                    }

                }
                break;
            }
            case 'h': {
                char version[100];

                memset(version, 0, 100);
                res = bep_version(&hcp_chain, version, 99);
                if (res == FPC_BEP_RESULT_OK) {
                  WRITE("%s\n", version);
                }
                break;
            }
            case 'i': {
                uint16_t timeout;
                uint16_t sleep_counter;

                WRITE("Timeout: ");
                fgets(cmd, sizeof(cmd), stdin);
                timeout = atoi(cmd);
                WRITE("Sleep polling interval (4-1020 ms): ");
                fgets(cmd, sizeof(cmd), stdin);
                sleep_counter = atoi(cmd);

                res = bep_sensor_wait_for_finger(&hcp_chain, timeout, sleep_counter);
                break;
            }
            case 'q':
                return 0;
            default:
                WRITE("\nUnknown command\n");
        }
        if (res == FPC_BEP_RESULT_OK) {
            WRITE("\nCommand succeeded\n");
        } else {
            WRITE("\nCommand failed with error code %s (%d)\n", result_string(res), res);
        }
        WRITE("Press enter to continue...");
        fgets(cmd, sizeof(cmd), stdin);
    }

    return 0;
}
