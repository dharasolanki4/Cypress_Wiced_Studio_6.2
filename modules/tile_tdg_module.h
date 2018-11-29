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

/** @file tile_gatt_server_driver.h
 ** @brief Tile GATT Server Driver interface
 */

#ifndef TILE_TDG_MODULE_H_
#define TILE_TDG_MODULE_H_

#include <stdint.h>

#include "modules/tile_toa_module.h"

/**
 * Tile Diagnostics module.
 *
 * This module is used by Tile Lib to send diagnostic information to the Tile
 * data collection system. Consult with Tile for the proper format for
 * diagnostic data, if it is to be automatically parsed by the Tile backend.
 */
struct tile_tdg_module {
  /**
   * Retrieve diagnostic information.
   *
   * This function should call @ref tdg_add_data for each diagnostic data
   * field to be added, and then @ref tdg_finish when all data has been added.
   */
  int (*get_diagnostics)(void);

  uint8_t buffer[TOA_MPS];
  uint8_t buffer_pos;
};


/**
 * Register the TDG module.
 */
int tile_tdg_register(struct tile_tdg_module *module);

/**
 * Add diagnostic data. Should be called during the call to get_diagnostics.
 * This function can be called multiple times, for each piece of diagnostic
 * info that is to be added.
 *
 * @param[in] data   Data to add to diagnostics.
 * @param[in] length Length of data to add.
 */
int tdg_add_data(void *data, uint8_t length);

/**
 * Finish adding diagnostic data. Should be called during the call to
 * get_diagnostics, after all data has been added.
 */
int tdg_finish(void);

#endif
