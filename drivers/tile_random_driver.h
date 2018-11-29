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

/** @file tile_random_driver.h
 ** @brief Tile Random Bytes Generation. Provides TileLib an interface to generate random numbers.
 */

#ifndef TILE_RANDOM_DRIVER_H_
#define TILE_RANDOM_DRIVER_H_

#include <stdint.h>


/**
 * Tile random number driver
 */
struct tile_random_driver
{
  /**
   * Generate random bytes.
   *
   * @param[out] dst    Buffer to write random bytes to.
   * @param[in]  length Number of random bytes to generate.
   *             Known limitation: On Nordic SDK v15.2.0, max number of random bytes available is 64
   * @return See @ref TILE_ERROR_CODES.
   */
  int (*random_bytes)(uint8_t *dst, uint8_t length);
};


/**
 * Register the Tile random driver.
 */
int tile_random_register(struct tile_random_driver *driver);

#endif
