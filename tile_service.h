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
 * @file tile_service.h"
 * @brief Core functionality for Tile Lib
 */


#ifndef TILE_SERVICE_H_
#define TILE_SERVICE_H_

#include "tile_lib.h"

#define OLD_CODE 0
/**
 * @brief Time before automatic disconnection if not authenticated
 */
//#define AUTHENTICATION_TIMER_DELAY_SHIPPING  (120 * TILE_TICKS_PER_SEC)
//#define AUTHENTICATION_TIMER_DELAY_ACTIVATED (30  * TILE_TICKS_PER_SEC)

void tile_service_init(void);
wiced_bt_gatt_status_t tile_gatts_callback( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data);
uint16_t tile_get_adv_uuid(void);

#endif

