
/*
 * NOTICE
 *
 * © 2014 Tile Inc.  All Rights Reserved.

 * All code or other information included in the accompanying files (“Tile Source Material”)
 * is CONFIDENTIAL and PROPRIETARY information of Tile Inc. and your access and use is subject
 * to the terms of Tile’s non-disclosure agreement as well as any other applicable agreement
 * with Tile.  The Tile Source Material may not be shared or disclosed outside your company,
 * nor distributed in or with any devices.  You may not use, copy or modify Tile Source
 * Material or any derivatives except for the purposes expressly agreed and approved by Tile.
 * All Tile Source Material is provided AS-IS without warranty of any kind.  Tile does not
 * warrant that the Tile Source Material will be error-free or fit for your purposes.
 * Tile will not be liable for any damages resulting from your use of or inability to use
 * the Tile Source Material.
 * You must include this Notice in any copies you make of the Tile Source Material.
 */

/** @file tcu.h
 ** @brief Tile Connection Update: used for changing the connection parameters.
 */

#ifndef TCU_H_
#define TCU_H_

#include <stdint.h>
#include "toa.h"

/** \defgroup TCU
- TCU is the feature to Read and Write Connection Parameters <br>
- TCU Supports @ref TOA_COMMON Codes <br>
- TCU Currently Supports the following Commands:
  - @ref TOA_COM_CMD_READ_VAL
  - @ref TOA_COM_CMD_WRITE_VAL
- TCU returns the following errors with @ref TOA_COM_RSP_ERROR
  Error | Error Number | Description
  ------|--------------|-------------
  @ref ERROR_PARAMETERS | 0x02 | Bad parameter sent to TCU command
  @ref ERROR_UNSUPPORTED | 0x01 | An unrecognized command was sent to TCU

*  @{
*/


/* default connection params */
#define TILE_CONN_INT_MIN_DEFAULT     288
#define TILE_CONN_INT_MAX_DEFAULT     304
#define TILE_CONN_SLAVE_LAT_DEFAULT   4
#define TILE_CONN_TIMEOUT_DEFAULT     600

#define TCU_RETRY_MAX 4 ///< maximum number of times to retry connection update
//#define TCU_RETRY_INTERVAL 1000 ///< interval between update retry attempts, in 10ms increments
#define TCU_HDC_THRESHOLD 400 ///< High duty cycle threshold, in 1.25ms increments
//#define TCU_HDC_TIME_LIMIT 6000 ///< Amount of time we can remain in high duty cycle, in 10ms increments

/**
 * @brief TCU Params <br>
This 8 Bytes structure is used as parameters of the following @ref TCU Commands and Responses:
  - @ref TOA_COM_CMD_WRITE_VAL
  - @ref TOA_COM_RSP_READ_VAL_OK
  - @ref TOA_COM_RSP_WRITE_VAL_OK
 */
typedef struct tcu_params_tag
{
  uint16_t min_conn_interval;   /**< Minimum Connection Interval in 1.25 ms units */
  uint16_t max_conn_interval;  /**< Maximum Connection Interval in 1.25 ms units */
  uint16_t slave_latency;       /**< Slave Latency in number of connection events */
  uint16_t conn_sup_timeout;    /**< Connection Supervision Timeout in 10 ms units */
} tcu_params_t;

/** @}*/


TOA_EXTERN_DECL(void, tcu_process_command, (const uint8_t cid, const uint8_t *data, uint8_t datalen));
TOA_EXTERN_DECL(void, tcu_params_updated, (void));
TOA_EXTERN_DECL(void, tcu_param_update_timer_handler, (void));
TOA_EXTERN_DECL(void, tcu_connected, (void));
TOA_EXTERN_DECL(void, tcu_param_update, (tcu_params_t* conn_params));



#endif
