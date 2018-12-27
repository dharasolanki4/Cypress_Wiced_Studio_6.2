/*
 * NOTICE
 *
 * Â© 2017 Tile Inc.  All Rights Reserved.
 *
 * All code or other information included in the accompanying files (â€œTile Source Materialâ€�)
 * is CONFIDENTIAL and PROPRIETARY information of Tile Inc. and your access and use is subject
 * to the terms of Tileâ€™s non-disclosure agreement as well as any other applicable agreement
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
 * @file tile_led.c
 * @brief LED Controller for Tile
 *        Modified wiced_bt_app_hal_common.c for Tile LED Control
 */

#include "wiced_bt_app_hal_common.h"
#include "wiced_timer.h"
#ifdef  WICED_BT_TRACE_ENABLE
#include "wiced_bt_trace.h"
#endif

#include "tile_storage.h"
#include "modules/tile_tmd_module.h"

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
void tile_led_timer_cb( uint32_t param );

/*******************************************************************************
 * Defines & types
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
uint32_t tile_led_id                      = (uint32_t)WICED_PLATFORM_LED_1;
uint16_t tile_wiced_bt_app_hal_led_on_ms  = 150;
uint16_t tile_wiced_bt_app_hal_led_off_ms = 150;
uint8_t activation_song_flag              = 0;
wiced_timer_t tile_led_timer;

/*******************************************************************************
 * Local variables
 ******************************************************************************/


/*******************************************************************************
 * Functions
 ******************************************************************************/

/**
 * @brief Wiced BT App Tile LED Initialization
 */
void tile_wiced_bt_app_led_init ( void )
{
    wiced_init_timer( &tile_led_timer, tile_led_timer_cb, 0, WICED_MILLI_SECONDS_TIMER );

    platform_led_init();
}

/**
 * @brief Timer CallBack to Control LED Blinking to reflect Tile States
 */
void tile_led_timer_cb( uint32_t param )
{
  WICED_BT_TRACE("LED Callback\r\n");
  static uint8_t led_on = 1;

  if ( led_on )
  {
    wiced_bt_app_hal_led_off(tile_led_id);
    led_on = 0;
    wiced_start_timer( &tile_led_timer, tile_wiced_bt_app_hal_led_off_ms );
  }
  else
  {
    led_on = 1;
    wiced_bt_app_hal_led_on(tile_led_id);
    wiced_start_timer( &tile_led_timer, tile_wiced_bt_app_hal_led_on_ms );
  }
}

/**
 * @brief Turn LED On, Start Timer if blinking is required
 */
void LedBlinkOn(void)
{
  if (activation_song_flag && (TILE_MODE_ACTIVATED == tile_checked->mode)) /* If playing Activation Song, Leave the LED On, do not blink */
  {
    wiced_bt_app_hal_led_on(tile_led_id);
    activation_song_flag = 0;
  }
  else
  {
    wiced_bt_app_hal_led_on(tile_led_id);
    wiced_start_timer(&tile_led_timer,tile_wiced_bt_app_hal_led_on_ms); /* Timer Callback has logic to blink the LED */
  }
}

/**
 * @brief Turn LED Blinking Off by stopping timer, and leaves LED On or Off
 */
void LedBlinkOff(void)
{
  if(TILE_MODE_ACTIVATED == tile_checked->mode) // Activated, leave it On
  {
    wiced_bt_app_hal_led_on(tile_led_id);
    wiced_stop_timer(&tile_led_timer);
  }
  else
  {
    wiced_bt_app_hal_led_off(tile_led_id);
    wiced_stop_timer(&tile_led_timer);
  }
}
