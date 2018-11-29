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
 * @file tile_storage.c
 * @brief Tile storage system
 */


#include "tile_storage.h"
#include "tile_config.h"
#include "modules/tile_tmd_module.h"

#include "fds.h"
#include "app_error.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


/*******************************************************************************
 * Global variables
 ******************************************************************************/
tile_data_t tile_data;


/*******************************************************************************
 * Local variables
 ******************************************************************************/

static uint32_t buffer[(sizeof(tile_data) + 3) / 4];


/*******************************************************************************
 * Forward declarations
 ******************************************************************************/

static void tile_data_init(void);


/*******************************************************************************
 * Global functions
 ******************************************************************************/

void tile_storage_init(void)
{
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;

  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  // Loop until all records with the given key and file ID have been found.
  if(fds_record_find(TILE_FDS_FILE_ID, TILE_FDS_REC_KEY, &record_desc, &ftok) != FDS_SUCCESS)
  {
    fds_record_t record;
    fds_record_chunk_t record_chunk;

    tile_data_init();

    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &tile_data, sizeof(tile_data));

    // Set up data.
    record_chunk.p_data         = buffer;
    record_chunk.length_words   = sizeof(buffer) / 4;

    // Set up record.
    record.file_id              = TILE_FDS_FILE_ID;
    record.key                  = TILE_FDS_REC_KEY;
    record.data.p_chunks        = &record_chunk;
    record.data.num_chunks      = 1;

    ret_code_t ret = fds_record_write(NULL, &record);
    if (ret != FDS_SUCCESS)
    {
      APP_ERROR_CHECK_BOOL(false);
    }
  }
  else
  {
    fds_flash_record_t  flash_record;

    if (fds_record_open(&record_desc, &flash_record) != FDS_SUCCESS)
    {
      APP_ERROR_CHECK_BOOL(false);
    }

    memcpy(&tile_data, flash_record.p_data, sizeof(tile_data));

    if (fds_record_close(&record_desc) != FDS_SUCCESS)
    {
      APP_ERROR_CHECK_BOOL(false);
    }

    if(tile_data.version != TILE_DATA_STRUCT_VERSION)
    {
      tile_data_init();
      tile_store_app_data();
    }
  }
}


/**
 * @brief Save tile_persist to flash
 */
void tile_store_app_data(void)
{
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;

  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  // Loop until all records with the given key and file ID have been found.
  if(fds_record_find(TILE_FDS_FILE_ID, TILE_FDS_REC_KEY, &record_desc, &ftok) != FDS_SUCCESS)
  {
    // Should never happen
    APP_ERROR_CHECK_BOOL(false);
    return;
  }

  fds_record_t record;
  fds_record_chunk_t record_chunk;

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, &tile_data, sizeof(tile_data));

  // Set up data.
  record_chunk.p_data         = buffer;
  record_chunk.length_words   = sizeof(buffer) / 4;

  // Set up record.
  record.file_id              = TILE_FDS_FILE_ID;
  record.key                  = TILE_FDS_REC_KEY;
  record.data.p_chunks        = &record_chunk;
  record.data.num_chunks      = 1;

  ret_code_t ret = fds_record_update(&record_desc, &record);
  if (ret != FDS_SUCCESS)
  {
    APP_ERROR_CHECK_BOOL(false);
  }

  // TODO: This is inefficient, but not necessary to fix.
  fds_gc();
}


/*******************************************************************************
 * Local functions
 ******************************************************************************/
static void tile_data_init(void)
{
  // Initialize
  memset(&tile_data, 0, sizeof(tile_data));
  tile_data = (tile_data_t){
    .version = TILE_DATA_STRUCT_VERSION,
    .mode = TILE_DEFAULT_MODE,
#if TILE_ENABLE_BUTTON
    .tdt_configuration = TILE_DEFAULT_TDT_CONFIG,
#endif
#if TILE_USE_HARDCODED_ID
    .tile_id = HARDCODED_TILE_ID,
    .auth_key = HARDCODED_AUTH_KEY,
    .model_number = HARDCODED_MODEL_NUMBER,
    .hardware_version = HARDCODED_HARDWARE_VERSION,
#endif
  };
}

