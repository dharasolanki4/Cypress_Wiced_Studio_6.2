
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

/** @file tcu.c
 ** @brief Tile Connection Update: used for changing the connection parameters.
 */

#include "tcu.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "tile_lib.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_tcu_module.h"
#include "toa.h"


static uint8_t tcu_update_retry_count;

static tcu_params_t new_conn_params;

struct tile_tcu_module *tcu = NULL;

extern struct tile_gap_driver *gap;
extern struct tile_timer_driver *timer;

static void tcu_hdc_check(void);



/**
 * @brief Connection parameters have been updated. Perform any necessary checks
 *        or operations.
 */
TOA_EXTERN_IMPL(void, tcu_params_updated, (void))
{
  timer->cancel(TILE_CONNECTION_TIMER);
  timer->cancel(TILE_TCU_PARAM_UPDATE_TIMER);

  /* Reset connection timer, if necessary */
  tcu_hdc_check();
}


/**
 * @brief If a requested update to the connection parameters has not completed by
 *        the time this timer expires, try again or disconnect if too many retries
 */
TOA_EXTERN_IMPL(void, tcu_param_update_timer_handler, (void))
{
  /* Too many retries, and we disconnect. Otherwise try again */
  if(tcu_update_retry_count >= TCU_RETRY_MAX)
  {
    if(NULL != tcu->disconnect_count)
    {
      (*tcu->disconnect_count)++;
    }
    gap->gap_disconnect();
  }
  else
  {
    tcu_update_retry_count++;
    if(tcu->tcu_update_retry_delay > 0)
    {
      timer->start(TILE_TCU_PARAM_UPDATE_TIMER, tcu->tcu_update_retry_delay);
    }

    /* Call this function directly (instead of tcu_param_update) so we don't reset the counter */
    tcu->update_params(new_conn_params.min_conn_interval, new_conn_params.max_conn_interval,
        new_conn_params.slave_latency, new_conn_params.conn_sup_timeout);
  }
}



/**
 ****************************************************************************************
 * @brief Process Incomming TCU Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tcu_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{

  switch(data[0])
  {
    case TOA_COM_CMD_READ_FEATURES:
    {
      uint8_t rsp[2] = {TOA_COM_RSP_READ_FEATURES_OK, 3};  // 3 stands for Read and Write
      toa_send_response(cid, TOA_RSP_TCU, &rsp[0], 2);
    }
    break;
    case TOA_COM_CMD_READ_VAL:
    {
      uint8_t rsp[sizeof(tcu_params_t)+1]; // Should evaluate to 9
      rsp[0] = TOA_COM_RSP_READ_VAL_OK;

      tcu_params_t params = {
          .min_conn_interval = gap->conn_params.conn_interval,
          .max_conn_interval = gap->conn_params.conn_interval,
          .slave_latency     = gap->conn_params.slave_latency,
          .conn_sup_timeout  = gap->conn_params.conn_sup_timeout
      };

      memcpy(&rsp[1], (uint8_t*)&params, sizeof(params));

      toa_send_response(cid, TOA_RSP_TCU, &rsp[0], sizeof(rsp));
    }
    break;
    case TOA_COM_CMD_WRITE_VAL:
      if(datalen < (sizeof(tcu_params_t)+1))
      {
        uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TCU, rsp, sizeof(rsp));
      }
      else
      {
        uint8_t rsp[sizeof(tcu_params_t)+1];

        /* update connection params */
        memcpy(&new_conn_params, &data[1], sizeof(new_conn_params));

        tcu_param_update(&new_conn_params);

        rsp[0] = TOA_COM_RSP_WRITE_VAL_OK;
        memcpy(&rsp[1], (uint8_t*) &new_conn_params, sizeof(new_conn_params));

        toa_send_response(cid, TOA_RSP_TCU, &rsp[0], sizeof(rsp));
      }
    break;
    default:
    {
      uint8_t rsp[] = {TOA_COM_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TCU, rsp, sizeof(rsp));
    }
    break;
  }
}


/**
 * @brief Check if we're in high duty cycle and set connection timer if we are.
 */
static void tcu_hdc_check(void)
{
  uint32_t interval = gap->conn_params.conn_interval * (gap->conn_params.slave_latency + 1);
  if(interval < TCU_HDC_THRESHOLD)
  {
    if(tcu->high_duty_cycle_timer_delay > 0)
    {
      timer->start(TILE_CONNECTION_TIMER, tcu->high_duty_cycle_timer_delay);
    }
  }
}


TOA_EXTERN_IMPL(void, tcu_connected, (void))
{
  timer->start(TILE_CONNECTION_TIMER, tcu->connection_timer_delay);
}


/**
 * @brief Update connection parameters. Will also begin defensive measures to ensure
 *        parameters actually get updated (will re-attempt update and disconnect if
 *        the update does not take place). Please always call this function and not
 *        app_param_update_func().
 *
 * @param[in] conn_params   Connection parameters struct
 */
TOA_EXTERN_IMPL(void, tcu_param_update, (tcu_params_t* conn_params))
{
  timer->cancel(TILE_CONNECTION_TIMER);

  /* Save parameters */
  if(NULL != conn_params)
  {
    new_conn_params.min_conn_interval = conn_params->min_conn_interval; // N * 1.25ms
    new_conn_params.max_conn_interval = conn_params->max_conn_interval; // N * 1.25ms
    new_conn_params.slave_latency     = conn_params->slave_latency; // Conn Events skipped
    new_conn_params.conn_sup_timeout  = conn_params->conn_sup_timeout ; // N * 10ms

  }
  else
  {
    /* Fill in with defaults */
    new_conn_params.min_conn_interval = TILE_CONN_INT_MIN_DEFAULT; // N * 1.25ms
    new_conn_params.max_conn_interval = TILE_CONN_INT_MAX_DEFAULT; // N * 1.25ms
    new_conn_params.slave_latency     = TILE_CONN_SLAVE_LAT_DEFAULT; // Conn Events skipped
    new_conn_params.conn_sup_timeout  = TILE_CONN_TIMEOUT_DEFAULT; // N * 10ms
  }

  if(new_conn_params.slave_latency != gap->conn_params.slave_latency ||
      new_conn_params.conn_sup_timeout != gap->conn_params.conn_sup_timeout ||
      new_conn_params.max_conn_interval < gap->conn_params.conn_interval ||
      new_conn_params.min_conn_interval > gap->conn_params.conn_interval)
  {
    /* Update if new parameters differ from what we already have */
    tcu_update_retry_count = 0;
    if(tcu->tcu_update_retry_delay > 0)
    {
      timer->start(TILE_TCU_PARAM_UPDATE_TIMER, tcu->tcu_update_retry_delay);
    }

    tcu->update_params(new_conn_params.min_conn_interval, new_conn_params.max_conn_interval,
        new_conn_params.slave_latency, new_conn_params.conn_sup_timeout);
  }
  else
  {
    /* Otherwise, reset connection timer, if necessary */
    tcu_hdc_check();
  }
}


/**
 ****************************************************************************************
 * @brief Register TCU module
 *
 * @param[in] module   pointer to module config.
 *
 ****************************************************************************************
 */
int tile_tcu_register(struct tile_tcu_module *module)
{
  tcu = module;
  toa_set_feature(TOA_FEATURE_TCU);

  TOA_EXTERN_LINK(tcu_process_command);
  TOA_EXTERN_LINK(tcu_params_updated);
  TOA_EXTERN_LINK(tcu_param_update_timer_handler);
  TOA_EXTERN_LINK(tcu_connected);
  TOA_EXTERN_LINK(tcu_param_update);

  return TILE_SUCCESS;
}
