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

/** @file tile_gap.c
 ** @brief Tile GAP Functionality
 */

#include "tile_lib.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_timer_driver.h"

#include "modules/tile_toa_module.h"
#include "modules/tile_tcu_module.h"
#include "../toa/toa.h"
#include "../toa/tcu.h"
#include "../toa/tofu.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>


struct tile_gap_driver *gap = NULL;

extern struct tile_timer_driver *timer;
extern struct tile_toa_module   *toa;


/**
 * Register the GAP driver with Tile Library.
 */
int tile_gap_register(struct tile_gap_driver *driver)
{
  gap = driver;

  return TILE_SUCCESS;
}


/**
 * Call when a connection has been established.
 */
int tile_gap_connected(struct tile_conn_params *params)
{
  if(NULL != params)
  {
    gap->conn_params = *params;
  }

  if(NULL != toa)
  {
    toa_init();
  }

  if(NULL != toa && toa_get_feature(TOA_FEATURE_TCU))
  {
    tcu_connected();
  }

  if(NULL != timer && gap->authentication_timer_delay > 0)
  {
    timer->start(TILE_AUTHENTICATION_TIMER, gap->authentication_timer_delay);
  }

  return TILE_SUCCESS;
}

/**
 * Call when a connection has been terminated.
 */
int tile_gap_disconnected(void)
{
  if(NULL != timer)
  {
    timer->cancel(TILE_CONNECTION_TIMER);
    timer->cancel(TILE_AUTHENTICATION_TIMER);
    timer->cancel(TILE_TCU_PARAM_UPDATE_TIMER);
  }
  
  if(NULL != toa)
  {
    toa_disconnect();
  }

  return TILE_SUCCESS;
}

/**
 * Call when the connection parameters have been updated.
 * Prior to calling this function, update the values in
 * the conn_params field of tile_gap_driver.
 */
int tile_gap_params_updated(struct tile_conn_params *conn_params)
{
  gap->conn_params = *conn_params;

  if(NULL != toa && toa_get_feature(TOA_FEATURE_TCU))
  {
    tcu_params_updated();
  }

  return TILE_SUCCESS;
}
