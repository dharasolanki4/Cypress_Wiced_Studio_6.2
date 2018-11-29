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

#ifndef TMF_H_
#define TMF_H_


#include <stdint.h>
#include <stdbool.h>
#include "toa.h"

/** \defgroup TMF
Tile ManuFacturing:
  Tile ManuFacturing is a feature for manufacturing tests and programming information to the Tile.
  TMF is available on the connectionless TOA channel only in Manufacturing Mode.
  Information available to write:
  - Tile ID
  - Tile Auth Key
  - Model number
  - Hardware version
  - MAC address

  Error codes returned by TMF (in format of @ref TOA_FEATURE_ERROR_CODES) :
  Error code | Error Number | Description
  -----------|--------------|-------------
  @ref ERROR_UNSUPPORTED | 0x01 | This Response is sent when an unknown or unsupported @ref TMF_CMD Command was received <br>
  @ref ERROR_PARAMETERS | 0x02 | This Response is sent when a command was received with invalid parameters
*  @{
*/


/**
 * @brief TMF Command Codes
 */
enum TMF_CMD
{
  TMF_CMD_WRITE_TILE_ID = 0x01,
  /**< This Command is used to write the Tile ID.<br>
  Response: @ref TMF_RSP_WRITE_TILE_ID
  Format:
  @ref TMF_CMD_WRITE_TILE_ID | Tile ID
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_CMD_WRITE_TILE_AUTH_KEY_START = 0x02,
  /**< This Command is used to write the first half of the TileAuthKey.<br>
  Response: @ref TMF_RSP_WRITE_TILE_AUTH_KEY_START
  Format:
  @ref TMF_CMD_WRITE_TILE_AUTH_KEY_START | TileAuthKey First Half
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_CMD_WRITE_TILE_AUTH_KEY_END = 0x03,
  /**< This Command is used to write the second half of the TileAuthKey.<br>
  Response: @ref TMF_RSP_WRITE_TILE_AUTH_KEY_END
  Format:
  @ref TMF_CMD_WRITE_TILE_AUTH_KEY_END | TileAuthKey Second Half
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_CMD_WRITE_MODEL_NUMBER = 0x04,
  /**< This Command is used to write the model number.<br>
  Response: @ref TMF_RSP_WRITE_MODEL_NUMBER
  Format:
  @ref TMF_CMD_WRITE_MODEL_NUMBER | Model Number
  ------------------------- | -------
  1 byte                    | 10 ASCII Characters (no NULL termination)
   */

  TMF_CMD_WRITE_HARDWARE_VERSION = 0x05,
  /**< This Command is used to write the hardware revision.<br>
  Response: @ref TMF_RSP_WRITE_HARDWARE_VERSION
  Format:
  @ref TMF_CMD_WRITE_HARDWARE_VERSION | Hardware Version
  ------------------------- | -------
  1 byte                    | 5 ASCII Characters (no NULL termination)
   */

  TMF_CMD_WRITE_BDADDR = 0x06,
  /**< This Command is used to write the Bluetooth MAC address.<br>
  Response: @ref TMF_RSP_WRITE_BDADDR
  Format:
  @ref TMF_CMD_WRITE_BDADDR | Bdaddr
  ------------------------- | -------
  1 byte                    | 6 Bytes
   */

  TMF_CMD_SHIP = 0x07,
  /**< This Command is used when everything is ready to ship.<br>
  The Manufacturing Information needs to have been written at this point (and verified).
  After this command has been sent, the Tile will be locked and Manufacturing Information cannot be changed.
  After receiving this command, the Tile will be set to Shipping Mode.
  The command will return ERROR_CODE ERROR_INVALID_DATA if not all MFG data was written.
  Response: @ref TMF_RSP_WRITE_BDADDR
   */

  TMF_CMD_PLAY_SONG = 0x08,
  /**< This Command is used to play a song at the specified strength .<br>
  @ref TMF_CMD_PLAY_SONG | Song Number (0xFF for Stop) | Strength
  ---------------------- | ------------| ---------
  1 byte                 | 1 Byte      | 1 Byte
  Response: @ref TMF_RSP_PLAY_SONG
   */

  TMF_CMD_READ_BATTERY_LEVEL = 0x09,
  /**< This Command is used to read the battery blevel .<br>
  There is no parameter for this command.
   */

  TMF_CMD_READ_LOADED_BATTERY_LEVEL = 0x0a,
  /**< This Command is used to read the loaded battery blevel .<br>
  There is no parameter for this command.
   */

  TMF_CMD_READ_CHIP_DATA = 0x0b,
  /**< This Command is used to read the chip data .<br>
  There is no parameter for this command.
   */

  TMF_CMD_READ_TILE_AUTH_KEY_START = 0x0c,
  /**< This Command is used to write the first half of the TileAuthKey.<br>
  Response: @ref TMF_RSP_READ_TILE_AUTH_KEY_START
  There is no parameter for this command.
   */

  TMF_CMD_READ_TILE_AUTH_KEY_END = 0x0d,
  /**< This Command is used to write the second half of the TileAuthKey.<br>
  Response: @ref TMF_RSP_READ_TILE_AUTH_KEY_END
  There is no parameter for this command.
   */

  TMF_CMD_READ_TILE_MODE = 0x0e,
  /**< This Command is used to read Tile Mode.<br>
  Response: @ref TMF_RSP_READ_TILE_MODE
  There is no parameter for this command.
   */

