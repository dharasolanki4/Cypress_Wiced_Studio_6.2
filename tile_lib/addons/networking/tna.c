/*
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

/** @file tna.c
 ** @brief Tile Network API
 */
 
#include "tna.h"

// common
#include "tile_schedule/tile_schedule.h"
#include "tile_timers/tile_timers.h"
#include "tile_logs/tile_logs.h"

#include <string.h>


/******************************************************************************
 * Global functions
 *****************************************************************************/

/**
 * Format a TNA Frame.
 *
 * @param[in] dst       Destination buffer to store the formatted message. Must be 30+payload_length bytes.
 * @param[in] length    Length of the TNA SDUpayload (SDUlength).
 * @param[in] data      TNA SDUpayload.
 *
 * @return Total size of data put into dst.
 */
uint32_t TNA_formatFrame(uint8_t *dst, uint16_t length, uint8_t *data)
{
  uint64_t  timestamp = 1512165724; // This is the Current UNIX timestamp
  uint8_t   tile_id[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; // The TileID

  memcpy(&dst[0], "TileTileTile", 12);
  memcpy(&dst[12], &timestamp, 8);
  memcpy(&dst[20], tile_id, 8);
  memcpy(&dst[28], &length, 2);
  if(NULL != data && length > 0)
  {
    memcpy(&dst[30], data, length);
  }
  return 30 + length;
}

/**
 * Process a TNA Frame.
 * For now only TNA frames containing a single TNA command are supported.
 *
 * @param[in] datalen   Length of the TNA SDUpayload (SDUlength).
 * @param[in] data      TNA SDUpayload.
 *
 * @return Total size of data put into dst.
 */
uint32_t TNA_processFrame(uint16_t datalen, uint8_t *data)
{
  LOG("TNA_processFrame\r\n");

  if(datalen < 32)
  {
    /* Invalid packet. Ignore. */
    LOG("TNA ignored\r\n");
    return 1;
  }

  //TODO: Add filtering and security
  //TODO: Add TNA commands aggregation

  data    += 32;
  datalen -= 32;

  switch(RDLE16(&data[30]))
  {
    case TNA_RSP_LOOPBACK_REPLY:
      LOG("TNA_RSP_LOOPBACK_REPLY\r\n");
      break;

    case TNA_RSP_CONFIGURATION_REQUEST:
      LOG("TNA_RSP_CONFIGURATION_REQUEST\r\n");
      break;

    case TNA_RSP_DIAGNOSTIC_REQUEST:
      LOG("TNA_RSP_DIAGNOSTIC_REQUEST\r\n");
      break;

    case TNA_RSP_LOCATION_NOTIFICATION:
      LOG("TNA_RSP_LOCATION_NOTIFICATION\r\n");
      break;

    default:
      LOG("TNA default\r\n");
      break;
  }
  return 0;
}


/******************************************************************************
 * Internal functions
 *****************************************************************************/


