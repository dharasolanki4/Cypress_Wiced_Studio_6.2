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
 * @file tile_storage.h
 * @brief Tile storage system
 */


#ifndef TILE_STORAGE_H_
#define TILE_STORAGE_H_


#include <stdint.h>

#include "tile_lib.h"

#if OLD_CODE
#include "nrf_fstorage_sd.h"
#endif
/*****************************************/
/* Copied from nordic14, TO DO: find correct definitions for Nordic 15.2 */
#define PAGE_SIZE                     4096

/* These addresses should be the two pages directly before the default bootloader location */
#define APP_DATA_BANK0_ADDRESS        0x76000
#define APP_DATA_BANK1_ADDRESS        0x77000

//#define APP_DATA_BANK0_ADDRESS        0x7D000
//#define APP_DATA_BANK1_ADDRESS        0x7E000

#define APP_DATA_NUM_PAGES            1
/****************************************/

#define DEFAULT_ADVERTISING_INTERVAL  160
#define DEFAULT_TDT_DELAY             45 /**< 0.91 seconds */

#define PERSIST_SIGNATURE             0xA5A5
#define CHECKED_SIZE                  128
#define UNCHECKED_SIZE                256

#define CHECKED_STRUCTURE_VERSION_1   1
#define CHECKED_STRUCTURE_VERSION_2   2
#define CHECKED_STRUCTURE_VERSION_3   3
#define CHECKED_STRUCTURE_VERSION_4   4
#define CHECKED_STRUCTURE_VERSION     CHECKED_STRUCTURE_VERSION_1

#define APP_DATA_SAVE_VOLT_LIMIT      100 /**< 100 = 2.3V. Don't save app data if loaded measurement is beneath this limit. */
#define LOADED_BATTERY_WINDOW_SIZE    50
#define LOADED_BATTERY_WINDOW_TOTAL   6


extern const uint8_t interim_tile_id[];
extern const uint8_t interim_tile_key[];
extern const char tile_model_number[];
extern const char tile_hw_version[];

struct tile_checked_tag 
{
  /**************************************************************************************************/
  /*** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING   ****/
  /*** THIS STRUCTURE IS SAVED TO FLASH AND RETRIEVED AFTER TOFU                                 ****/
  /*** THIS MEANS STUFF SHOULD NOT BE MODIFIED BUT ONLY AT THE END TO MAINTAIN COMPATIBILITY     ****/
  /**************************************************************************************************/
  uint16_t        version;
  uint8_t         id;
  uint8_t         bank;
  uint8_t         mode;
  uint8_t         tile_id[TILE_ID_LEN];
  uint8_t         tile_auth_key[TILE_AUTH_KEY_LEN];
  char            model_number[TILE_MODEL_NUMBER_LEN];
  char            hardware_version[TILE_HARDWARE_VERSION_LEN];
  uint8_t         bdaddr[TILE_BDADDR_LEN];
};

struct tile_unchecked_tag 
{
  /**************************************************************************************************/
  /*** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING   ****/
  /*** THIS STRUCTURE IS SAVED TO FLASH AND RETRIEVED AFTER TOFU                                 ****/
  /*** THIS MEANS STUFF SHOULD NOT BE MODIFIED BUT ONLY AT THE END TO MAINTAIN COMPATIBILITY     ****/
  /**************************************************************************************************/

  // Activity tracking
  uint32_t  connection_count;           /**< number of connections */
  uint32_t  disconnect_count;           /**< Number of disconnections */
  uint8_t   auth_fail_count;            /**< authentication failures count */
  uint8_t   micFailures;                /**< mic failures */
  uint8_t   reset_count;                /**< Reset Count */
  uint32_t  piezoMs;                    /**< time for which piezo was active in '10 ms' units */

  // TOA Activity monitoring
  uint32_t  toa_channel_open_count;     /**< Number of successfull TOA Channel Open (with a successfull authentication) */
  uint32_t  toa_authenticate_count;     /**< number of TOA Authenticate Commands received */
  uint16_t  tka_closed_channel_count;   /**< number of TOA Channel close triggered by TKA */
  uint16_t  auth_disconnect_count;      /**< number of disconnections triggered by Auth Timer */
};

struct tile_persist_tag {
  uint16_t crc;
  uint16_t signature;
  union
  {
    struct tile_checked_tag s;
    uint8_t d[CHECKED_SIZE-4]; /* -4 for CRC + signature */
  } checked;
  union
  {
    struct tile_unchecked_tag s;
    uint8_t d[UNCHECKED_SIZE];
  } unchecked;
};


/**
 * @brief Persistent structure, which is saved to flash. Does not need to be
 *        accessed directly. Access elements with tile_checked and tile_unchecked.
 */
extern struct tile_persist_tag tile_persist; //__attribute__((section("NoInit")));

/**
 * @brief CRC checked portion of persistent data.
 */
extern struct tile_checked_tag * const tile_checked;

/**
 * @brief Non-CRC portion of persistent data. This get reinitialized when
 *        the CRC of the checked portion fails.
 */
extern struct tile_unchecked_tag * const tile_unchecked;
#if OLD_CODE
/**
 * @brief Tile environment data. Lost at reboot.
 */
struct tile_env_tag 
{
  uint16_t  last_reset_reason;      ///> Contains the reason for the last reset
  uint8_t   authorized;
};

extern struct tile_env_tag tile_env;

__packed struct tile_chip_data_tag
{
  uint32_t timestamp;
  uint8_t site_id;
};

//extern struct tile_chip_data_tag tile_chip_data;

void tile_storage_init(void);
void tile_store_app_data(void);
#endif
#endif
void tile_storage_init(void);
