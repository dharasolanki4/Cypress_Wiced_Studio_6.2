

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
 * In case of a disconnection during a TDG get transaction, the TDG Client SHALL send the partial Diagnostic data tothe Cloud Server.
 */

 /** @file tdg.c
 ** @brief Tile Diagnostic Feature
 */


#include "tdg.h"

#include "tile_lib.h"
#include "modules/tile_tdg_module.h"

#include <string.h>

#include "toa.h"

struct tile_tdg_module *tdg;

static uint8_t cached_cid = 0;


int tdg_add_data(void * data, uint8_t length)
{
  uint8_t *pdata = data;

  while(length--)
  {
    if(TOA_MPS == tdg->buffer_pos) /* TODO: Replace TOA_MPS with toa->mps when that feature is ready */
    {
      /* Send out buffer if it is full */
      tdg->buffer[0] = TDG_RSP_DATA_CONT;
      toa_send_response(cached_cid, TOA_RSP_TDG, tdg->buffer, tdg->buffer_pos);
      tdg->buffer_pos = 1;
    }
    tdg->buffer[tdg->buffer_pos++] = *pdata++;
  }

  return TILE_SUCCESS;
}


int tdg_finish(void)
{
  tdg->buffer[0] = TDG_RSP_DATA_END;
  toa_send_response(cached_cid, TOA_RSP_TDG, tdg->buffer, tdg->buffer_pos);

  cached_cid = 0;

  return TILE_SUCCESS;
}


/**
 ****************************************************************************************
 * @brief process incomming TDG commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdg_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
    case TDG_CMD_GET:
      if(0 != cached_cid)
      {
        /* TODO: Send resource in use error */
      }
      else
      {
        /* Start from pos = 1 to leave room for the RSP code */
        cached_cid = cid;
        tdg->buffer_pos = 1;
        tdg->get_diagnostics();
      }
    break;
    default:
    {
      uint8_t rsp[] = {TDG_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TDG, rsp, sizeof(rsp));
      break;
    }
  }
}



/**
 ****************************************************************************************
 * @brief Register the TDG module.
 *
 ****************************************************************************************
 */
int tile_tdg_register(struct tile_tdg_module *module)
{
  /* TODO: Error checking? */
  tdg = module;
  toa_set_feature(TOA_FEATURE_TDG);

  TOA_EXTERN_LINK(tdg_process_command);

  return TILE_SUCCESS;
}
