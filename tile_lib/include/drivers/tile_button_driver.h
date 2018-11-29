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

/** @file tile_button_driver.h
 ** @brief Tile Button driver
 */

#ifndef TILE_BUTTON_DRIVER_H_
#define TILE_BUTTON_DRIVER_H_

#include <stdint.h>


/**
 * Button states
 */
enum TILE_BUTTON_STATES
{
  TILE_BUTTON_PRESSED,
  TILE_BUTTON_RELEASED,
};


/**
 * Tile button driver.
 */
struct tile_button_driver
{
  /**
   * Read the state of the Tile button.
   *
   * @param[out] button_state  State of the button. See @ref TILE_BUTTON_STATES.
   *
   * @return See @ref TILE_ERROR_CODES.
   */
  int (*read_state)(uint8_t *button_state);
};


/**
 * Register the button module.
 *
 * @param[in] driver Driver for the Tile button.
 *
 * @return TILE_SUCCESS.
 */
int tile_button_register(struct tile_button_driver *driver);


/**
 * Call when the Tile button has been pressed.
 *
 * @return TILE_SUCCESS.
 */
int tile_button_pressed(void);

#endif
