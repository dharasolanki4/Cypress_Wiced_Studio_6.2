/*
 * NOTICE
 *
 * © 2016 Tile Inc.  All Rights Reserved.
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
 * @file tile_features.h
 * @brief Support for features in Tile Lib
 */

#ifndef TILE_FEATURES_H_
#define TILE_FEATURES_H_

//#include "app_timer.h"
//#include "tile_gatt_db/tile_gatt_db.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_test_module.h"

#include "wiced.h"
#include "wiced_timer.h"

/**
 * @brief Number of TOA channels supported
 */
#define NUM_TOA_CHANNELS             8

/**
 * @brief Diagnostic Version
 */
#define DIAGNOSTIC_VERSION           80

/**
 * @brief Size of the TOA message buffer
 */
#define TOA_QUEUE_BUFFER_SIZE        (100 + 40 * (NUM_TOA_CHANNELS - 1))

typedef struct
{
  //tile_gatt_db_t service;
  wiced_bool_t tile_gatt_connection_status;
} tile_ble_env_t;

enum CUSTOM_EVENTS
{
  NOTIFICATION_WRITTEN_EVT
};

struct my_evt
{
  uint8_t type;
};

enum
{
  TEST_CMD_REBOOT = TILE_TEST_MODULE_CODE_BASE,
  TEST_CMD_STORAGE,
};

/**
 * @brief Types of reboots which can be triggered by \ref TEST_CMD_REBOOT
 */
enum TEST_REBOOT
{
  TEST_CMD_REBOOT_RESET         = 0x00,
  TEST_CMD_REBOOT_WATCHDOG      = 0x01,
  TEST_CMD_REBOOT_MEMORY_FAULT  = 0x02,
  TEST_CMD_REBOOT_OTHER         = 0x03,
  TEST_CMD_REBOOT_ASSERT        = 0x04,
  TEST_CMD_REBOOT_DURING_FLASH  = 0x05,
};

extern tile_ble_env_t tile_ble_env;
#if OLD_CODE
extern app_timer_id_t tile_timer[TILE_MAX_TIMERS];
#endif
extern wiced_timer_t tile_timer[TILE_MAX_TIMERS];
void tile_features_init(void);

#endif
