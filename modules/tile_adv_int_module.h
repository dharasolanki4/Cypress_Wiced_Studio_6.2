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

/** @file tile_adv_int_module.h
 ** @brief Tile Advertising Interval Module interface
 */

#ifndef TILE_ADV_INT_MODULE_H_
#define TILE_ADV_INT_MODULE_H_

#include <stdint.h>

/**
 * Tile ADV_INT module.
 *
 * This module is used to adjust the advertising interval on the device.
 */
struct tile_adv_int_module
{
  /**
   * Get the current advertsing interval.
   *
   * @param[out] adv_int  The current advertising interval, in 0.625ms increments.
   *
   * @return See @ref TILE_ERROR_CODES.
   */
  int (*get)(uint16_t *adv_int);

  /**
   * Set the advertising interval.
   *
   * @param[in] adv_int  The advertising interval to set, in 0.625ms increments.
   *
   * @return See @ref TILE_ERROR_CODES.
   */
  int (*set)(uint16_t adv_int);
};


/**
 * Register the adv_int module.
 */
int tile_adv_int_register(struct tile_adv_int_module *module);

#endif
