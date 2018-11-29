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

/** @file song.h
 ** @brief Tile Song. Control and program the song the Tile plays
 */

 #ifndef SONG_H_
 #define SONG_H_


#include <stdint.h>
#include "toa.h"

/** \defgroup SONG
This Feature manages Songs playing on the Tile.
Error codes returned by SONG (in format of @ref TOA_FEATURE_ERROR_CODES):
Error Code            | Description
--------------------- | -----------
::ERROR_UNSUPPORTED   | An unknown command was received
::ERROR_PARAMETERS    | A command with an invalid parameter was sent
::ERROR_INVALID_SIZE  | Attempted write of data with an invalid size
::ERROR_INVALID_STATE | Cannot perform requested operation in current state
::ERROR_DATA_LENGTH   | The received data size does not match the requested data size
::ERROR_MEM_READ      | Failed to read from flash
::ERROR_MEM_WRITE     | Failed to write to flash
::ERROR_INVALID_DATA  | Data received could not be validated
::ERROR_CRC           | Data read back from flash failed CRC check
::ERROR_CRC2          | Data received from app failed CRC check
::ERROR_SAME_IMAGE    | Song is the same as last programmed song
::ERROR_HASH          | Hash did not match
::ERROR_SIGNATURE     | Signature did not match
An optionnal subfeature of @ref SONG may be supported: Tile Programmable Songs Feature, see @ref TPS
*  @{
*/


/**
 * @brief TOA Song Command Codes
 */
enum TOA_SONG_CMD
{
  TOA_SONG_CMD_READ_FEATURES  = 0x01,
  /**< Discover what Song Commands are supported <br>
    Format: there is no parameter.
  */
  TOA_SONG_CMD_PLAY           = 0x02,
  /**< Song Command Play <br>

  Format:
  @ref TOA_SONG_CMD_PLAY Code | Song Number | Strength
  ---------------------- | ----------- | ------------
  1 Byte                 | 1 Byte (see @ref TILE_SONG) | 1 Byte (1: Low / 2: Med / 3:High)
  */
  TOA_SONG_CMD_STOP           = 0x03,
  /**< Song Command Stop <br>
    Format: there is no parameter.
  */
  TOA_SONG_CMD_PROGRAM        = 0x04,
  /**< Initialize song programming. Currently, only programming song number 1 (Find song)
  is supported.<br>
  This command is only supported when @ref TPS feature is supported <br>
  Format:
  @ref TOA_SONG_CMD_PROGRAM code | Song number | Song size (bytes)
  ------------------------------ | ----------- | -----------------
  1 Byte                         | 1 Byte      | 2 Bytes
  */
  TOA_SONG_CMD_DATA           = 0x05,
  /**< Send next chunk of data. There are ceil( (block_size+2) / chunk_size )
  chunks per block, except the final block may contain fewer chunks. Each
  block must be appended with the 16-bit CRC-CCITT, so the total data sent
  per block is block_size + 2. After a
  complete block has been received, the Tile will respond with a
  @ref TOA_SONG_RSP_BLOCK_OK message, unless the block is the final block,
  in which case it will respond with @ref TOA_SONG_RSP_PROGRAM_COMPLETE.<br>
  This command is only supported when @ref TPS feature is supported <br>

  Format:
  @ref TOA_SONG_CMD_DATA | Data
  ---------------------- | ----
  1 Byte                 | @ref TOA_MPS - 1 Bytes
  */
  TOA_SONG_CMD_READ_SONG_MAP  = 0x06,
  /**< Read the mapping of song numbers to song IDs for all songs with
  non-default tunes.
  This command is only supported when @ref TPS feature is supported <br>

  Format: There is no format
  */
};

/**
 * @brief TOA Song Response Codes
 */
