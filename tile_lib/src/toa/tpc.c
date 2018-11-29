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

 /** @file tpc.c
 ** @brief Tile Power Control
 */

#include "tpc.h"
#include "toa.h"
#include "tile_lib.h"
#include "modules/tile_tpc_module.h"

#include <string.h>


struct tile_tpc_module *tpc;

/* Local Function Declaration */
static void tpc_cmd_set_tx_limit(uint8_t cid, const uint8_t * data, uint8_t datalen);
static void tpc_cmd_read_tx_limit(uint8_t cid, const uint8_t * data, uint8_t datalen);
static void tpc_cmd_read_tx_min_max(uint8_t cid, const uint8_t * data, uint8_t datalen);


/**
 ****************************************************************************************
 * @brief Process Incomming TPC Commands
 *
 * @param[in] cid        CID of channel that made request.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tpc_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
    case TPC_CMD_READ_CONFIG:
    {
      tpc_config_t conf;
      tpc->get(&conf);

      uint8_t rsp[] = {
        TPC_RSP_READ_CONFIG,
        conf.enabled,
        conf.lag,
        conf.low_level,
        conf.high_level
      };
      toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
    }
    break;
    case TPC_CMD_WRITE_CONFIG:
      if(datalen < 5)
      {
        uint8_t rsp[] = {TPC_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
      }
      else
      {
        tpc_config_t conf = {
          .enabled    = data[1],
          .lag        = data[2],
          .low_level  = data[3],
          .high_level = data[4],
        };

        if(0 == conf.enabled)
        {
          /* If disabled, just clear everything */
          memset(&conf, 0, sizeof(conf));
        }
        else
        {
          if(conf.low_level >= conf.high_level)
          {
            /* low_level must be less than high_level */
            uint8_t rsp[] = {TPC_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
            toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
            return;
          }
        }

        /* Tell application */
        int ret = tpc->set(&conf);

        /* Send response */
        if(TILE_SUCCESS == ret)
        {
          uint8_t rsp[] = {
            TPC_RSP_WRITE_CONFIG,
            conf.enabled,
            conf.lag,
            conf.low_level,
            conf.high_level
          };
          toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
        }
        else
        {
          uint8_t rsp[] = {TPC_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
          toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
        }
      }
    break;

    case TPC_CMD_SET_TX_LIMIT:
      tpc_cmd_set_tx_limit(cid, &data[1], datalen-1);
      break;

    case TPC_CMD_READ_TX_LIMIT:
      tpc_cmd_read_tx_limit(cid, &data[1], datalen-1);
      break;

    case TPC_CMD_READ_TX_MIN_MAX:
      tpc_cmd_read_tx_min_max(cid, &data[1], datalen-1);
      break;

    default:
    {
      uint8_t rsp[] = {TPC_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
    }
    break;
  }
}

/**
 * Sets the transmit power limit
 */
static void tpc_cmd_set_tx_limit(uint8_t cid, const uint8_t * data, uint8_t datalen)
{
  if (datalen < 4)
  {
    uint8_t rsp[] = {TPC_RSP_ERROR, TPC_CMD_SET_TX_LIMIT, TOA_ERROR_PARAMETERS};
    toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
    return;
  }

  txl_config_t conf;

  conf.adv_tx_limit = RDLE16(&data[0]);
  conf.con_tx_limit = RDLE16(&data[2]);

  /* Tell application */
  int ret = tpc->set_tx_limits(&conf);

  /* Send response */
  if(TILE_SUCCESS == ret)
  {
    int16_t adv_tx;
    int16_t con_tx;

    tpc->get_tx_setting(&adv_tx, &con_tx);

    uint8_t rsp[] = {TPC_RSP_SET_TX_LIMIT, MKLE16(adv_tx), MKLE16(con_tx)};

    toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
  }
  else
  {
    uint8_t rsp[] = {TPC_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
    toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
  }
}

/**
 * Read back the current transmit power limits
 */
static void tpc_cmd_read_tx_limit(uint8_t cid, const uint8_t * data, uint8_t datalen)
{
  txl_config_t conf;
  tpc->get_tx_limits(&conf);

  int16_t actual_adv_tx;
  int16_t actual_con_tx;

  tpc->get_tx_setting(&actual_adv_tx, &actual_con_tx);

  uint8_t rsp[] = {
    TPC_RSP_READ_TX_LIMIT,
    MKLE16(conf.adv_tx_limit),
    MKLE16(conf.con_tx_limit),
    MKLE16(actual_adv_tx),
    MKLE16(actual_con_tx)
  };

  toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
}

/**
 * Read the minimum and maximum transmit capablities of the Tile.
 */
static void tpc_cmd_read_tx_min_max(uint8_t cid, const uint8_t * data, uint8_t datalen)
{
  int16_t min_dbm;
  int16_t max_dbm;

  tpc->get_tx_min_max(&min_dbm, &max_dbm);

  uint8_t rsp[] = {TPC_RSP_READ_TX_MIN_MAX, MKLE16(min_dbm), MKLE16(max_dbm)};
  toa_send_response(cid, TOA_RSP_TPC, rsp, sizeof(rsp));
}



/**
 ****************************************************************************************
 * @brief Register TPC module
 *
 * @param[in] module    Pointer to TPC module.
 * @return              @ref TILE_SUCCESS
 *
 ****************************************************************************************
 */
int tile_tpc_register(struct tile_tpc_module *module)
{
  tpc = module;
  toa_set_feature(TOA_FEATURE_TPC);

  TOA_EXTERN_LINK(tpc_process_command);

  return TILE_SUCCESS;
}
