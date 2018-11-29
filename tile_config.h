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

/** @file tile_config.h
 ** @brief Configuration for Tile functionality
 */

#ifndef TILE_CONFIG_H_
#define TILE_CONFIG_H_

#define INTERIM_TILE_ID {0x91, 0x19, 0xaa, 0xd0, 0x6f, 0xe8, 0x33, 0xd6}
#define INTERIM_AUTH_KEY {0xd7, 0x6d, 0x07, 0x0f, 0x61, 0x28, 0x22, 0x9b, 0x5b, 0x20, 0x8c, 0xd8, 0xe5, 0x9e, 0x45, 0x98}

#define TILE_MODEL_NUMBER "TEST 02.00"
#define TILE_HARDWARE_VERSION "00.01"

#define TILE_DEFAULT_MODE TILE_MODE_SHIPPING

#define TILE_ENABLE_PLAYER 1

#endif
