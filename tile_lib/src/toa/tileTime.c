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

#include "tileTime.h"

#include <stdint.h>
#include <string.h>
#include "tile_lib.h"
#include "toa.h"
#include "modules/tile_time_module.h"

struct tile_time_module *time;



TOA_EXTERN_IMPL(void, time_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
#ifdef INT64_MAX
    case TOA_COM_CMD_READ_FEATURES:
    {
      uint8_t rsp[2] = {TOA_COM_RSP_READ_FEATURES_OK, 3};  // 3 stands for Read and Write
      toa_send_response(cid, TOA_RSP_TIME, &rsp[0], sizeof(rsp));
    }
    break;
    case TOA_COM_CMD_READ_VAL:
    {
      int64_t timestamp;
      uint16_t inaccuracy;
      time->get(&timestamp, &inaccuracy);

      uint8_t rsp[11] = {
          TOA_COM_RSP_READ_VAL_OK,
          MKLE16(inaccuracy),
      };

      memcpy(&rsp[3], &timestamp, 8);

      toa_send_response(cid, TOA_RSP_TIME, &rsp[0], sizeof(rsp));
    }
    break;
    case TOA_COM_CMD_WRITE_VAL:
      if(datalen < 9)
      {
        uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TIME, rsp, sizeof(rsp));
      }
      else
      {
        int64_t timestamp;
        memcpy(&timestamp, &data[1], 8);

        uint8_t rsp[11] = {
            TOA_COM_RSP_WRITE_VAL_OK,
            MKLE16(1), /* Inaccuracy of 1 just after a write */
        };

        memcpy(&rsp[3], &timestamp, 8);

        time->set(timestamp);

        toa_send_response(cid, TOA_RSP_TIME, &rsp[0], sizeof(rsp));
      }
    break;
#endif /* INT64_MAX */
    default:
    {
      uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TIME, rsp, sizeof(rsp));
      break;
    }
  }
}


int tile_time_register(struct tile_time_module *module)
{
  time = module;

  toa_set_feature(TOA_FEATURE_TIME);

  TOA_EXTERN_LINK(time_process_command);

  return TILE_SUCCESS;
}

