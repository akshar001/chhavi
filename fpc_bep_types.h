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

#ifndef FPC_BEP_TYPES_H
#define FPC_BEP_TYPES_H

#include <stdbool.h>

/**
 * @file    fpc_bep_types.h
 * @brief   Biometric Embedded Platform types.
 *
 * This is the common types used by Biometric Embedded Platform (BEP) library.
 *
 * @note This is a work-in-progress specification. Implementers are informed
 * that this API may change without providing any backward compatibility.
 * However it is FPC's ambition that the API shall remain compatible between
 * releases.
 */

/** @brief Common result returned by BEP functions. */
typedef enum {
    /** No errors occurred. */
    FPC_BEP_RESULT_OK = 0,
    /** General error. */
    FPC_BEP_RESULT_GENERAL_ERROR = -1,
    /** Internal error. */
    FPC_BEP_RESULT_INTERNAL_ERROR = -2,
    /** Invalid argument. */
    FPC_BEP_RESULT_INVALID_ARGUMENT = -3,
    /** The functionality is not implemented. */
    FPC_BEP_RESULT_NOT_IMPLEMENTED = -4,
    /** The operation was cancelled. */
    FPC_BEP_RESULT_CANCELLED = -5,
    /** Out of memory. */
    FPC_BEP_RESULT_NO_MEMORY = -6,
    /** Resources are not available. */
    FPC_BEP_RESULT_NO_RESOURCE = -7,
    /** An I/O error occurred. */
    FPC_BEP_RESULT_IO_ERROR = -8,
    /** Sensor is broken. */
    FPC_BEP_RESULT_BROKEN_SENSOR = -9,
    /** The operation cannot be performed in the current state. */
    FPC_BEP_RESULT_WRONG_STATE = -10,
    /** The operation timed out. */
    FPC_BEP_RESULT_TIMEOUT = -11,
    /** The ID is not unique. */
    FPC_BEP_RESULT_ID_NOT_UNIQUE = -12,
    /** The ID is not found. */
    FPC_BEP_RESULT_ID_NOT_FOUND = -13,
    /** The format is invalid. */
    FPC_BEP_RESULT_INVALID_FORMAT = -14,
    /** An image capture error occurred. */
    FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR = -15,
    /** Sensor hardware id or sensor configuration mismatch. */
    FPC_BEP_RESULT_SENSOR_MISMATCH = -16,
    /** Invalid parameter. */
    FPC_BEP_RESULT_INVALID_PARAMETER = -17,
    /** Missing Template. */
    FPC_BEP_RESULT_MISSING_TEMPLATE = -18,
    /** Invalid Calibration.*/
    FPC_BEP_RESULT_INVALID_CALIBRATION = -19,
    /** Calibration/template storage not formatted.*/
    FPC_BEP_RESULT_STORAGE_NOT_FORMATTED = -20,
    /** Sensor hasn't been initialized. */
    FPC_BEP_RESULT_SENSOR_NOT_INITIALIZED = -21,
    /** Enroll fail after too many bad images. */
    FPC_BEP_RESULT_TOO_MANY_BAD_IMAGES = -22,
    /** Cryptographic operation failed. */
    FPC_BEP_RESULT_CRYPTO_ERROR = -23,
    /** The functionality is not supported. */
    FPC_BEP_RESULT_NOT_SUPPORTED = -24,
    /** Finger not stable during image capture. */
    FPC_BEP_FINGER_NOT_STABLE = -25,
    /** The functionality could not be used before it's initialized. */
    FPC_BEP_RESULT_NOT_INITIALIZED = -26,
} fpc_bep_result_t;

#endif /* FPC_BEP_TYPES_H */
