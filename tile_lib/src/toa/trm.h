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

 /** @file trm.h
 ** @brief Tile RSSI Measure feature
 */

#ifndef TRM_H_
#define TRM_H_

#include <stdint.h>
#include "toa.h"


/** \defgroup TRM
- TRM is the feature to Read RSSI values
- TRM Currently Supports the following Commands:
  - @ref TRM_CMD_START_NOTIFY
  - @ref TRM_CMD_STOP_NOTIFY
  - @ref TRM_CMD_READ
- TRM returns the following error codes with @ref TRM_RSP_ERROR
  Error                       | Error Number    | Description
  ----------------------------|-----------------|------------
  @ref ERROR_RESOURCE_IN_USE  | 0x12            | Resource is used by someone else
  @ref ERROR_PARAMETERS       | 0x02            | A bad parameter value has been passed to TRM
  @ref ERROR_UNSUPPORTED      | 0x01            | An unrecognized command was sent to TRM
*  @{
*/

/**
 * @brief TRM Command Codes
 */
enum TRM_CMD
{
  TRM_CMD_START_NOTIFY      = 0x01,
  /**< This Command is used by the TRM Client to request for RSSI notifications
  In response, the TRM Server will send @ref TRM_RSP_START_NOTIFY
  and starts sending notifications with RSSI values @ref TRM_RSP_NOTIFY

  The format is as follows:
  Param         | Size      | Description
  ------------- | --------- | -----------
  RSSI sample   | 1 Byte    | Number of RSSI values in 1 notification*/

  TRM_CMD_STOP_NOTIFY       = 0x02,
  /**< This Command is used by the TRM Client to request to stop
  notifying	RSSI values
  In response, the TRM Server will send @ref TRM_RSP_STOP_NOTIFY

  Format: No parameters */

  TRM_CMD_READ              = 0x03,
  /**< This Command is used by the TRM Client to request for RSSI values one time.
  In response, the TRM Server will send @ref TRM_RSP_READ
  and sends the notification with RSSI values @ref TRM_RSP_NOTIFY. Number of
  samples are specified in the first byte of the payload

  The format is as follows:
  Param         | Size      | Description
  ------------- | --------- | -----------
  RSSI sample   | 1 Byte    | Number of RSSI values in the notification*/
};

/**
 * @brief TRM Response Codes
 */
enum TRM_RSP
{
  TRM_RSP_START_NOTIFY = 0x01,
  /**< The Server sends this response to @ref TRM_CMD_START_NOTIFY
  */
  TRM_RSP_STOP_NOTIFY = 0x02,
  /**< This response is sent from the Server for @ref TRM_CMD_STOP_NOTIFY
  @ref TRM_RSP_NOTIFY is not sent after this */
  TRM_RSP_READ = 0x03,
  /**< This response is sent from the Server for @ref TRM_CMD_READ
  RSSI values are notified one time after this response*/
  TRM_RSP_NOTIFY = 0x04,
  /**< This is the notification sent to the TRM Client with RSSI values
  This Command is used by the TRM Client to request for RSSI notifications
  In response, the TRM Server will send @ref TRM_RSP_START_NOTIFY
  and starts sending notifications with RSSI values @ref TRM_RSP_NOTIFY

  The format is as follows:
  Param         | Size             | Description
  ------------- | ---------------- | -----------
  RSSI sample   | 1 Byte           | Number of as RSSI values in 1 notification
  ------------- | ---------------  | -----------
  RSSI values   | 1 to TOA_MPS - 2 | Array of RSSI values
	*/
  TRM_RSP_ERROR = 0x20,
  /**< This response is sent when an error occurs in TRM */
};

/** @}*/
TOA_EXTERN_DECL(void, trm_process_command, (const uint8_t cid, const uint8_t *data, uint8_t datalen));
TOA_EXTERN_DECL(void, trm_init, (void));

#endif
