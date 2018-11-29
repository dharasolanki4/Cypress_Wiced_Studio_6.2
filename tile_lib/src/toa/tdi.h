/*
 * NOTICE
 *
 * (c) 2016 Tile Inc.  All Rights Reserved.

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

#ifndef TDI_H_
#define TDI_H_


#include <stdint.h>
#include <stdbool.h>

/** \defgroup TDI
Tile Device Information:
  Tile Device Information is a feature for reading information from the Tile. TDI is available
  on the connectionless TOA channel. Information available to read:
  - Tile ID
  - Firmware version
  - Model number
  - Hardware version
  - MAC address

  Error codes returned by TDI (in format of @ref TOA_FEATURE_ERROR_CODES) :
  Error code | Error Number | Description
  -----------|--------------|-------------
  @ref ERROR_UNSUPPORTED | 0x01 | This Response is sent when an unknown or unsupported @ref TDI_CMD Command was received <br>
  @ref ERROR_PARAMETERS | 0x02 | This Response is sent when a command was received with invalid parameters
*  @{
*/


/**
 * @brief TDI Command Codes
 */
enum TDI_CMD
{
  TDI_CMD_READ_AVAILABLE_INFO = 0x01,
  /**< This Command is used to read which information is available over TDI.<br>
  Response: @ref TDI_RSP_READ_AVAILABLE_INFO
   */

  TDI_CMD_READ_TILE_ID = 0x02,
  /**< This Command is used to read the Tile ID.<br>
  Response: @ref TDI_RSP_READ_TILE_ID
   */

  TDI_CMD_READ_FIRMWARE_VERSION = 0x03,
  /**< This Command is used to read the firmware version.<br>
  Response: @ref TDI_RSP_READ_FIRMWARE_VERSION
   */

  TDI_CMD_READ_MODEL_NUMBER = 0x04,
  /**< This Command is used to read the model number.<br>
  Response: @ref TDI_RSP_READ_MODEL_NUMBER
   */

  TDI_CMD_READ_HARDWARE_VERSION = 0x05,
  /**< This Command is used to read the hardware revision.<br>
  Response: @ref TDI_RSP_READ_HARDWARE_VERSION
   */

  TDI_CMD_READ_BDADDR = 0x06,
  /**< This Command is used to read the Bluetooth MAC address.<br>
  Response: @ref TDI_RSP_READ_BDADDR
   */
};

/**
 * @brief TDI Response Codes<br>
 */
enum TDI_RSP
{
  TDI_RSP_READ_AVAILABLE_INFO = 0x01,
  /**< This response is a bitfield showing which information can be read.<br>
  Format:
  @ref TDI_RSP_READ_AVAILABLE_INFO | Available info
  -------------------------------- | --------------
  1 byte                           | Variable

  Info bitfield:
  - Byte 1
     - Bit 0: Tile ID
     - Bit 1: Firmware Version
     - Bit 2: Model Number
     - Bit 3: Hardware Version
     - Bit 4: Bluetooth MAC Address
   */

  TDI_RSP_READ_TILE_ID = 0x02,
  /**< This response contains the Tile ID.<br>
  Format:
  @ref TDI_RSP_READ_TILE_ID | Tile ID
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TDI_RSP_READ_FIRMWARE_VERSION = 0x03,
  /**< This response contains the firmware version number in the format "XX.XX.XX.X".<br>
  Format:
  @ref TDI_RSP_READ_FIRMWARE_VERSION | Version
  ---------------------------------- | -------
  1 byte                             | 10 bytes
   */

  TDI_RSP_READ_MODEL_NUMBER = 0x04,
  /**< This response contains the model number in the format "<4-character name> XX.XX".<br>
  Format:
  @ref TDI_RSP_READ_MODEL_NUMBER | Number
  ------------------------------ | ------
  1 byte                         | 10 bytes
   */

  TDI_RSP_READ_HARDWARE_VERSION = 0x05,
  /**< This response contains the hardware version in the format "XX.XX".<br>
  Format:
  @ref TDI_RSP_READ_HARDWARE_VERSION | Version
  ---------------------------------- | -------
  1 byte                             | 5 bytes
   */

  TDI_RSP_READ_BDADDR = 0x06,
  /**< This response contains the Bluetooth MAC address for the device.<br>
  Format:
  @ref TDI_RSP_READ_BDADDR | Address
  ------------------------ | -------
  1 byte                   | 6 bytes
   */


  TDI_RSP_ERROR = 0x20,
  /**< This response is returned when there is a TDI error. Please see @ref TOA_FEATURE_ERROR_CODES
  for format of these error messages */
};


/** @}*/

void tdi_process_command(const uint8_t *token, const uint8_t *data, uint8_t datalen);

#endif
