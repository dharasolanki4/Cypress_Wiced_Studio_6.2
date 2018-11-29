
/*
 * NOTICE
 *
 * © 2014 Tile Inc.  All Rights Reserved.

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

#ifndef TDG_H_
#define TDG_H_


#include <stdint.h>
#include "toa.h"


/** \defgroup TDG
Tile Diagnostic Feature: <br>
- The Diagnostic Feature provides a scheme to download diagnostic data to the Cloud Server.
- The Diagnostic data SHALL be forwarded AS IS to the Cloud Server and SHALL not be modified by the TDG Client.
- There is no need for the TDG Client to have knowledge of the diagnostic format for it to transmit it to the Cloud Server.
- The TDG Client SHALL support a dynamic diagnostic data lenght (up to 1KB SHALL be supported by the TDG Client).
- The Diagnostic data SHALL be read each time a connection is established and then forwarded to the Cloud Server.
- In case the TDG Client cannot forward the Diagnostic Data to the Cloud Server, the Diagnostic Data SHALL be cached for later sending.
- In case cached Diagnostic Data grows too big, these MAY be flushed, but then the Cloud Server SHALL be notified of the flush when connected.
- The minimum size of diagnostic data cache the TDG Client SHALL support is 100 KB.
TDG returns the following error codes according to @ref TOA_FEATURE_ERROR_CODES format
  Error | Error Number | Description
  ------|--------------|------------
  @ref ERROR_UNSUPPORTED | 0x01 | An unrecognized command was sent to TDG

	For a description of the Diagnostic formats, check @ref DIAGNOSTIC_VERSIONS
*  @{
*/


/**
 * @brief TDG Command Codes
 */
enum TDG_CMD
{
  TDG_CMD_GET = 0x01,
  /**< This Command is used by the TDG Client to request Diagnostic Data <br>
  In response, the TDG Server will provide diagnostic data stream using @ref TDG_RSP_DATA_CONT responses<br>
  and finish diagnostic data stream with @ref TDG_RSP_DATA_END response.<br>
  Format: there is no parameter.
  */
};

/**
 * @brief TDG Response Codes
 */
enum TDG_RSP
{
  TDG_RSP_DATA_END = 0x01,
  /**< The Server sends this response last to send the last Bytes of diagnostic data. <br>
  */
  TDG_RSP_DATA_CONT = 0x02,
  /**< The Server sends multiple instances of this response to transmit diagnostic data after a @ref TDG_CMD_GET was received. <br>
  */
  TDG_RSP_ERROR = 0x20,
  /**< This response is sent when an error occurs in TDG */
};


/** @}*/


TOA_EXTERN_DECL(void, tdg_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen));

#endif  // TDG_H_
