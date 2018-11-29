/*
 * NOTICE
 *
 * © 2016 Tile Inc.  All Rights Reserved.

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


#ifndef TDT_H_
#define TDT_H_

#include "toa.h"

/** \defgroup TDT
Tile Double Tap Feature:
 - TDT supports Single, Double and Long Tap settings and Notifications.
 - TDT uses TOA_CMD_TDT Code from client to server and TOA_RSP_TDT Code from server to client.
*
* Double Tap Diagram:
* @image html TDT_DoubleTapDiagram.jpg
*
* TDT Decision Table:
* @image html TDT_DecisionTable.jpg
*  @{
*/


#define TDT_DELAY_DEFAULT 45  ///< default Double Tap Delay is 0.91 seconds
#define TDT_DELAY_OFFSET  1   ///< always add an offset of 10ms to the Double Tap Delay, this guarantees non zero values.
#define TDT_NOTIF_DEBOUNCE_DELAY_DEFAULT 0    ///< By default, TDT Notification Debouncing is disabled.
#define TDT_NOTIF_DEBOUNCE_DELAY_INCREMENT 10 ///< number of tens of miliseconds per increment (ie 10 means each increment is 100ms)

/**
 * @brief TDT Command Codes
 */
enum TDT_CMD
{
  TDT_CMD_CONFIG      = 0x01,
  /**< This Command is used to configure TDT <br>
  Response: @ref TDT_RSP_CONFIG or an ERROR <br>
  The Configuration will persist after disconnection. However, <br>
  it is recommended to always configure at connection in case the configuration is lost.<br>
  The Server Shall respond to a @ref TDT_CMD_CONFIG with a @ref TDT_RSP_CONFIG. <br>

  The format is as follows:
  Param         | Size      | Description
  ------------- | --------- | -----------
  TDT Config    | 4 Byte    | @ref tdt_config_t in Little Endian
  */

  TDT_CMD_READ_CONFIG = 0x02,
  /**< This Command is used to read TDT configuration <br>
  Response: @ref TDT_RSP_READ_CONFIG or an ERROR <br>

  Format: there is no parameter.
  */
};

/**
 * @brief TDT Response Codes
 */
enum TDT_RSP
{
  TDT_RSP_CONFIG      = 0x01,
  /**< This response is the successful response to @ref TDT_CMD_CONFIG Command. <br>
  The Response contains the actual configuration. <br>

  The format is as follows:
  Param         | Size      | Description
  ------------- | --------- | -----------
  TDT Config    | 4 Byte    | @ref tdt_config_t in Little Endian
  */

  TDT_RSP_NOTIFY      = 0x02,
  /**< Notifies that a Tap was detected. <br>

  The format is as follows:

  Param       | Size      | Description
  ----------- | --------- | -----------
  Tap Type    | 1 Byte    | @ref TDT_TAP_TYPE
  */

  TDT_RSP_READ_CONFIG = 0x03,
  /**< This response is the successful response to @ref TDT_CMD_READ_CONFIG Command. <br>

  The format is as follows:
  Param         | Size      | Description
  ------------- | --------- | -----------
  TDT Config    | 4 Byte    | @ref tdt_config_t in Little Endian
  */

  TDT_RSP_ERROR_UNSUPPORTED = 0x10,
  /**< This error is send by the TDT Server when an unsupported TDT_CMD is received. <br>
  */
  TDT_RSP_ERROR_PARAMS      = 0x11,
  /**< This error is send by the TDT Server when a TDT_CMD with bad parameters is received. <br>
  */
};

/**
 * @brief TDT_TAP_TYPE enum
 */
enum TDT_TAP_TYPE
{
  TDT_NOTIFY_STI  = 0x00, // SingleTap Immediate
  TDT_NOTIFY_STD  = 0x01, // SingleTap Delayed
  TDT_NOTIFY_DT   = 0x02, // DoubleTap
  TDT_NOTIFY_LT   = 0x03, // LongTap
};

/**
 * @brief TDT Local Config Struct
 */
//typedef struct
//{
//  uint16_t  SE_LTF:1;           ///< [0] Song Enable: LongTap Failure
//  uint16_t  SE_LTS:1;           ///< [1] Song Enable: LongTap Success
//  uint16_t  SE_DTF:1;           ///< [2] Song Enable: DoubleTap Failure
//  uint16_t  SE_DTS:1;           ///< [3] Song Enable: DoubleTap Success
//  uint16_t  SE_STIF:1;          ///< [4] Song Enable: SingleTapImmediate Failure
//  uint16_t  SE_STIS:1;          ///< [5] Song Enable: SingleTapImmediate Success
//  uint16_t  SE_STDF:1;          ///< [6] Song Enable: SingleTapDelayed Failure
//  uint16_t  SE_STDS:1;          ///< [7] Song Enable: SingleTapDelayed Success
//  uint16_t  EN_DT:1;            ///< [8] Enable: DoubleTap
//  uint16_t  EN_LT:1;            ///< [9] Enable: LongTap
//  uint16_t  EN_STI:1;           ///< [10] Enable: SingleTapImmediate
//  uint16_t  EN_STD:1;           ///< [11] Enable: SingleTapDelayed
//  uint16_t  SS_Strength:2;      ///< [12:13] Success Song Strength (0/1: Low; 2: Med; 3: High)
//  uint16_t  FS_Strength:2;      ///< [14:15] Fail Song Strength (0/1: Low; 2: Med; 3: High)
//
//  uint8_t   Delay;              ///< DoubleTap and LongTap detection delay: in units of 20 ms, plus an offset of 10ms.
//  uint8_t   NotifDebounceDelay; ///< DoubleTap Notification Debouncing Delay: in units of 100ms. 0 means no debouncing.
//
//} tdt_config_t;

/** @}*/


TOA_EXTERN_DECL(void, tdt_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen));
TOA_EXTERN_DECL(void, tdt_process_button_press, (void));
TOA_EXTERN_DECL(void, tdt_init, (void));
TOA_EXTERN_DECL(void, tdt_process_tap, (uint8_t type));
TOA_EXTERN_DECL(void, tdt_DT_timer_handler, (void));
TOA_EXTERN_DECL(void, tdt_LT_timer_handler, (void));
TOA_EXTERN_DECL(void, tdt_STD_timer_handler, (void));
TOA_EXTERN_DECL(void, tdt_STI_timer_handler, (void));
TOA_EXTERN_DECL(void, tdt_doubletap_timer_handler, (void));
TOA_EXTERN_DECL(void, tdt_HDC_timer_handler, (void));

#endif  // TDT_H_

