/*
 * NOTICE
 *
 * © 2017 Tile Inc.  All Rights Reserved.
 *
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

/**
 * @file tile_service.h
 * @brief Set up the Tile service
 */


#ifndef TILE_GATT_DB_H_
#define TILE_GATT_DB_H_

#include "drivers/tile_gatt_server_driver.h"
#include <stdint.h>

typedef struct
{
  uint16_t service_handle;
  uint16_t characteristic_handles[TILE_NUM_ATTRS];
} tile_gatt_db_t;

void tile_gatt_db_init(tile_gatt_db_t *p_service);

#endif
