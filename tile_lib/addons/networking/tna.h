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

/** @file tna.h
 ** @brief Tile Network API
 */


#ifndef TNA_H_
#define TNA_H_

#include <stdint.h>

/** \defgroup TNA
Tile Network API is the API used by Tile Nodes with direct network access (usually Cellular) <br>
 to communicate with Tile Server.

*  @{
*/

/**
 * @brief TNA Max Payload Size.
 * This is the maximum Payload that can be carried by a TNA Command or Response.
 * It excludes the TNA_CMD/TNA_RSP Code and excludes the MIC.
 */
#define TNA_MPS  200


/**
 * @brief TNA Response Codes<br>
 */
enum TNA_RSP
{
  TNA_RSP_RESERVED                = 0x00,
  TNA_RSP_LOOPBACK_REPLY          = 0x01,
  TNA_RSP_CONFIGURATION_REQUEST   = 0x02,
  TNA_RSP_DIAGNOSTIC_REQUEST      = 0x03,
  TNA_RSP_LOCATION_NOTIFICATION   = 0x04,
};

/**
 * @brief TNA Command Codes<br>
 */
enum TNA_CMD
{
  TNA_CMD_RESERVED          = 0x00,
  TNA_CMD_LOOPBACK_REQUEST  = 0x01,
  TNA_CMD_DIAGNOSTIC_EVENT  = 0x02,
  TNA_CMD_LOCATION_EVENT    = 0x03,
  TNA_CMD_WLAN_SNIFF_EVENT  = 0x04,
};

/** @}*/

/**
 * @brief Message packing helpers
 */

//TODO: move these somewhere else.

/** Extract a particular byte from an integer */
#define BYTE(a, n) (((a) >> ((n)*8)) & 0xFF)

/**
 * Convert an integer into the elements of a byte array, in little-Endian.
 * Example: uint8_t msg[] = {TOA_RSP_X, MKLE32(myUint32Var), MKLE16(myUint16Var)};
 */
#define MKLE16(x) BYTE(x, 0), BYTE(x, 1)
#define MKLE32(x) MKLE16(x),  MKLE16((x) >> 16)

/**
 * Write an integer into a byte array, in little-Endian.
 */
#define WRLE16(a, x) do {(a)[0] = (x) & 0xFF; (a)[1] = ((x) >> 8) & 0xFF;} while(0)
#define WRLE32(a, x) do {WRLE16(a, x); WRLE16((a) + 2, (x) >> 16);} while(0)

/**
 * @brief Message unpacking helpers
 */

/** Convert little-Endian uint8_t array to integer. */
#define RDLE16(a) ((a)[0]    | ((a)[1] << 8))
#define RDLE32(a) (RDLE16(a) | (RDLE16((a)+2) << 16))



/******************************************************************************
 * TNA API functions
 *****************************************************************************/

uint32_t TNA_formatFrame(uint8_t *dst, uint16_t payload_length, uint8_t *payload);
uint32_t TNA_processFrame(uint16_t payload_length, uint8_t *payload);

#endif /* TNA_H_ */

