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
 * @file tile_storage.h
 * @brief Tile storage system
 */


#ifndef TILE_STORAGE_H_
#define TILE_STORAGE_H_


#include <stdint.h>
#include "modules/tile_tdt_module.h"
#include "modules/tile_tmd_module.h"
#include "tile_lib.h"

#define TILE_FDS_FILE_ID  0xFEED
#define TILE_FDS_REC_KEY  0xFEEC

#define TILE_DATA_STRUCT_VERSION 1

typedef struct {
  uint16_t version;
  uint8_t tile_id[TILE_ID_LEN];
  uint8_t auth_key[TILE_AUTH_KEY_LEN];
  char model_number[TILE_MODEL_NUMBER_LEN];
  char hardware_version[TILE_HARDWARE_VERSION_LEN];
  uint8_t mode;
  tdt_config_t tdt_configuration;
} tile_data_t;

extern tile_data_t tile_data;

void tile_storage_init(void);
void tile_store_app_data(void);


#endif
