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

/** @file tile_tmf_moidule.h
 ** @brief Tile ManuFacturing Module
 */

#ifndef TILE_TMF_MODULE_H_
#define TILE_TMF_MODULE_H_

#include <stdint.h>


/**
 * Tile Manufacturing module.
 *
 * This module can be used during the manufacturing process to write the
 * unique numbers to the device and set to shipping mode.
 */
struct tile_tmf_module
{
  /**
   * Ship the Tile.
   * The Tile shall return @ref TILE_SUCCESS when the following conditions have been met:
   * - Tile ID is non zero.
   * - Auth Key is non zero.
   * - Model number does not equal "TILE 99.99"
   * - Hardware version does not equal "99.99"
   * - The Tile has a valid bdaddr. 
   * 
   * Otherwise, return @ref TILE_ERROR_NOT_INITIALIZED.
   */
  int (*ship)(void);

  /**
   * Read the Tile Battery.
   */
  int (*read_battery_level)(uint8_t*);

  /**
   * Read Loaded battery level.
   * @param[out]    the loaded battery level
   * @param[inout]  data to store any additional loaded battery information
   * @param[inout]  the length of the data array given to application. On output, the number of bytes actually used.
   */
  int (*read_loaded_battery_level)(uint8_t *loaded_battery, uint8_t *data, uint8_t * data_len);

  /**
   * Tile ID -- 64-bit identifier for a Tile
   */
  uint8_t *tile_id;
  /**
   * Tile Auth Key -- 128-bit key for a Tile
   */
  uint8_t *tile_auth_key;
  /**
   * BLE MAC address -- 48-bit number
   */
  uint8_t *bdaddr;
  /**
   * Model Number -- 10 8-bit ASCII characters (null terminaison accepted but not required)
   */
  char *model_number;
  /**
   * Hardware Revision -- 5 8-bit ASCII characters (null terminaison accepted but not required)
   */
  char *hardware_version;
  /**
   * Chip Data -- size is given by chip_data_length (Max 255 Bytes)
   */
  uint8_t *chip_data;
  /**
   * Length of the Chip Data
   */
  uint8_t chip_data_length;
};


/**
 * Register the TMF module with Tile Library.
 */
int tile_tmf_register(struct tile_tmf_module *module);

/**
 * Unregister the TMF module with Tile Library.
 */
int tile_tmf_unregister(void);


#endif