enum TOA_SONG_RSP
{
  TOA_SONG_RSP_READ_FEATURES_OK = 0x01,
  /**< Response to @ref TOA_SONG_CMD_READ_FEATURES <br>
  Format:
  @ref TOA_SONG_RSP_READ_FEATURES_OK  | Supported features
  ----------------- | -----------
  1 Byte            | 1 Byte (bit 0 for @ref TOA_SONG_CMD_PLAY / bit 1 for @ref TOA_SONG_CMD_STOP / bit 2 for @ref TOA_SONG_CMD_PROGRAM and @ref TOA_SONG_CMD_DATA)
  */
  TOA_SONG_RSP_PLAY_OK          = 0x02,
  /**< Song Response Play OK <br>
  Format: there is no parameter.
  */
  TOA_SONG_RSP_STOP_OK          = 0x03,
  /**< Song Response Stop OK
  Format: there is no parameter.
  */
  TOA_SONG_RSP_PROGRAM_READY    = 0x04,
  /**< Tile is ready to program song.<br>
  This response is only supported when @ref TPS feature is supported <br>

  Format:
  @ref TOA_SONG_RSP_PROGRAM_READY | Block size
  ------------------------------- | ----------
  1 Byte                          | 1 Byte
  */
  TOA_SONG_RSP_BLOCK_OK         = 0x05,
  /**< Block was received and written to flash.<br>

  This response is only supported when @ref TPS feature is supported <br>

  Format: there is no parameter.
  */
  TOA_SONG_RSP_PROGRAM_COMPLETE = 0x06,
  /**< Song was programmed successfully.<br>
  This response is only supported when @ref TPS feature is supported <br>

  Format: there is no parameter.
  */
  TOA_SONG_RSP_SONG_MAP         = 0x07,
  /**< Map of song IDs for programmed songs. The response will
  be a sequence of (song number, song ID) pairs, indicating which
  songs are programmable as well as the ID of the song which is
  currently programmed.

  This response is only supported when @ref TPS feature is supported <br>

  Format:
  @ref TOA_SONG_RSP_SONG_ID | Song number | Song ID | ...
  ------------------------- | ----------- | ------- | ---
  1 Byte                    | 1 Byte      | 2 Bytes | ...
  */
  TOA_SONG_RSP_QUALITY          = 0x08,
  /**< Quality of the find song that just played. The response will have a
  "quality" value, which will be the millivolts difference between the battery
  voltage during a REST and the voltage during an 18kHz note. Format:
  @ref TOA_SONG_RSP_QUALITY | Quality
  ------------------------- | -------
  1 byte                    | 1 byte
  */
  TOA_SONG_RSP_ERROR            = 0x20,
  /**< An error occurred.<br>
  Format: format follows the format of @ref TOA_FEATURE_ERROR_CODES
  */

};



/** \defgroup TPS
Tile Programmable Songs Feature <br>
This subFeature of the Song feature manages Songs programming to the Tile. <br>
The following commands have been added to the @ref SONG feature for supporting @ref TPS :
 - @ref TOA_SONG_CMD_PROGRAM
 - @ref TOA_SONG_CMD_DATA
 - @ref TOA_SONG_CMD_READ_SONG_MAP
The following responses have been added to the @ref SONG feature for supporting @ref TPS :
 - @ref TOA_SONG_RSP_PROGRAM_READY
 - @ref TOA_SONG_RSP_BLOCK_OK
 - @ref TOA_SONG_RSP_PROGRAM_COMPLETE
 - @ref TOA_SONG_RSP_SONG_MAP
<br>
<br>
 * @section TSONG_FORMAT TSONG Format Description
 *
 * The Tsong file header consists of two pieces: the info
 * portion and the security portion, in that order. The info
 * portion includes some useful information about the song and the
 * security portion includes the hash and signature.
 *
 * The Song itself is a collection of (Note, Duartion) Pairs
<table>
<caption id="multi_row">TSONG FORMAT</caption>
<tr><th colspan="6">Header                                            <th colspan="6">Payload
<tr><td colspan="6">Size = 112                                               <td colspan="6">Size Varies, equals SongSize Bytes, or 2 * number of Note Pairs
<tr><td colspan="4">Info Header                  <td colspan="2">Security Header   <td colspan="6">Song Data
<tr><td colspan="4">Size = 6                     <td colspan="2">Size = 96          <td colspan="6">Size Varies, equals SongSize Bytes, or 2 * number of Note Pairs
<tr><td rowspan="2">SongFormat<td rowspan="2">SongNumber<td rowspan="2">SongId<td rowspan="2">SongSize  <td rowspan="2">SongHash<td rowspan="2">SongSignature       <td colspan="2">Note Pair 1  <td colspan="2">Note Pair 2  <td colspan="2">Note Pair N
<tr><td>Note 1<td>Duration 1<td>Note 2<td>Duration 2<td>Note N<td>Duration N
<tr><td>Size = 1     <td>Size = 1   <td>Size = 2 <td>Size = 2   <td>Size = 32     <td>Size = 64   <td>Size = 1     <td>Size = 1    <td>Size = 1     <td>Size = 1    <td>Size = 1     <td>Size = 1
</table>
<br>
<br>
 * @section SONG_ID SONG_ID Assigned Numbers
 *
 * A SongID uniquely identifies a Tile Song.
<table>
<caption id="multi_row">Assigned SongID</caption>
<tr><th>SongID                      <th>SongName
<tr><td>0                           <td>DutchSong
<tr><td>1                           <td>ImperialMarch
<tr><td>2                           <td>ScaleSong
<tr><td>3                           <td>BumblebeeSong
<tr><td>4                           <td>ClassicSong
</table>
*  @{
*/



/** @}*/

/** @}*/



TOA_EXTERN_DECL(void, toa_process_Song_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen));

/* TPS */
TOA_EXTERN_DECL(void, song_init, (void));


#endif
