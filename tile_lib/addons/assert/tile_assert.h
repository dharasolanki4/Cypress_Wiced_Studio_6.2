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

/** @file tile_assert.h
 ** @brief Define a standard Tile assert interface
 */

#ifndef TILE_ASSERT_H_
#define TILE_ASSERT_H_

#include <stdint.h>
#include <stdbool.h>

void tile_assert(bool cond, uint32_t line, const char file[], const char func[], bool ignore);

#define TILE_ASSERT(cond)         tile_assert(cond, __LINE__, __FILE__, __func__, false)
#define TILE_ASSERT_IGNORE(cond)  tile_assert(cond, __LINE__, __FILE__, __func__, true)

#endif
