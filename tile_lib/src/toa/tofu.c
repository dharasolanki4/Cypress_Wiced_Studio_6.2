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

/** @file tofu.c
 ** @brief Tile Overtheair Firmware Upgrade
 */

#include "tofu.h"
#include "toa.h"

#include "../crc.h"
#include "../tileHash.h"
#include "tile_lib.h"
#include "modules/tile_tofu_module.h"
#include "modules/tile_tdi_module.h"
#include <string.h>

extern struct tile_tdi_module *tdi;

struct tile_tofu_module *tofu;


static void tofu_process_block(const uint8_t cid);
static void tofu_process_command_resume(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void tofu_send_response(const uint8_t cid, uint8_t command, const uint8_t* data, uint8_t datalen);
static void tofu_send_response_resume(const uint8_t cid);
static void tofu_send_response_error(const uint8_t cid, uint8_t error, uint8_t command);
static void tofu_block_terminal_error(uint8_t status);
static int  tofu_process_first_block(uint8_t* data, uint32_t data_len);
static void tofu_clear(void);


/**
 ****************************************************************************************
 * @brief process incomming TOFU Control Commands
 *
 * @param[in] cid        TOA CID that was unassigned to TOFU.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 * @return    void
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tofu_process_control_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  if(TOA_CONNECTIONLESS_CID != tofu->state.cached_cid
    && cid != tofu->state.cached_cid)
  {
    /* Someone else is running a TOFU */
    tofu_send_response_error(cid, TOA_ERROR_RESOURCE_IN_USE, data[0]);
    return;
  }

  switch(data[0])
  {
    case TOFU_CTL_CMD_RESUME:
      tofu_process_command_resume(cid, &data[1], datalen-1);
    break;

    case TOFU_CTL_CMD_EXIT:
      if(datalen < 2)
      {
        tofu_send_response_error(cid, TOA_ERROR_PARAMETERS, TOFU_CTL_CMD_EXIT);
      }
      else if(TOFU_EXIT_PARAM_INIT == data[1])
      {
        /* Init Session Info, Resume is still possible after that */
        tofu_init();
        tofu_send_response(cid, TOFU_CTL_RSP_EXIT_OK, NULL, 0);
      }
      else if(TOFU_EXIT_PARAM_CLEAR == data[1])
      {
        /* Clear everything, Resume will restart from scratch */
        tofu_clear();
        tofu_send_response(cid, TOFU_CTL_RSP_EXIT_OK, NULL, 0);
      }
      else
      {
        tofu_send_response_error(cid, TOA_ERROR_PARAMETERS, TOFU_CTL_CMD_EXIT);
      }
    break;

    default:
      tofu_send_response_error(cid, TOA_ERROR_UNSUPPORTED, data[0]);
    break;
  }
}

/**
 ****************************************************************************************
 * @brief process incomming TOFU Data packets
 *
 * @param[in] cid        TOA CID that was unassigned to TOFU.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 * @return    void
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tofu_process_data, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  if(cid != tofu->state.cached_cid)
  {
    tofu_send_response_error(cid, TOA_ERROR_RESOURCE_IN_USE, TOFU_CTL_CMD_TOFU_DATA);
    return;
  }

  if((TOFU_STATE_RX != tofu->state.state) || (true == tofu->state.img_ok))
  {
    tofu_send_response_error(cid, TOA_ERROR_INVALID_STATE, TOFU_CTL_CMD_TOFU_DATA);
    return;
  }

  if(   ((tofu->state.block_idx + datalen) > (tofu->state.block_len + TOFU_CRC16_LEN))
    ||  ((tofu->state.img_idx + tofu->state.block_idx + datalen) > (tofu->state.img_len + TOFU_CRC16_LEN)) )
  {
    /* check if block data exceeds max or if image data received exceeds image size */
    tofu_send_response_error(cid, TOA_ERROR_DATA_LENGTH, TOFU_CTL_CMD_TOFU_DATA);
    tofu_clear();
    return;
  }

  /* add new data to the block buffer and move index */
  memcpy(&tofu->block[tofu->state.block_idx], data, datalen);
  tofu->state.block_idx += datalen;

  if((tofu->state.img_idx + tofu->state.block_idx) == (tofu->state.img_len + TOFU_CRC16_LEN))
  {
    /* full image has been received */
    tofu_process_block(cid);
  }
  else if((tofu->state.block_len+TOFU_CRC16_LEN) == tofu->state.block_idx)
  {
    /* whole block has been received, process it */
    tofu_process_block(cid);
  }
  else
  {
    /* just wait, more data to receive */
  }
}

