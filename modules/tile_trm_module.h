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

/** @file tile_trm_module.h
 ** @brief Tile RSSI Measure module interface
 */

#ifndef TILE_TRM_MODULE_H_
#define TILE_TRM_MODULE_H_

#include <stdint.h>

/**
 ****************************************************************************************
 * @brief This function should be called in the application every time there is a
 *        new rssi value to be read. Based on the flag set for rssi notification,
 *        it will notify the rssi values once the sample size is reached.
 *
 * @param[in] rssi       rssi value every connection event when there is no
 *                       packet error.
 *
 ****************************************************************************************
 */
void tile_trm_rssi(int8_t rssi);
/**
 ****************************************************************************************
 * @brief This function enables trm feature
 ****************************************************************************************
 */
int tile_trm_register(void);

#endif
