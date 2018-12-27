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
 * @file tile_led.h
 * @brief LED Controller for Tile
 */


#ifndef TILE_LED_H_
#define TILE_LED_H_

/******************************************************************************
 *                       External Definitions
 ******************************************************************************/
extern uint32_t tile_led_id;
extern uint16_t tile_wiced_bt_app_hal_led_on_ms;
extern uint16_t tile_wiced_bt_app_hal_led_off_ms;
extern uint8_t  activation_song_flag;

/******************************************************************************
 *                       Function Prototypes
 ******************************************************************************/
void tile_wiced_bt_app_led_init (void);
void LedBlinkOn(void);
void LedBlinkOff(void);

#endif

