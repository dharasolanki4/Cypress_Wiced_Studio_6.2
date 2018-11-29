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

/** @file tile_schedule_test.c
 ** @brief test the schedule implementation
 */
 
#include "schedule/tile_schedule.h"
#include "logs/tile_logs.h"

static void tile_schedule_test_handler(void * arg)
{
  if ((size_t)arg == TILE_SCHEDULE_QUEUE_SIZE)
  {
    TILE_LOG("tile_schedule_test(): success! %d items run!\n", (size_t)arg);
  }
}

void tile_schedule_test(void)
{
  bool success;
  size_t k = 0;
  
  do
  {
    success = tile_schedule(tile_schedule_test_handler, (void *)++k);
  } while(success);
}