/**
 ****************************************************************************************
 * @brief process Resume Command
 *
 * @param[in] cid        TOA CID that was unassigned to TOFU.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void tofu_process_command_resume(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  if((TOFU_STATE_MEM == tofu->state.state) || (true == tofu->state.img_ok))
  {
    tofu_send_response_error(cid, TOA_ERROR_INVALID_STATE, TOFU_CTL_CMD_RESUME);
    return;
  }

  if(datalen < (TILE_FIRMWARE_VERSION_LEN+3))
  {
    tofu_send_response_error(cid, TOA_ERROR_PARAMETERS, TOFU_CTL_CMD_RESUME);
    return;
  }

  if(0 == memcmp(tdi->firmware_version, data, TILE_FIRMWARE_VERSION_LEN))
  {
    /* Same image as the booted one, reject */
    tofu_send_response_error(cid, TOA_ERROR_SAME_IMAGE, TOFU_CTL_CMD_RESUME);
    return;
  }

  uint32_t img_len = 0; // set all 4 bytes to 0
  memcpy(&img_len, &data[TILE_FIRMWARE_VERSION_LEN], 3);

  if(img_len < 1000)
  {
    /* Catch very small images error, probably caused by a failure to read the file */
    /* The limit is very arbitrary and can be anything */
    tofu_send_response_error(cid, TOA_ERROR_INVALID_SIZE, TOFU_CTL_CMD_RESUME);
  }
  else if(   (0 == tofu->state.img_idx)
    ||  (0 != (tofu->state.img_idx%tofu->block_len))
    ||  (0 != memcmp(tofu->state.img_version, data, TILE_FIRMWARE_VERSION_LEN))
    ||  (img_len != tofu->state.img_len))
  {
    /* start from 0 */
    tofu_clear();
    /* save new FW info */
    memcpy(tofu->state.img_version, data, TILE_FIRMWARE_VERSION_LEN);
    tofu->state.img_len = img_len;
    /* init hash calculation */
    sha256_init_tile(&tofu->state.img_hash_ctx);
    uint8_t res = tofu->begin();
    
    if(TOA_ERROR_OK != res)
    {
      tofu_send_response_error(cid, res, TOFU_CTL_CMD_RESUME);
      return;
    }
  }
  else
  {
    /* We have something started already and is same FW version and same length: resume */
    tofu_init();
  }

  tofu->state.cached_cid = cid;
  tofu->state.state = TOFU_STATE_RX;
  tofu_send_response_resume(cid);
}


/**
 ****************************************************************************************
 * @brief TOFU image block handler. Validates image block and stores it to
 *        external memory device.
 *
 * @param[in]   cid the TOA cid using TOFU
 *
 * @return      void
 *
 ****************************************************************************************
 */
static void tofu_process_block(const uint8_t cid)
{
  tofu->state.state = TOFU_STATE_MEM;
  int ret;

  if (tofu->state.img_len < ( tofu->state.img_idx + tofu->state.block_idx - TOFU_CRC16_LEN ))
  {
    tofu_block_terminal_error(TOA_ERROR_DATA_LENGTH);
    return;
  }
  if(0 != crc16(0, tofu->block, tofu->state.block_idx))
  {
    tofu_block_terminal_error(TOA_ERROR_CRC2);
    return;
  }

  tofu->state.block_idx -= TOFU_CRC16_LEN;

  // When the first block is received, read image header first
  if( tofu->state.block_idx != 0 && tofu->state.img_idx == 0 )
  {
    ret = tofu_process_first_block(&tofu->block[0], tofu->state.block_idx );
    if(TOA_ERROR_OK != ret)
    {
      tofu_block_terminal_error(ret);
      return;
    }
  }
  else
  {
    /* Update Hash calculation with new data */
    sha256_update_tile(&tofu->state.img_hash_ctx, tofu->block, tofu->state.block_idx);
  }

  tofu->block_ready();
}


/**
 ****************************************************************************************
 * @brief This function is called when the first TOFU block is received.
 *        Firstly, the image header is extracted from the first block, then the external memmory
 *        is checked to determine where to write this new image and finaly the header and the first
 *        image data are written to external memory.
 *
 * @param[in]   data:           Points to the first data block received over TOFU
 *              data_len:       Length of the data block
 *
 * @return      0 for success, otherwise error codes
 *
 ****************************************************************************************
 */
