

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

/** @file tofu.h
** @brief Tile Overtheair Firmware Upgrade
 */

#ifndef TOFU_H_
#define TOFU_H_

#include <stdint.h>
#include <stdbool.h>
#include "toa.h"
#include "crypto/hmac_sha256.h"
#include "tile_lib.h"
#include "modules/tile_tofu_module.h"


/** \defgroup TOFU
TOFU supports 2 channels:
  - A bi-directionnal control channel, using @ref TOA_CMD_TOFU_CTL Code from client to server and @ref TOA_RSP_TOFU_CTL Code from server to client.
  - uni-directional data channel, using @ref TOA_CMD_TOFU_DATA Code from the Server.

  Error codes returned by TOFU (in format of @ref TOA_FEATURE_ERROR_CODES) :
  Error code | Description
  -----------|-------------
  @ref ERROR_UNSUPPORTED | This Response is sent when an unknown or unsupported @ref TOFU_CTL_CMD Command was received <br>NB: there is no effect on TOFU states due to invalid commands
  @ref ERROR_PARAMETERS | This Response is sent when a command was received with invalid parameters
  @ref ERROR_DATA_LENGTH | This Response is sent when too much data was sent and exceeds the block_len
  @ref ERROR_INVALID_STATE | This Response is sent when a command or data was received when TOFU state does not allow it
  @ref ERROR_MEM_WRITE | This Response is sent when a write to memory fails
  @ref ERROR_MEM_READ | This Response is sent when a read to memory fails
  @ref ERROR_CRC | This Response is sent when the Block CRC check fails <br>This CRC check is run on the read back data
  @ref ERROR_CRC2 | This Response is sent when the Block CRC check fails <br>This CRC check is run on the received data
  @ref ERROR_SIGNATURE | This Response is sent when the image signature check failed
  @ref ERROR_HASH |This Response is sent when the image hash check failed
  @ref ERROR_IMAGE_HEADER |	This Response is sent when the Header of the provided fw_image is invalid
  @ref ERROR_PRODUCT_HEADER | This Response is sent when the product Header read from memory is invalid
  @ref ERROR_INVALID_SIZE |This Response is sent when the image size is invalid
  @ref ERROR_SAME_IMAGE |	This Response is sent when the image being transfered is already in memory

*  @{
*/


/**
 * @brief TOFU Control Command Codes
 */
enum TOFU_CTL_CMD
{
  TOFU_CTL_CMD_TOFU_DATA  = 0x00,
  /**< This is not actually used as a command, but is used
   * to return errors on TOFU data channel */
  TOFU_CTL_CMD_RESUME     = 0x01,
  /**< This Command is used to resume a TOFU session <br>
  Response: @ref TOFU_CTL_RSP_RESUME_READY or a TOFU_CTL_RSP_ERROR <br>

  Param        | Size          | Description
  ------------ | ------------- | -----------
  img_version  | 10 Bytes      | 8-bit US-ASCII string with no NULL termination required, format: xx.xx.xx.x
  img_size     | 4 Bytes       | Little Endian uint32_t
  */
  TOFU_CTL_CMD_EXIT       = 0x02,
  /**< This Command is used to EXIT a TOFU Session <br>
  Response: @ref TOFU_CTL_RSP_EXIT_OK or a TOFU_CTL_RSP_ERR_ <br>

  Param        | Size          | Description
  ------------ | ------------- | -----------
  @ref TOFU_EXIT_PARAM    | 1 Byte       | see @ref TOFU_EXIT_PARAM
  */
};

/**
 * @brief TOFU Control Response Codes<br>
 * TOFU Error Codes are sent by the Server in place of a positive response message.<br>
 * TOFU Errors are not recoverable, so no more TOFU data should be sent before a @ref TOFU_CTL_CMD_RESUME is sent.<br>
 * TOFU Errors are not part of normal operation and are not supposed to happen in the field, they are mostly for debugging
 */
