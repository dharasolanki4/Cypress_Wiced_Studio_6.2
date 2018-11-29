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

 /** @file tmd.c
 ** @brief Tile Mode
 */

#include "tmd.h"
#include "toa.h"
#include "tdt.h"
#include "song.h"
#include "tile_lib.h"
#include "modules/tile_tmd_module.h"
#include "modules/tile_song_module.h"


struct tile_tmd_module *tmd;

extern struct tile_song_module *song;


/**
 ****************************************************************************************
 * @brief Process Incomming TMD Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tmd_process_command,(const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
    case TOA_COM_CMD_READ_FEATURES:
    {
      uint8_t rsp[2] = {TOA_COM_RSP_READ_FEATURES_OK, 3};
      toa_send_response(cid, TOA_RSP_TMD, &rsp[0], 2);
    }
    break;
    case TOA_COM_CMD_READ_VAL:
    {
      uint8_t mode;
      tmd->get(&mode);
      uint8_t rsp[] = {TOA_COM_RSP_READ_VAL_OK, mode};
      toa_send_response(cid, TOA_RSP_TMD, rsp, sizeof(rsp));
    }
    break;
    case TOA_COM_CMD_WRITE_VAL:
      if(datalen < 2)
      {
        uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TMD, rsp, sizeof(rsp));
      }
      else
      {
        uint8_t mode = data[1];
        uint8_t current_mode;
        int ret = TILE_SUCCESS;

        tmd->get(&current_mode);

        if(mode != current_mode)
        {
          /* Tell application */
          ret = tmd->set(mode);
          
          if(toa_get_feature(TOA_FEATURE_SONG))
          {
            if(TILE_MODE_ACTIVATED == mode)
            {
              song->play(TILE_SONG_ACTIVE, 2);
            }
            else
            {
              song->play(TILE_SONG_SLEEP, 2);
            }
          }
        }

        /* Send response */
        if(TILE_SUCCESS == ret)
        {
          uint8_t rsp[] = {TOA_COM_RSP_WRITE_VAL_OK, mode};
          toa_send_response(cid, TOA_RSP_TMD, rsp, sizeof(rsp));
        }
        else
        {
          uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
          toa_send_response(cid, TOA_RSP_TMD, rsp, sizeof(rsp));
        }
      }
    break;
    default:
    {
      uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TMD, rsp, sizeof(rsp));
    }
    break;
  }
}



/**
 ****************************************************************************************
 * @brief Process Incomming TMD Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
int tile_tmd_register(struct tile_tmd_module *module)
{
  /* TODO: Error checking? */
  tmd = module;
  toa_set_feature(TOA_FEATURE_TMD);

  TOA_EXTERN_LINK(tmd_process_command);

  return TILE_SUCCESS;
}