static int tofu_process_first_block(uint8_t* data, uint32_t data_len)
{
  image_header_t* pReceivedFwHeader = (image_header_t*)data;
  uint32_t  codesize;

  /* perform CRC check of the block first */
  if(0 != crc16(0, data, data_len+TOFU_CRC16_LEN))
  {
    return TOA_ERROR_CRC2;
  }

  if( data_len < sizeof(image_header_t) )
  {
    // block size should be at least image header size
    return TOA_ERROR_DATA_LENGTH;
  }

  // check firmware header
  if( (pReceivedFwHeader->signature[0] != IMAGE_HEADER_SIGNATURE1) || (pReceivedFwHeader->signature[1] != IMAGE_HEADER_SIGNATURE2) )
  {
    return TOA_ERROR_IMAGE_HEADER;
  }

  // Get code size
  codesize = pReceivedFwHeader->code_size;

  /* extra check Image Size */
  if(tofu->state.img_len != codesize+sizeof(image_header_t))
  {
    return TOA_ERROR_INVALID_SIZE;
  }

  /* Verify the FW Version matches what we got from CMD_TOFU_RESUME */
  if(0 != memcmp(tofu->state.img_version, pReceivedFwHeader->version, TILE_FIRMWARE_VERSION_LEN))
  {
    return TOA_ERROR_IMAGE_HEADER;
  }

  /* Retain signature and hash */
  memcpy(tofu->state.img_signature, pReceivedFwHeader->sign, TOFU_SIGNATURE_LEN);
  memcpy(tofu->state.img_hash, pReceivedFwHeader->hash, TOFU_HASH_LEN);

  if (!codesize)
  {
      return TOA_ERROR_INVALID_SIZE;
  }

  /* Update Hash calculation with important header values */
  sha256_update_tile(&tofu->state.img_hash_ctx, (uint8_t*)&pReceivedFwHeader->code_size, 4);
  sha256_update_tile(&tofu->state.img_hash_ctx, &pReceivedFwHeader->version[0], TILE_FIRMWARE_VERSION_LEN);

  /* Add hash for the rest of the block */
  sha256_update_tile(&tofu->state.img_hash_ctx, &data[sizeof(image_header_t)], data_len - sizeof(image_header_t));

  return TOA_ERROR_OK;
}


/**
 ****************************************************************************************
 * @brief send TOFU Resume Response
 *
 * @param[in] void
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void tofu_send_response_resume(const uint8_t cid)
{
  uint32_t block_len = tofu->block_len;
  uint8_t transaction[TOFU_TRANSACTION_MAX_SIZE];

  memset(&transaction, 0, sizeof(transaction));

  transaction[0] = TOFU_CTL_RSP_RESUME_READY;
  memcpy(&transaction[1], &block_len, 4);
  memcpy(&transaction[5], &tofu->state.img_idx, 4);

  toa_send_response(cid, TOA_RSP_TOFU_CTL, &transaction[0], 9);
}

/**
 ****************************************************************************************
 * @brief send a TOFU Response
 *
 * @param[in] response   the response to send.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void tofu_send_response(const uint8_t cid, uint8_t response, const uint8_t* data, uint8_t datalen)
{
  uint8_t transaction[TOFU_TRANSACTION_MAX_SIZE];
  memset(&transaction, 0, sizeof(transaction));

  transaction[0]  = response;
  if((datalen <= TOFU_TRANSACTION_MAX_SIZE) && (0 != datalen) && (NULL != data))
  {
    memcpy(&transaction[1], data, datalen);
  }

  toa_send_response(cid, TOA_RSP_TOFU_CTL, &transaction[0], datalen+1);
}

static void tofu_send_response_error(const uint8_t cid, uint8_t error, uint8_t command)
{
  uint8_t transaction[] = {command, error};
  tofu_send_response(cid, TOFU_CTL_RSP_ERROR, transaction, sizeof(transaction));
}

/**
 ****************************************************************************************
 * @brief Init Session Info, Resume is NOT cleared
 *
 * @param[in] void
 *
 * @return    void
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tofu_init, (void))
{
  tofu->state.state        = TOFU_STATE_IDLE;
  tofu->state.cached_cid   = TOA_CONNECTIONLESS_CID;
  tofu->state.block_idx    = 0;
  tofu->state.block_len    = tofu->block_len;
  memset(tofu->block, 0, tofu->block_len);
}

/**
 ****************************************************************************************
 * @brief Clear everything (Session Info and Resume Context)
 *
 * @param[in] void
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void tofu_clear(void)
{
  tofu_init();
  tofu->state.img_idx      = 0;
  tofu->state.img_len      = 0;
  memset(&tofu->state.img_hash_ctx, 0, sizeof(tofu->state.img_hash_ctx));
  memset(&tofu->state.img_signature, 0, sizeof(tofu->state.img_signature));
  memset(&tofu->state.img_version, 0, sizeof(tofu->state.img_version));
  memset(&tofu->state.img_hash, 0, sizeof(tofu->state.img_hash));
}


/**
 ****************************************************************************************
 * @brief A terminal error has occurred while processing a block. Will clear the TOFU state.
 *
 * @param[in] status  Error code to send back to app
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void tofu_block_terminal_error(uint8_t status)
{
  if(TOA_CONNECTIONLESS_CID != tofu->state.cached_cid)
  {
    /* Only send if we still have a valid CID */
    tofu_send_response_error(tofu->state.cached_cid, status, TOFU_CTL_CMD_TOFU_DATA);
  }
  tofu_clear();
}


