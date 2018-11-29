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

/**
 * Timer prescaler. Set to same value as in main.c.
 */
#define APP_TIMER_PRESCALER 0

/**
 * Hard code a Tile ID. Populate with a test Tile ID provided by Tile.
 */
#ifndef TILE_USE_HARDCODED_ID
  #define TILE_USE_HARDCODED_ID 1
#endif

#if TILE_USE_HARDCODED_ID
  #define HARDCODED_TILE_ID {0x1a, 0x95, 0xd9, 0x97, 0xf0, 0xf2, 0x66, 0x07}
  #define HARDCODED_AUTH_KEY {0x14, 0x27, 0xe3, 0x03, 0xa2, 0x51, 0xc5, 0xb5, 0x07, 0x2a, 0xa9, 0x81, 0xa9, 0x42, 0x8a, 0x43}
  #define HARDCODED_MODEL_NUMBER "TEST 00.00"
  #define HARDCODED_HARDWARE_VERSION "01.00"
#endif

/**
 * Define a firmware version for your version. Must conform to the format
 *   "XX.XX.XX.X"
 */
#define TILE_FIRMWARE_VERSION "02.00.00.0"

/**
 * Use Tile's manufacturing mode to write Tile unique numbers over the air
 */
#ifndef TILE_USE_MANUFACTURING_MODE
  #define TILE_USE_MANUFACTURING_MODE 0
#endif

#if TILE_USE_MANUFACTURING_MODE
  #define TILE_DEFAULT_MODE TILE_MODE_MANUFACTURING
#else
  #define TILE_DEFAULT_MODE TILE_MODE_SHIPPING
#endif

/**
 * Enable use of Tile button.
 */
#ifndef TILE_ENABLE_BUTTON
  #define TILE_ENABLE_BUTTON 1
#endif

#if TILE_ENABLE_BUTTON
  #define TILE_BUTTON BUTTON_4
  #define TILE_DEFAULT_TDT_CONFIG (tdt_config_t) { \
      .Delay       = 45,                           \
      .EN_DT       = true,                         \
      .SE_DTF      = true,                         \
      .SE_DTS      = true,                         \
      .FS_Strength = 1,                            \
      .SS_Strength = 1,                            \
    }
#endif

/**
 * Enable use of Tile player
 */
#ifndef TILE_ENABLE_PLAYER
  #define TILE_ENABLE_PLAYER 1
#endif

#if TILE_ENABLE_PLAYER
  #define TILE_BUZZER_PIN 3
  #define TILE_PLAYER_USE_AMPLIFIER 0
  #if TILE_PLAYER_USE_AMPLIFIER
    #define TILE_AMP_POWER_ON_PIN 5
    #define TILE_AMP_POWER_ON_VAL 0
    #define TILE_AMP_EN1_PIN 6
    #define TILE_AMP_EN2_PIN 7
  #endif
#endif

#endif
