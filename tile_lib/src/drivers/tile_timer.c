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

 /** @file tile_timer.c
 ** @brief Tile Timer Driver
 */

#include "tile_lib.h"
#include "drivers/tile_timer_driver.h"
#include "drivers/tile_gap_driver.h"
#include "../toa/tcu.h"
#include "../toa/tdt.h"
#include "../toa/tka.h"
#include "../toa/test.h"
#include <stdint.h>
#include <stddef.h>

struct tile_timer_driver *timer = NULL;

extern struct tile_gap_driver *gap;

int tile_timer_register(struct tile_timer_driver *driver)
{
  /* TODO: Error checking? */
  timer = driver;

  return TILE_SUCCESS;
}


int tile_timer_expired(uint8_t timer_id)
{
  switch(timer_id)
  {
    case TILE_CONNECTION_TIMER:
      tcu_param_update(NULL);
      break;
    case TILE_AUTHENTICATION_TIMER:
      if(NULL != gap->auth_disconnect_count)
      {
        (*gap->auth_disconnect_count)++;
      }
      gap->gap_disconnect();
      break;
    case TILE_TDT_DOUBLETAP_TIMER:
      tdt_doubletap_timer_handler();
      break;
    case TILE_TDT_STI_TIMER:
      tdt_STI_timer_handler();
      break;
    case TILE_TDT_STD_TIMER:
      tdt_STD_timer_handler();
      break;
    case TILE_TDT_LT_TIMER:
      tdt_LT_timer_handler();
      break;
    case TILE_TDT_DT_TIMER:
      tdt_DT_timer_handler();
      break;
    case TILE_TDT_HDC_TIMER:
      tdt_HDC_timer_handler();
      break;
    case TILE_TCU_PARAM_UPDATE_TIMER:
      tcu_param_update_timer_handler();
      break;
    case TILE_TKA_TIMER1:
    case TILE_TKA_TIMER2:
    case TILE_TKA_TIMER3:
    case TILE_TKA_TIMER4:
    case TILE_TKA_TIMER5:
    case TILE_TKA_TIMER6:
    case TILE_TKA_TIMER7:
    case TILE_TKA_TIMER8:
      tka_check_time(timer_id - TILE_TKA_TIMER1);
      break;
    case TILE_TEST_TIMER1:
    case TILE_TEST_TIMER2:
    case TILE_TEST_TIMER3:
    case TILE_TEST_TIMER4:
    case TILE_TEST_TIMER5:
    case TILE_TEST_TIMER6:
    case TILE_TEST_TIMER7:
    case TILE_TEST_TIMER8:
      test_cmd_start_timer_handler(timer_id);
      break;
    default:
      /* TODO: Assert */
      break;
  }

  return TILE_SUCCESS;
}
