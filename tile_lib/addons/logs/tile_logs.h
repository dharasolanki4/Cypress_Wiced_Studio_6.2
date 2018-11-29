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

/** @file tile_log.h
 ** @brief Definitions for supporting TILE_LOGS
 */

#ifndef TILE_LOG_H_
#define TILE_LOG_H_

void tile_log_init(void);
void tile_log(const char fmt[], ...);

#define TILE_ENABLE_LOGS

/** 
 * @brief Log a message for debugging purposes.
 * The syntax is similar to a printf.
 * @warning Logging messages can affect the timing of your program.
 */
#ifdef TILE_ENABLE_LOGS
#define TILE_LOG(...) tile_log(__VA_ARGS__);
#else
#define TILE_LOG(...) /*Nothing*/
#endif

/**
 * @brief Init the Logging Feature.
 */
#ifdef TILE_ENABLE_LOGS
#define TILE_LOG_INIT() tile_log_init();
#else
#define TILE_LOG_INIT() /*Nothing*/
#endif

#endif //TILE_LOG_H_
