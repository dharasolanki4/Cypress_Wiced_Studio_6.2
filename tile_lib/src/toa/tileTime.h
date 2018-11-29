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

#ifndef TILE_TIME_H_
#define TILE_TIME_H_

#include <stdint.h>
#include <stdbool.h>
#include "toa.h"


#define TILE_TIME_INACCURACY_MAX 0xFFFF

/** \defgroup TIME
TIME is used to set and get time on the Tile. This feature
supports @ref TOA_COMMON. TIME is stored as a 64-bit integer
representing milliseconds since January 1, 2016 00:00:00 UTC.
Reads and writes to TIME will return a response with the following
format:
Field | Size
------|-----
Estimated inaccuracy (in seconds) | 2 bytes
Tile Time | 8 bytes
If the time has not previously been written, the estimated inaccuracy
will be 0xFFFF. The Tile will maintain the current time as best as it
can (subject to clock drift) after the time has been written. The estimated
inaccuracy will increment by 1 second every 3 hours, roughly corresponding
to a drift of 100ppm.
Note: On a reboot, the Tile will be able to maintain time only on
software resets. Hardware resets (using RST or when there is a
brownout) will cause the Tile time to fall behind by 0 to 2 seconds.
On any reboot, the estimated inaccuracy will increment by 2 seconds.
TIME returns the following error codes with @ref TOA_COM_RSP_ERROR
Error | Description
------|------------
@ref ERROR_PARAMETERS | Bad values given with command
@ref ERROR_UNSUPPORTED | An unrecognized command was sent
*/

TOA_EXTERN_DECL(void, time_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen));


#endif
