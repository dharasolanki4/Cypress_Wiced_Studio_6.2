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

#include "tdi.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tile_lib.h"
#include "modules/tile_tdi_module.h"
#include "toa.h"

#include "wiced_bt_trace.h"

struct tile_tdi_module *tdi = NULL;;

static void tdi_send_response(const uint8_t *token, uint8_t response, uint8_t *data, uint8_t datalen);

/**
 ****************************************************************************************
 * @brief Register the TDI module.
 *
 ****************************************************************************************
 */
int tile_tdi_register(struct tile_tdi_module *module)
{
  tdi = module;
  //toa_set_feature(TOA_FEATURE_TDI);

  return TILE_SUCCESS;
}

void tdi_process_command(const uint8_t *token, const uint8_t *data, uint8_t datalen)
{
  if(NULL == tdi)
  {
    uint8_t rsp[] = {data[0], TOA_ERROR_UNSUPPORTED};
    tdi_send_response(token, TDI_RSP_ERROR, rsp, sizeof(rsp));
    return;
  }

  switch(data[0])
  {
    case TDI_CMD_READ_AVAILABLE_INFO:
    {
      uint8_t info = 0x1F;
      tdi_send_response(token, TDI_RSP_READ_AVAILABLE_INFO, &info, 1);
      break;
    }
    case TDI_CMD_READ_TILE_ID:
      tdi_send_response(token, TDI_RSP_READ_TILE_ID, (uint8_t*)tdi->tile_id, TILE_ID_LEN);
      break;
    case TDI_CMD_READ_FIRMWARE_VERSION:
      tdi_send_response(token, TDI_RSP_READ_FIRMWARE_VERSION, (uint8_t*)tdi->firmware_version, TILE_FIRMWARE_VERSION_LEN);
      break;
    case TDI_CMD_READ_MODEL_NUMBER:
      tdi_send_response(token, TDI_CMD_READ_MODEL_NUMBER, (uint8_t*)tdi->model_number, TILE_MODEL_NUMBER_LEN);
      break;
    case TDI_CMD_READ_HARDWARE_VERSION:
      tdi_send_response(token, TDI_CMD_READ_HARDWARE_VERSION, (uint8_t*)tdi->hardware_version, TILE_HARDWARE_VERSION_LEN);
      break;
    case TDI_CMD_READ_BDADDR:
      tdi_send_response(token, TDI_CMD_READ_BDADDR, (uint8_t*)tdi->bdaddr, TILE_BDADDR_LEN);
      break;
    default:
    {
      uint8_t rsp[] = {data[0], TOA_ERROR_UNSUPPORTED};
      tdi_send_response(token, TDI_RSP_ERROR, rsp, sizeof(rsp));
    }
  }
}

static void tdi_send_response(const uint8_t *token, uint8_t response, uint8_t *data, uint8_t datalen)
{
  WICED_BT_TRACE("Tile: tdi_send_response\r\n");
  uint8_t rsp[20];

  rsp[0] = response;
  if(datalen > 0 && NULL != data)
  {
    memcpy(&rsp[1], data, datalen);
  }

  toa_send_connectionless_response((uint8_t*)token, TOA_RSP_TDI, rsp, datalen+1);
}