enum TOFU_CTL_RSP
{
  TOFU_CTL_RSP_RESUME_READY = 0x01,
  /**< This Response is the successful response to @ref TOFU_CTL_CMD_RESUME command <br>
  The TOFU Client shall not send any TOFU data before TOFU_CTL_RSP_RESUME_READY is received.<br>
  The TOFU Client shall use the exact Block Size provided by the server for transmitting the FW.

  Param        | Size          | Description
  ------------ | ------------- | -----------
  block_len  | 4 Bytes   | Little Endian uint32_t. Size of a TOFU data block
  img_idx    | 4 Bytes   | Little Endian uint32_t. Index within the fw_image from which data stream is to be resumed
  */

  TOFU_CTL_RSP_BLOCK_OK     = 0x02,
  /**< This Response is the successful response when a full TOFU data block has been successfully transmitted  <br>
  NB: TOFU Data are transmitted using the @ref TOA_CMD_TOFU_DATA Command  <br>
  There are no parameters for this response.
  */

  TOFU_CTL_RSP_IMAGE_OK     = 0x03,
  /**< This Response is the successful response when a full TOFU Image has been successfully transmitted  <br>
  NB: TOFU Data is are transmitted using the @ref TOA_CMD_TOFU_DATA Command  <br>
  NB: The TOFU session is considered closed after this response.  <br>
  NB: The TOFU Server will apply the new fw_image after the link is disconnected.  <br>
  There are no parameters for this response.
  */

  TOFU_CTL_RSP_EXIT_OK      = 0x04,
  /**< This Response is the successful response to @ref TOFU_CTL_CMD_EXIT command <br>
  NB: The TOFU session is considered closed after this response.  <br>
  There are no parameters for this response.
  */

  TOFU_CTL_RSP_ERROR                    = 0x20,
  /**< This response is returned when there is a TOFU error. Please see @ref TOA_FEATURE_ERROR_CODES
  for format of these error messages */
};


/**
 * @brief TOFU EXIT command params
 */
enum TOFU_EXIT_PARAM
{
  TOFU_EXIT_PARAM_INIT  = 0x00, ///< Only Session Info are inited, Resume from last img_idx still possible after that
  TOFU_EXIT_PARAM_CLEAR = 0x01, ///< Clear everything (Session Info and Resume Context), Resume will restart from img_idx = 0 after this.
};

/** @}*/


/** \name Global TOFU defines
 *  @{
 */
#define TOFU_TRANSACTION_MAX_SIZE 15    ///< TOA Payload Max Size used by TOFU

/** @}*/


TOA_EXTERN_DECL(void, tofu_init, (void));
TOA_EXTERN_DECL(void, tofu_channel_unassigned, (uint8_t cid));
TOA_EXTERN_DECL(void, tofu_process_control_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen));
TOA_EXTERN_DECL(void, tofu_process_data, (const uint8_t cid, const uint8_t* data, uint8_t datalen));

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/** \name TOFU image header defines
 *  @{
 */
#define IMAGE_HEADER_SIGNATURE1     0x70
#define IMAGE_HEADER_SIGNATURE2     0x51
#define STATUS_INVALID_IMAGE        0x0
#define STATUS_VALID_IMAGE          0xAA
/** @}*/



/**
 * @brief TOFU Image Header
 */
typedef struct
{
  uint8_t signature[2];                 ///< Image Header Signature (magic number)
  uint8_t validflag;                    ///< Set to STATUS_VALID_IMAGE at the end of the image update
  uint8_t imageid;                      ///< used to determine which image is the newest
  uint32_t code_size;                   ///< Image size
  uint8_t hash[TOFU_HASH_LEN];          ///< Hash of the Image
  uint8_t sign[TOFU_SIGNATURE_LEN];     ///< Signature of the Image
  uint8_t version[TILE_FIRMWARE_VERSION_LEN]; ///< Version of the FW contained in the image
  uint8_t encryption;                   ///< Flag set when the Image is encrypted
  uint8_t reserved[1];                  ///< RFU

}image_header_t;

#endif  // TOFU_H_