/**
 ****************************************************************************************
 * @brief A channel has been unassigned. Check if the channel was TOFU'ing.
 *
 * @param[in] cid    CID that was unassigned.
 *
 * @return    void
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tofu_channel_unassigned, (uint8_t cid))
{
  if(cid == tofu->state.cached_cid)
  {
    tofu_init();
  }
}


int tile_tofu_register(struct tile_tofu_module *module)
{
  tofu = module;
  toa_set_feature(TOA_FEATURE_TOFU);

  TOA_EXTERN_LINK(tofu_process_control_command);
  TOA_EXTERN_LINK(tofu_process_data);
  TOA_EXTERN_LINK(tofu_init);
  TOA_EXTERN_LINK(tofu_channel_unassigned);

  return TILE_SUCCESS;
}


/***********************************************************************/
/********************* TileLib TOFU API Functions **********************/
/***********************************************************************/

/**
 ****************************************************************************************
 * @brief Application has finished processing a complete TOFU Image
 *
 * @param[in] @TOA_FEATURE_ERROR_CODES error code
 *
 ****************************************************************************************
 */
void tofu_complete_done(uint8_t error)
{
  if(TOA_ERROR_OK != error)
  {
    tofu_block_terminal_error(TOA_ERROR_MEM_WRITE);
  }
  else
  {
    tofu->state.img_ok = true;
    if(TOA_CONNECTIONLESS_CID != tofu->state.cached_cid)
    {
      /* We may have disconnected before this call, so only send if we still have valid CID */
      tofu_send_response(tofu->state.cached_cid, TOFU_CTL_RSP_IMAGE_OK, NULL, 0);
    }
    tofu_clear();
  }
}

/**
 ****************************************************************************************
 * @brief Application has finished processing a TOFU Block
 *
 * @param[in] @TOA_FEATURE_ERROR_CODES error code
 *
 ****************************************************************************************
 */
void tofu_block_done(uint8_t error)
{
  if(TOFU_STATE_MEM != tofu->state.state)
  {
    /* Should never happen */
    return;
  }
  
  if(TOA_ERROR_OK != error)
  {
    tofu_block_terminal_error(error);
    return;
  }

  // Update block index
  tofu->state.img_idx += tofu->state.block_idx;
  tofu->state.block_idx = 0;

  if(tofu->state.img_idx >= tofu->state.img_len)
  {
    /* We have received the full image */
    uint8_t hash[TOFU_HASH_LEN];

    // finalize the image Hash calculation
    sha256_final_tile(&tofu->state.img_hash_ctx, hash);

    if(0 != memcmp(hash, tofu->state.img_hash, TOFU_HASH_LEN))
    {
      // The Hashes don't match
      tofu_block_terminal_error(TOA_ERROR_HASH);
      return;
    }

    /* We do not allow updates without a signature verification */
    if (NULL == tofu->pub_key || 1 != tile_sign_verify(tofu->pub_key, hash, tofu->state.img_signature))
    {
      // The Signatures don't match
      tofu_block_terminal_error(TOA_ERROR_SIGNATURE);
      return;
    }
    
    /* Let Application complete TOFU properly */
    tofu->complete();
  }
  else
  {
    if(TOA_CONNECTIONLESS_CID != tofu->state.cached_cid)
    {
      tofu_send_response(tofu->state.cached_cid, TOFU_CTL_RSP_BLOCK_OK, NULL, 0);
      tofu->state.state = TOFU_STATE_RX;
    }
    else
    {
      tofu->state.state = TOFU_STATE_IDLE;
    }
  }
  return;
}

