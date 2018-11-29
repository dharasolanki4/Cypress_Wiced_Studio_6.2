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


#ifndef NORDIC_SHIM_PLAYER_H
#define NORDIC_SHIM_PLAYER_H

#include <stdint.h>
#include <stdbool.h>


#define GPIOTE_SOUND_CHANNEL ((nrf_timer_cc_channel_t) 0) /**< PPI channel to use for connecting timer to piezo output */
#define PLAYER_TIMER_ID 1      /**< Timer ID to use with the player */

void InitPlayer(void);
int PlaySong(uint8_t number, uint8_t strength);
int StopSong(void);
bool SongPlaying(void);

#endif

