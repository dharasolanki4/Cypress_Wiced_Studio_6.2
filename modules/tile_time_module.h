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

/** @file tile_time_module.h
 ** @brief Tile Time Module
 */

#ifndef TILE_TIME_MODULE_H_
#define TILE_TIME_MODULE_H_

#include <stdint.h>

#ifdef INT64_MAX

/**
 * Tile Time module.
 *
 * This module is used by Tile Lib to set and get the current time.
 */
struct tile_time_module {
  /**
   * Set the current time, given as time in milliseconds since 01 January, 2016 00:00:00 UTC.
   */
  int (*set)(int64_t time);

  /**
   * Get the current time, given as time in milliseconds since 01 January, 2016 00:00:00 UTC.
   * There is also an "inaccuracy" value, which should be an estimation of the number of seconds
   * the clock has drifted by.
   */
  int (*get)(int64_t *time, uint16_t *inaccuracy);
};


/**
 * Register the Tile time module.
 */
int tile_time_register(struct tile_time_module *module);

#endif /* INT64_MAX */

#endif