  TMF_CMD_UNDO_SHIP = 0x0f,
  /**< This Command is used to revert the Tile back to to manufacturing from shipping mode.<br>
  The parameter is a 13 byte key.
  @ref TMF_CMD_UNDO_SHIP | Key
  ---------------------- | ------------
  1 byte                 | 13 Bytes     
  Response: @ref TMF_RSP_UNDO_SHIP
   */
};

/**
 * @brief TMF Response Codes<br>
 */
enum TMF_RSP
{
  TMF_RSP_WRITE_TILE_ID = 0x01,
  /**< This response contains the Tile ID.<br>
  Format:
  @ref TMF_RSP_WRITE_TILE_ID | Tile ID
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_RSP_WRITE_TILE_AUTH_KEY_START = 0x02,
  /**< This response contains the first half of the TileAuthKey.<br>
  Format:
  @ref TMF_RSP_WRITE_TILE_AUTH_KEY_START | First Half of Tile Auth Key
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_RSP_WRITE_TILE_AUTH_KEY_END = 0x03,
  /**< This response contains the second half of the TileAuthKey.<br>
  Format:
  @ref TMF_RSP_WRITE_TILE_AUTH_KEY_END | Second Half of Tile Auth Key
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_RSP_WRITE_MODEL_NUMBER = 0x04,
  /**< This response contains the model number in the format "<4-character name> XX.XX".<br>
  Format:
  @ref TMF_RSP_WRITE_MODEL_NUMBER | Number
  ------------------------------ | ------
  1 byte                         | 10 bytes
   */

  TMF_RSP_WRITE_HARDWARE_VERSION = 0x05,
  /**< This response contains the hardware version in the format "XX.XX".<br>
  Format:
  @ref TMF_RSP_WRITE_HARDWARE_VERSION | Version
  ---------------------------------- | -------
  1 byte                             | 5 bytes
   */

  TMF_RSP_WRITE_BDADDR = 0x06,
  /**< This response contains the Bluetooth MAC address for the device.<br>
  Format:
  @ref TMF_RSP_WRITE_BDADDR | Address
  ------------------------ | -------
  1 byte                   | 6 bytes
   */

  TMF_RSP_SHIP = 0x07,
  /**< This response returns a 1 Byte error code (0 = success).<br>
  Format:
  @ref TMF_RSP_SHIP | Error_Code
  ----------------- | -------
  1 byte            | 1 byte
   */

  TMF_RSP_PLAY_SONG = 0x08,
  /**< This response returns no <br>
  Format:
  @ref TMF_RSP_PLAY_SONG | Error_Code
  -----------------------| -------
  1 byte                 | 1 byte
   */

  TMF_RSP_READ_BATTERY_LEVEL = 0x09,
  /**< This Response returns the battery blevel .<br>
  Format:
  @ref TMF_RSP_READ_BATTERY_LEVEL | Battery Level
  -----------------------| -------
  1 byte                 | 1 byte
   */

  TMF_RSP_READ_LOADED_BATTERY_LEVEL = 0x0a,
  /**< This Response returns the loaded battery blevel .<br>
  Format:
  @ref TMF_RSP_READ_LOADED_BATTERY_LEVEL | Battery Level
  -----------------------| -------
  1 byte                 | 1 byte
   */

  TMF_RSP_READ_CHIP_DATA_CONT = 0x0b,
  /**< This Response returns the chip data .<br>
  It also means there is more data to come
  Format:
  @ref TMF_RSP_READ_CHIP_DATA | Chip Data
  -----------------------| -------
  1 byte                 | TOA_MPS-1
   */

  TMF_RSP_READ_CHIP_DATA_END = 0x0c,
  /**< This Response returns the last portion of chip data .<br>
  Format:
  @ref TMF_RSP_READ_CHIP_DATA | Chip Data
  -----------------------| -------
  1 byte                 | varies
   */

  TMF_RSP_READ_TILE_AUTH_KEY_START = 0x0d,
  /**< This response contains the first half of the TileAuthKey.<br>
  Format:
  @ref TMF_RSP_READ_TILE_AUTH_KEY_START | First Half of Tile Auth Key
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_RSP_READ_TILE_AUTH_KEY_END = 0x0e,
  /**< This response contains the second half of the TileAuthKey.<br>
  Format:
  @ref TMF_RSP_READ_TILE_AUTH_KEY_END | Second Half of Tile Auth Key
  ------------------------- | -------
  1 byte                    | 8 bytes
   */

  TMF_RSP_READ_TILE_MODE = 0x0f,
  /**< This response contains the Tile Mode.<br>
  Format:
  @ref TMF_RSP_READ_TILE_MODE | Tile Mode
  ------------------------- | -------
  1 byte                    | 1 byte
   */

  TMF_RSP_UNDO_SHIP = 0x10,
  /**< This response contains the Tile Mode.<br>
  Format:
  @ref TMF_RSP_UNDO_SHIP | Tile Mode
  -----------------------| -------
  1 byte                 | 1 byte
   */

  TMF_RSP_ERROR = 0x20,
  /**< This response is returned when there is a TMF error. Please see @ref TOA_FEATURE_ERROR_CODES
  for format of these error messages */
};


/** @}*/

TOA_EXTERN_DECL(void, tmf_process_command, (const uint8_t *token, const uint8_t *data, uint8_t datalen));

#endif
