/*
 * NOTICE
 *
 * © 2017 Tile Inc.  All Rights Reserved.

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

 /** @file tpc.h
 ** @brief Tile Power Control
 */

#ifndef TPC_H_
#define TPC_H_

#include <stdint.h>
#include "toa.h"


/** \defgroup TPC
- TPC (Tile Power Control) is the feature to allow the Tile to control its transmit and/or receive
  power based on RSSI measurements.
- TPC returns the following error codes with @ref TPC_RSP_ERROR
  Error | Error Number | Description
  ------|--------------|------------
  @ref ERROR_PARAMETERS | 0x02 | A bad parameter value has been passed to TPC
  @ref ERROR_UNSUPPORTED | 0x01 | An unrecognized command was sent to TPC
*  @{
*/

/**
 * @brief TPC command codes
 */
enum TPC_CMD {
  TPC_CMD_READ_CONFIG  = 0x01,
  /**< Read TPC configuration. No format. */
  TPC_CMD_WRITE_CONFIG = 0x02,
  /**< Write TPC configuration. Format:
      @ref TPC_CMD_WRITE_CONFIG | @ref tpc_config_t
      ------------------------- | -----------------
      1 byte                    | 4 bytes
  */

  TPC_CMD_SET_TX_LIMIT = 0x03,
  /**< Limit the transmit power for advertising and connected states.

      Values are given in tenths of a dBm. 10 indicates 1 dBm.
      Format:
      @ref TPC_CMD_SET_TX_LIMIT | Adv tx lim | Con tx lim
      ------------------------- | ---------- | ----------
      1 byte                    | int16_t    | int16_t
  */

  TPC_CMD_READ_TX_LIMIT = 0x04,
  /**< Read back the tx power limits. No format. */

  TPC_CMD_READ_TX_MIN_MAX = 0x05,
  /**< Read back the minimum and maximum tx power. No format */
};

/**
 * @brief TPC response codes
 */
enum TPC_RSP {
  TPC_RSP_READ_CONFIG  = 0x01,
  /**< Read TPC configuration response. Format:
      @ref TPC_RSP_READ_CONFIG | @ref tpc_config_t
      ------------------------ | -----------------
      1 byte                   | 4 bytes
  */
  TPC_RSP_WRITE_CONFIG = 0x02,
  /**< TPC configuration written successfully. Format:
      @ref TPC_RSP_WRITE_CONFIG | @ref tpc_config_t
      ------------------------- | -----------------
      1 byte                    | 4 bytes
  */
  TPC_RSP_SET_TX_LIMIT = 0x03,
  /**< TX limits set successfully. Returns the actual
       adv and con tx power.

       Values are given in tenths of a dBm. 10 indicates 1 dBm.
       Format:
       @ref TPC_RSP_SET_TX_LIMIT | Adv tx pwr | Con tx pwr
       ------------------------- | ---------- | ----------
       1 byte                    | int16_t    | int16_t
  */

  TPC_RSP_READ_TX_LIMIT = 0x04,
  /**< Read back the transmit power limits, current transmit power,
       and minimum transmit power.

       Values are given in tenths of a dBm. 10 indicates 1 dBm.
       Format:
       @ref TPC_CMD_READ_TX_LIMIT | Adv tx lim | Con tx lim | Adv tx pwr | Con tx pwr | Min tx pwr
       -------------------------- | ---------- | ---------- | ---------- | ---------- | ----------
       1 byte                     | int16_t    | int16_t    | int16_t    | int16_t    | int16_t
  */

  TPC_RSP_READ_TX_MIN_MAX = 0x05,
  /**< Read back the min and max tx power.

       Values are given in tenths of a dBm. 10 indicates 1 dBm.
       Format:
       @ref TPC_RSP_READ_TX_MIN_MAX | Min tx pwr | Max tx pwr
       ---------------------------- | ---------- | ----------
       1 byte                       | int16_t    | int16_t
  */

  TPC_RSP_ERROR        = 0x20,
  /**< Error. Uses @ref TOA_FEATURE_ERROR_CODES format. */
};

/** @}*/

TOA_EXTERN_DECL(void, tpc_process_command, (const uint8_t cid, const uint8_t *data, uint8_t datalen));

#endif
