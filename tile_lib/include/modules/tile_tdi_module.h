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

/** @file tile_tdi_module.h
 ** @brief Tile Device Information Module. Provides TileLib Device specific information and unique numbers.
 */

#ifndef TILE_TDI_MODULE_H_
#define TILE_TDI_MODULE_H_

#include <stdint.h>


/**
 * Tile Device Information module.
 *
 * This module is used by Tile Lib to allow the Tile app to read some
 * information about the device in order to properly activate and authenticate
 * with the device.
 */
struct tile_tdi_module
{
  /**
   * Tile ID -- 64-bit identifier for Tile Nodes.
   *			Example: {0x1a, 0x95, 0xd9, 0x97, 0xf0, 0xf2, 0x66, 0x07}.
   */
  uint8_t *tile_id;
  /**
   * BLE MAC address -- 48-bit number. Points to the MAC address advertised Over The Air.
   */
  uint8_t *bdaddr;
  /**
   * Firmware Version -- 10 8-bit ASCII characters (null terminaison accepted but not required)
   *	Format: "xx.xx.xx.x"
   *	Example: "02.00.00.0"
   */
  char *firmware_version;
  /**
   * Model Number -- 10 8-bit ASCII characters (null terminaison accepted but not required)
   *  Format shall follow the following pattern: "XXXX YY.YY" with the following constraints:
   *		- "XXXX" uses 4 ASCII letters ('A' to 'Z') to describe the Vendor ID.
   *    - The Vendor ID is assigned by Tile.
   *    - A space character after "XXXX".
   *    - "YY.YY" uses 4 ASCII numbers ('0' to '9') and describes the Model ID.
   *  Example: "TEST 00.00".

   */
  char *model_number;
  /**
   * Hardware Revision -- 5 8-bit ASCII characters (null terminaison accepted but not required)
   *  The character pattern is "YY.YY" and uses 4 ASCII numbers ('0' to '9').
   *  Example: "01.00".
   */
  char *hardware_version;
};


/**
 * Register the TDI module with Tile Library.
 */
int tile_tdi_register(struct tile_tdi_module *module);


#endif
