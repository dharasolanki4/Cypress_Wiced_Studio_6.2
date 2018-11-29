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

/** @file tile_tcu_module.h
 ** @brief Tile Connection Update module
 */

#ifndef TILE_TCU_MODULE_H_
#define TILE_TCU_MODULE_H_

#include <stdint.h>


/**
 * Tile Connection Update module.
 *
 * This module is used by Tile Lib to update the connection parameters
 * during a connection.
 */
struct tile_tcu_module
{
  /**
   * Time in 10ms increments after connection until an automatic update to
   * low duty cycle connection parameters. A value of 0 indicates that this
   * timer will not be used. This value may be updated at any time, except
   * it cannot clear a running timer. The value is used after connection.
   */
  uint16_t connection_timer_delay;

  /**
   * Time in 10ms increments before an automatic update from high duty cycle
   * parameters to low duty cycle parameters. A value of 0 indicates that this
   * timer will not be used. This value may be updated at any time, except
   * it cannot clear a running timer. The value is used after the device starts
   * to use high duty cycle connection parameters (when the connection interval
   * is less than 500ms).
   */
  uint16_t high_duty_cycle_timer_delay;

  /**
   * Time in 10ms increments from connection update request sent to retry, if
   * parameters have not been updated. A value of 0 indicates that this timer
   * will not be used. This value may be updated at any time, except it
   * cannot clear a running timer. The value is used after a TCU update
   * request is issued from the client.
   */
  uint16_t tcu_update_retry_delay;

  /**
  * Diagnostic info: counts the number of disconnections triggered by TCU
   */
  uint16_t* disconnect_count;

  /**
   * Send a connection parameters update request for the given parameters.
   */
  int (*update_params)(uint16_t min_conn_interval, uint16_t max_conn_interval,
      uint16_t slave_latency, uint16_t conn_sup_timeout);
};


/**
 * Register the TCU module.
 */
int tile_tcu_register(struct tile_tcu_module *module);

#endif
