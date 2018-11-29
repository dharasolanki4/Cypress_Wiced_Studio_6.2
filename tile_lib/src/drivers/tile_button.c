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

/** @file tile_button.c
 ** @brief Tile Button functionality
 */

#include "drivers/tile_button_driver.h"

#include "tile_lib.h"
#include "modules/tile_toa_module.h"
#include "../toa/toa.h"
#include "../toa/tdt.h"

#include <stdint.h>
#include <stddef.h>

struct tile_button_driver *button = NULL;

extern struct tile_toa_module *toa;

int tile_button_register(struct tile_button_driver *driver)
{
  /* TODO: Error checking? */
  button = driver;

  return TILE_SUCCESS;
}

int tile_button_pressed(void)
{
  if(NULL != toa && toa_get_feature(TOA_FEATURE_TDT))
  {
    tdt_process_button_press();
  }

  return TILE_SUCCESS;
}
