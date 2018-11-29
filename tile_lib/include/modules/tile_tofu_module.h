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

/** @file tile_tofu_module.h
 ** @brief Tile Over-the-air Firmware Update module
 */

#ifndef TILE_TOFU_MODULE_H_
#define TILE_TOFU_MODULE_H_

#include <stdint.h>
#include <stdbool.h>
#include "tile_lib.h"
#include "crypto/hmac_sha256.h"

#define TOFU_HASH_LEN             32    ///< Size of the TOFU Image Hash
#define TOFU_SIGNATURE_LEN        64    ///< Size of the TOFU Image Signature
#define TOFU_CRC16_LEN            2     ///< Size of the Block CRC
#define TOFU_IMAGE_HEADER_SIZE    116   ///< Size of the image header

/**
 * @enum TOFU State
 */
enum TOFU_STATE
{
  TOFU_STATE_IDLE = 0x00, ///< TOFU has not been started
  TOFU_STATE_RX   = 0x01, ///< TOFU Server is accepting data
  TOFU_STATE_MEM  = 0x02, ///< TOFU Server is processing/saving data and is NOT accepting data
};


/**
 * @brief TOFU State vars
 */
typedef struct
{
  uint8_t     state;                                    ///< TOFU state, see @ref TOFU_STATE
  uint8_t     cached_cid;                               ///< current actively-TOFU'ing CID
  uint8_t     img_ok;                                   ///< a successfull TOFU happened already
  uint8_t     img_version[TILE_FIRMWARE_VERSION_LEN];   ///< version of FW image being uploaded
  uint8_t     img_hash[TOFU_HASH_LEN];                  ///< hash from the image header (part of Resume Context)
  uint8_t     img_signature[TOFU_SIGNATURE_LEN];        ///< signature from the image header (part of Resume Context)
  uint32_t    img_idx;                                  ///< accumulated number of bytes written for current image (part of Resume Context)
  uint32_t    img_len;                                  ///< length of the full image (part of Resume Context)
  sha256_ctx  img_hash_ctx;                             ///< SHA256 context of current image being TOFUed.
  uint32_t    block_idx;                                ///< accumulated number of bytes received for current datablock
  uint32_t    block_len;                                ///< lengh of next block (indicated by initiator)

}tofu_state_t;


struct tile_tofu_module {
  /**
   * Public key used for ECC signature verification
   * Generating ECC keys:
   * $ openssl ecparam -genkey -name secp256k1 -out k.perm
   * $ openssl ec -outform DER -in k.perm -noout -text
   */
  uint8_t *pub_key;

  /**
   * Buffer for storing chunks as they are received
   */
  uint8_t *block;

  /**
   * Also the size of each block sent over the air. Must be less than or equal
   * to the size of the block buffer.
   */
  uint16_t block_len;

  /**
   * Internal state used by TOFU
   */
  tofu_state_t state;

  /**
   * TOFU is starting (a resume command was received)
   * @return @TOA_FEATURE_ERROR_CODES error code
   */
  uint8_t (*begin)(void);

  /**
   * A TOFU block has been received. Write to nonvolatile storage.
   */
  void (*block_ready)(void);

  /**
   * TOFU has completed successfully
   */
  void (*complete)(void);
  
  /**
   * Callback to allow platform specific actions be taken during an ECC calculation
   */
  void (*ecc_loop_cb)(int numbits, int index);
};


/**
 * @brief Register the TOFU module.
 *
 * @param[in] a pointer to the TOFU module struct
 */
int tile_tofu_register(struct tile_tofu_module *module);

/**
 * @brief Application has finished processing a complete TOFU Image
 *
 * @param[in] @TOA_FEATURE_ERROR_CODES error code
 */
void tofu_block_done(uint8_t code);

/**
 * @brief Application has finished processing a complete TOFU Image
 *
 * @param[in] @TOA_FEATURE_ERROR_CODES error code
 */
void tofu_complete_done(uint8_t error);


#endif
