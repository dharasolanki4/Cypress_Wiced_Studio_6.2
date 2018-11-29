/*
 * NOTICE
 *
 * © 2016 Tile Inc.  All Rights Reserved.

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

 /** @file tmd.h
 ** @brief Tile Mode
 */

#ifndef TMD_H_
#define TMD_H_

#include <stdint.h>
#include "toa.h"


/** \defgroup TMD
- TMD is the feature to Read and Write Tile Mode <br>
- TMD Supports @ref TOA_COMMON Codes <br>
- TMD Currently Supports the following Commands:
  - @ref TOA_COM_CMD_READ_VAL
  - @ref TOA_COM_CMD_WRITE_VAL
- TMD returns the following error codes with @ref TOA_COM_RSP_ERROR
  Error | Error Number | Description
  ------|--------------|------------
  @ref ERROR_PARAMETERS | 0x02 | A bad parameter value has been passed to TMD
  @ref ERROR_UNSUPPORTED | 0x01 | An unrecognized command was sent to TMD
*  @{
*/

/** @}*/
TOA_EXTERN_DECL(void, tmd_process_command, (const uint8_t cid, const uint8_t *data, uint8_t datalen));

#endif
