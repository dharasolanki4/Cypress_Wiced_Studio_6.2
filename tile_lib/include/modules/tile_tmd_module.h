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

/** @file tile_tmd_module.h
 ** @brief Tile Mode module interface. Controles Tile Mode.
 */

#ifndef TILE_TMD_MODULE_H_
#define TILE_TMD_MODULE_H_

#include <stdint.h>

/** \addtogroup TMD
 *  @{
 */
/**
 * @brief TILE_MODE <br>
this 1 Byte value is used as parameters of the following @ref TMD Commands and Responses:
  - @ref TOA_COM_CMD_WRITE_VAL
  - @ref TOA_COM_RSP_READ_VAL_OK
  - @ref TOA_COM_RSP_WRITE_VAL_OK
 */
enum TILE_MODE
{
  TILE_MODE_MANUFACTURING = 0x0,
  TILE_MODE_SHIPPING      = 0x1,
  TILE_MODE_ACTIVATED     = 0x2
};

/** @} */


/**
 * Tile Mode module.
 *
 * This module is used by Tile Lib to get and set the mode.
 */
struct tile_tmd_module
{
  /**
   * Get the current mode
   */
  int (*get)(uint8_t *mode);

  /**
   * Set the mode. Value should be saved in NVM.
   */
  int (*set)(uint8_t mode);
};


/**
 * Register the TMD module
 */
int tile_tmd_register(struct tile_tmd_module *module);


#endif
