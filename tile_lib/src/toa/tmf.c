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

#include "tmf.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tile_lib.h"
#include "modules/tile_tmf_module.h"
#include "modules/tile_song_module.h"
#include "modules/tile_tmd_module.h"

#include "song.h"
#include "toa.h"


struct tile_tmf_module *tmf = NULL;
static const uint8_t tmf_null[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t tmf_undo_ship_key[] = {0xc3, 0x71, 0x3a, 0xba, 0x70, 0x78, 0x1c, 0x6d, 0x10, 0xc4, 0x45, 0x24, 0xc6};

static void tmf_send_response(const uint8_t *token, uint8_t response, uint8_t *data, uint8_t datalen);

extern struct tile_song_module *song;
extern struct tile_tmd_module *tmd;


TOA_EXTERN_IMPL(void, tmf_process_command, (const uint8_t *token, const uint8_t *data, uint8_t datalen))
{
  uint8_t res_8 = TOA_ERROR_OK;
  uint8_t battery_level;

  if(0 == datalen)
  {
    /* ignore */
    return;
  }
  
  if(NULL == tmf)
  {
    uint8_t rsp[] = {data[0], TOA_ERROR_UNSUPPORTED};
    tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
    return;
  }
  
  if(tmd != NULL)
  {
    uint8_t mode;
    tmd->get(&mode);
    
    if(TILE_MODE_SHIPPING == mode)
    {
      switch(data[0])
      {
        case TMF_CMD_UNDO_SHIP:
        case TMF_CMD_READ_TILE_MODE:
        case TMF_CMD_READ_BATTERY_LEVEL:
        case TMF_CMD_READ_LOADED_BATTERY_LEVEL:
          /* continue */
          break;
        default:
        {
          uint8_t rsp[] = {data[0], TOA_ERROR_INVALID_STATE};
          tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
          return;
        }
      }
    }
    else if(TILE_MODE_ACTIVATED == mode)
    {
      uint8_t rsp[] = {data[0], TOA_ERROR_INVALID_STATE};
      tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      return;
    }
    else
    {
      if(TMF_CMD_UNDO_SHIP == data[0])
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_INVALID_STATE};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
        return;
      }
      /* manufacturing mode, proceed */
    }
  }
  
  switch(data[0])
  {
    case TMF_CMD_WRITE_TILE_ID:
      if(datalen < TILE_ID_LEN+1 || 0 == memcmp(&data[1], tmf_null, TILE_ID_LEN))
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(tmf->tile_id, &data[1], TILE_ID_LEN);
        tmf_send_response(token, TMF_RSP_WRITE_TILE_ID, (uint8_t*)tmf->tile_id, TILE_ID_LEN);
      }
      break;
    case TMF_CMD_WRITE_TILE_AUTH_KEY_START:
      if(datalen < (TILE_AUTH_KEY_LEN/2)+1 || 0 == memcmp(&data[1], tmf_null, TILE_AUTH_KEY_LEN/2))
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(tmf->tile_auth_key, &data[1], TILE_AUTH_KEY_LEN/2);
        tmf_send_response(token, TMF_RSP_WRITE_TILE_AUTH_KEY_START, (uint8_t*)tmf->tile_auth_key, TILE_AUTH_KEY_LEN/2);
      }
      break;
    case TMF_CMD_WRITE_TILE_AUTH_KEY_END:
      if(datalen < (TILE_AUTH_KEY_LEN/2)+1 || 0 == memcmp(&data[1], tmf_null, TILE_AUTH_KEY_LEN/2))
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(&tmf->tile_auth_key[TILE_AUTH_KEY_LEN/2], &data[1], TILE_AUTH_KEY_LEN/2);
        tmf_send_response(token, TMF_RSP_WRITE_TILE_AUTH_KEY_END, (uint8_t*)&tmf->tile_auth_key[TILE_AUTH_KEY_LEN/2], TILE_AUTH_KEY_LEN/2);
      }
      break;
    case TMF_CMD_WRITE_MODEL_NUMBER:
      if(datalen < TILE_MODEL_NUMBER_LEN+1 || 0 == data[1])
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(tmf->model_number, &data[1], TILE_MODEL_NUMBER_LEN);
        tmf_send_response(token, TMF_RSP_WRITE_MODEL_NUMBER, (uint8_t*)tmf->model_number, TILE_MODEL_NUMBER_LEN);
      }
      break;
    case TMF_CMD_WRITE_HARDWARE_VERSION:
      if(datalen < TILE_HARDWARE_VERSION_LEN+1 || 0 == data[1])
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(tmf->hardware_version, &data[1], TILE_HARDWARE_VERSION_LEN);
        tmf_send_response(token, TMF_RSP_WRITE_HARDWARE_VERSION, (uint8_t*)tmf->hardware_version, TILE_HARDWARE_VERSION_LEN);
      }
      break;
    case TMF_CMD_WRITE_BDADDR:
      if(datalen < TILE_BDADDR_LEN+1 || 0 == memcmp(&data[1], tmf_null, TILE_BDADDR_LEN))
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        memcpy(tmf->bdaddr, &data[1], TILE_BDADDR_LEN);
        tmf_send_response(token, TMF_RSP_WRITE_BDADDR, (uint8_t*)tmf->bdaddr, TILE_BDADDR_LEN);
      }
      break;
    case TMF_CMD_SHIP:
      res_8 = tmf->ship();
    
      if(TILE_SUCCESS != res_8)
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS, res_8};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      else
      {
        tmf_send_response(token, TMF_RSP_SHIP, (uint8_t*)&res_8, 1);
      }
      break;
    case TMF_CMD_PLAY_SONG:
      if(0xFF != data[1] && 3 == datalen)
      {
        song->play(data[1], data[2]);
        tmf_send_response(token, TMF_RSP_PLAY_SONG, (uint8_t*)&data[1], 2);
      }
      else if(0xFF == data[1] && 2 == datalen)
      {
        song->stop();
        tmf_send_response(token, TMF_RSP_PLAY_SONG, (uint8_t*)&data[1], 1);
      }
      else
      {
        uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
        tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
      }
      break;
    case TMF_CMD_READ_BATTERY_LEVEL:
        tmf->read_battery_level(&battery_level);
        tmf_send_response(token, TMF_RSP_READ_BATTERY_LEVEL, &battery_level, 1);
      break;
    case TMF_CMD_READ_LOADED_BATTERY_LEVEL:
      {
        uint8_t data[TOA_MPS - 1] = {0};      /* minus one for TMF_RSP_READ_LOADED_BATTERY_LEVEL*/
        uint8_t data_len = sizeof(data) - 1;  /* minus one because data[0] is for the battery level */
        
        tmf->read_loaded_battery_level(&data[0], &data[1], &data_len);
        
        battery_level = data[0];  /* copy to global variable */
        
        tmf_send_response(token, TMF_RSP_READ_LOADED_BATTERY_LEVEL, data, data_len + 1);  /* plus one because data[0] is for battery level */
      }
      break;
    case TMF_CMD_READ_CHIP_DATA:
      {
        uint8_t index = 0;

        while((tmf->chip_data_length - index) > (TOA_MPS-1))
        {
          tmf_send_response(token, TMF_RSP_READ_CHIP_DATA_CONT, (uint8_t*)&tmf->chip_data[index], TOA_MPS-1);
          index += (TOA_MPS-1);
        }
        tmf_send_response(token, TMF_RSP_READ_CHIP_DATA_END, (uint8_t*)&tmf->chip_data[index], tmf->chip_data_length - index);
      }
      break;
    case TMF_CMD_READ_TILE_AUTH_KEY_START:
        tmf_send_response(token, TMF_RSP_READ_TILE_AUTH_KEY_START, (uint8_t*)tmf->tile_auth_key, TILE_AUTH_KEY_LEN/2);
      break;
    case TMF_CMD_READ_TILE_AUTH_KEY_END:
        tmf_send_response(token, TMF_RSP_READ_TILE_AUTH_KEY_END, (uint8_t*)&tmf->tile_auth_key[TILE_AUTH_KEY_LEN/2], TILE_AUTH_KEY_LEN/2);
      break;
    case TMF_CMD_READ_TILE_MODE:
      {
        uint8_t current_mode = 0xFF;

        if(NULL != tmd)
        {
          tmd->get(&current_mode);
        }
        tmf_send_response(token, TMF_RSP_READ_TILE_MODE, &current_mode, 1);
      }
      break;
    case TMF_CMD_UNDO_SHIP:
      {
        if(datalen < (1 + sizeof(tmf_undo_ship_key)))
        {
          uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
          tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
          return;
        }
        
        if(0 != memcmp(&data[1], tmf_undo_ship_key, sizeof(tmf_undo_ship_key)))
        {
          uint8_t rsp[] = {data[0], TOA_ERROR_PARAMETERS};
          tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
          return;
        }
        
        uint8_t mode = 0xff;
        
        if(tmd != NULL)
        {
          tmd->set(TILE_MODE_MANUFACTURING);
          tmd->get(&mode);
        }
        
        tmf_send_response(token, TMF_RSP_UNDO_SHIP, &mode, 1);
      }
      break;
    default:
    {
      uint8_t rsp[] = {data[0], TOA_ERROR_UNSUPPORTED};
      tmf_send_response(token, TMF_RSP_ERROR, rsp, sizeof(rsp));
    }
  }
}

static void tmf_send_response(const uint8_t *token, uint8_t response, uint8_t *data, uint8_t datalen)
{
  uint8_t rsp[20];

  rsp[0] = response;
  if(datalen > 0 && NULL != data)
  {
    memcpy(&rsp[1], data, datalen);
  }

  toa_send_connectionless_response((uint8_t*)token, TOA_RSP_TMF, rsp, datalen+1);
}



/**
 ****************************************************************************************
 * @brief Register the TMF module.
 *
 ****************************************************************************************
 */
int tile_tmf_register(struct tile_tmf_module *module)
{
  tmf = module;
  toa_set_feature(TOA_FEATURE_TMF);

  TOA_EXTERN_LINK(tmf_process_command);

  return TILE_SUCCESS;
}


/**
 ****************************************************************************************
 * @brief Unregister the TMF module.
 *
 ****************************************************************************************
 */
int tile_tmf_unregister(void)
{
  toa_clear_feature(TOA_FEATURE_TMF);
  tmf = NULL;

  return TILE_SUCCESS;
}
