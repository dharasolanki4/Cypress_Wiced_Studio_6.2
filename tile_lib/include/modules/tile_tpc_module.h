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

/** @file tile_tpc_module.h
 ** @brief Tile Power Control module
 */

#ifndef TILE_TPC_MODULE_H_
#define TILE_TPC_MODULE_H_

#include <stdint.h>

/** \addtogroup TPC
 *  @{
 */

/**
 * @brief Configuration for TPC
 */
typedef struct {
  uint8_t enabled;     /**< 0 = TPC disabled, else TPC enabled */
  uint8_t lag;         /**< Number of consecutive measurements above high_level before power can be decreased */
  int8_t  low_level;   /**< Below this RSSI level, the device shall increase power if possible. */
  int8_t  high_level;  /**< Above this RSSI level, the device may decrease power */
} tpc_config_t;

/** @} */

typedef struct {
  int16_t adv_tx_limit;  /**< Advertising transmit power in 1 MHz bandwidth limit */
  int16_t con_tx_limit;  /**< Connected transmit power in 1 MHz bandwidth limit */
} txl_config_t;          /**< tx limit */

struct tile_tpc_module
{
  int (*get)(tpc_config_t *config);
  int (*set)(tpc_config_t *config);
  int (*set_tx_limits)(txl_config_t *config);  /**< Set the current tx power limits */
  int (*get_tx_limits)(txl_config_t *config);  /**< Get the current tx power limits */
  int (*get_tx_setting)(int16_t *adv_dbm, int16_t *con_dbm);  /**< Get the current adv and con tx power level setting */
  int (*get_tx_min_max)(int16_t *min_dbm, int16_t *max_dbm);  /**< Get the minimum and maximum possible transmit power */
};


int tile_tpc_register(struct tile_tpc_module *module);


#endif
