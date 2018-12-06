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

/** @file song.c
 ** @brief Tile Song. Control and program the song the Tile plays
 */

#include "song.h"
#include "tile_lib.h"
#include "toa.h"
#include "../tileHash.h"
//#include "tileAssert.h"
#include "../crc.h"

#include "modules/tile_song_module.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))


static void song_send_error(uint8_t cid, uint8_t cmd, uint8_t err);
static void song_process_data(const uint8_t* data, uint8_t datalen);
static void song_process_block(void);
static void song_prepare_program(uint8_t cid, uint8_t song_number, uint16_t song_file_size);

/* TODO: Find a better way to do this */
static uint8_t last_song_cid = TOA_CONNECTIONLESS_CID;


/**
 * @enum SONG Programming State
 */
enum
{
  SONG_PROGRAM_STATE_NO_ACTIVITY, ///< SONG Programming has not been started
  SONG_PROGRAM_STATE_READY,       ///< SONG Programming Server is accepting data
  SONG_PROGRAM_STATE_MEM,         ///< SONG Programming Server is processing/saving data and is NOT accepting data
};


struct tile_song_module *song;



/**
 ****************************************************************************************
 * @brief Process Incomming SONG Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, toa_process_Song_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  uint8_t index;
  uint8_t strength;

  switch(data[0])
  {
    case TOA_SONG_CMD_READ_FEATURES:
    {
      uint8_t rsp[2] = {TOA_SONG_RSP_READ_FEATURES_OK, 7};
      toa_send_response(cid, TOA_RSP_SONG, &rsp[0], 2);
    }
    break;
    case TOA_SONG_CMD_PLAY:
      if(datalen < 3)
      {
        song_send_error(cid, data[0], TOA_ERROR_PARAMETERS);
      }
      else
      {
        index = data[1];

        if (index < TILE_SONG_MAX)
        {
          uint8_t rsp[1] = {TOA_SONG_RSP_PLAY_OK};
          strength = data[2];

          last_song_cid = cid;
          song->play(index, strength);
          toa_send_response(cid, TOA_RSP_SONG, &rsp[0], 1);
        }
        else
        {
          song_send_error(cid, data[0], TOA_ERROR_PARAMETERS);
        }
      }
    break;
    case TOA_SONG_CMD_STOP:
    {
      uint8_t rsp[1] = {TOA_SONG_RSP_STOP_OK};
      song->stop();
      toa_send_response(cid, TOA_RSP_SONG, &rsp[0], 1);
    }
    break;
    case TOA_SONG_CMD_PROGRAM:
    {
      if(TOA_CONNECTIONLESS_CID != song->tps_module->state.cached_cid
        && cid != song->tps_module->state.cached_cid)
      {
        /* Someone else is running a TPS */
        song_send_error(cid, data[0], TOA_ERROR_RESOURCE_IN_USE);
        return;
      }

      if(datalen < 4)
      {
        song_send_error(cid, data[0], TOA_ERROR_PARAMETERS);
      }
      else
      {
        uint16_t song_file_size;
        uint8_t  song_number = data[1];
        if(song_number >= TILE_SONG_MAX)
        {
          song_send_error(cid, data[0], TOA_ERROR_PARAMETERS);
        }
        else if(song_number != TILE_SONG_FIND)
        {
          song_send_error(cid, data[0], TOA_ERROR_UNSUPPORTED);
        }
        else
        {
          memcpy(&song_file_size, &data[2], 2);
          song_prepare_program(cid, song_number, song_file_size);
        }
      }
    }
    break;
    case TOA_SONG_CMD_DATA:
    {
      if(cid != song->tps_module->state.cached_cid)
      {
        /* Someone else is running a TPS */
        song_send_error(cid, data[0], TOA_ERROR_RESOURCE_IN_USE);
        return;
      }

      if(datalen < 2)
      {
        song_send_error(cid, data[0], TOA_ERROR_PARAMETERS);
      }
      else
      {
        song_process_data(&data[1], datalen - 1);
      }
    }
    break;
    case TOA_SONG_CMD_READ_SONG_MAP:
    {
      uint8_t datasize = 4;
      /* Default to song ID 0, if no song has been previously programmed */
      uint8_t rsp[4] = {TOA_SONG_RSP_SONG_MAP, TILE_SONG_FIND, 0, 0};
      if(true == song->tps_module->useProgrammableSong)
      {
        memcpy(&rsp[2], (uint8_t*)&song->tps_module->song_info_cache.curInfo.song_id, 2);
      }
      toa_send_response(cid, TOA_RSP_SONG, rsp, datasize);
    }
    break;
    default:
    {
      song_send_error(cid, data[0], TOA_ERROR_UNSUPPORTED);
    }
    break;
  }
}


/**
 ****************************************************************************************
 * @brief Prepare to program a new find song. Initializes song_program_state.
 *
 * @param[in] song_number Song number to program.
 * @param[in] song_length Length of new song file.
 *
 ****************************************************************************************
 */
void song_prepare_program(uint8_t cid, uint8_t song_number, uint16_t song_file_size)
{
  if(SONG_PROGRAM_STATE_MEM == song->tps_module->state.state)
  {
    /* This is the only state where you cannot send a program command */
    song_send_error(cid, TOA_SONG_CMD_PROGRAM, TOA_ERROR_INVALID_STATE);
    return;
  }
  /* Song must fit in flash section and its length must be even (note, duration) pairs */
  if(song_file_size > (TILE_PROGRAMMABLE_SONG_LENGTH - SONG_METADATA_SIZE) ||
    ((song_file_size - SONG_HEADER_SIZE) & 1) != 0)
  {
    song_send_error(cid, TOA_SONG_CMD_PROGRAM, TOA_ERROR_INVALID_SIZE);
    return;
  }
  /* Song must contain at least a header */
  if(song_file_size < SONG_HEADER_SIZE)
  {
    song_send_error(cid, TOA_SONG_CMD_PROGRAM, TOA_ERROR_INVALID_SIZE);
    return;
  }

  // init everything
  memset(&song->tps_module->state, 0, sizeof(song->tps_module->state));

  // configure
  song->tps_module->state.cached_cid  = cid;
  song->tps_module->state.file_size   = song_file_size;
  sha256_init_tile(&song->tps_module->state.hash_ctx);

  /* Let the Application know */
  song->tps_module->begin();
}


/**
 ****************************************************************************************
 * @brief Application is ready (or not) to begin receiving a song
 *
 * @param[in] error: 0 means OK, !=0 means MEM_WRITE error
 *
 ****************************************************************************************
 */
void song_begin_done(uint8_t error)
{
  if(0 == error)
  {
    song->tps_module->state.state = SONG_PROGRAM_STATE_READY;
    uint8_t rsp[] = {TOA_SONG_RSP_PROGRAM_READY, TILE_SONG_BLOCK_SIZE};
    toa_send_response(song->tps_module->state.cached_cid, TOA_RSP_SONG, &rsp[0], sizeof(rsp));
  }
  else
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_PROGRAM, TOA_ERROR_MEM_WRITE);
  }
}

/**
 ****************************************************************************************
 * @brief process incomming TPS Data packets
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void song_process_data(const uint8_t* data, uint8_t datalen)
{
  if(SONG_PROGRAM_STATE_READY != song->tps_module->state.state)
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_INVALID_STATE);
    return;
  }
  if((song->tps_module->state.buf_pos + datalen) > TILE_SONG_BUFFER_SIZE)
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_DATA_LENGTH);
    return;
  }

  memcpy(&song->tps_module->tileSongBuffer[song->tps_module->state.buf_pos], data, datalen);
  song->tps_module->state.buf_pos += datalen;

  /* If buffer is full or reached end of song */
  if(song->tps_module->state.buf_pos == TILE_SONG_BUFFER_SIZE ||
    (song->tps_module->state.file_size - song->tps_module->state.pos) ==
      (song->tps_module->state.buf_pos - SONG_CRC16_SIZE))
  {
    song->tps_module->state.state = SONG_PROGRAM_STATE_MEM;
    song_process_block();
  }
  else
  {
    // nothing to do: keep receive chunks
  }
}

/**
 ****************************************************************************************
 * @brief process incomming TPS block
 *
 * @return    void
 *
 ****************************************************************************************
 */
static void song_process_block(void)
{
  song->tps_module->state.block_dataSize = MIN(TILE_SONG_BUFFER_SIZE - SONG_CRC16_SIZE,
    song->tps_module->state.file_size - song->tps_module->state.pos);
#if 0
  if(0 != crc16(0, song->tps_module->tileSongBuffer,
      song->tps_module->state.block_dataSize + SONG_CRC16_SIZE))
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_CRC2);
    return;
  }
#endif
  /* Special treatment for the first block */
  if(0 == song->tps_module->state.pos)
  {
    memcpy(&song->tps_module->state.info, song->tps_module->tileSongBuffer, SONG_INFO_SIZE);
    memcpy(&song->tps_module->state.sec, &song->tps_module->tileSongBuffer[SONG_INFO_SIZE],
      SONG_SECURITY_SIZE);

    if(song->tps_module->song_info_cache.curMeta.valid == TILE_SONG_VALID)
    {
      if(song->tps_module->state.info.song_id == song->tps_module->song_info_cache.curInfo.song_id)
      {
        song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_SAME_IMAGE);
        return;
      }
    }

    /* Include important information in hash */
    sha256_update_tile(&song->tps_module->state.hash_ctx,
      (uint8_t*)&song->tps_module->state.info, SONG_INFO_SIZE);

    /* Sha the song data portion of the first block */
    uint8_t songDataSize = song->tps_module->state.block_dataSize - SONG_HEADER_SIZE;
    sha256_update_tile(&song->tps_module->state.hash_ctx,
      &song->tps_module->tileSongBuffer[SONG_HEADER_SIZE], songDataSize);
  }
  else
  {
    /* Sha the whole buffer */
    sha256_update_tile(&song->tps_module->state.hash_ctx, song->tps_module->tileSongBuffer,
      song->tps_module->state.block_dataSize);
  }
  /* Let Application save the block */
  song->tps_module->block_ready();
}


/**
 ****************************************************************************************
 * @brief Application has finished processing a block
 *
 * @param[in] error: 0 means OK, !=0 means MEM_WRITE error
 *
 ****************************************************************************************
 */
void song_block_done(uint8_t error)
{
  if(0 != error)
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_MEM_WRITE);
//    TILE_ASSERT_IGNORE(false);
    return;
  }

  if(0 != crc16(0, song->tps_module->tileSongBuffer,
      song->tps_module->state.block_dataSize + SONG_CRC16_SIZE))
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_CRC);
    return;
  }

  song->tps_module->state.pos += song->tps_module->state.block_dataSize;

  if(song->tps_module->state.pos == song->tps_module->state.file_size) /* We have a complete song */
  {
    uint8_t hash[SONG_HASH_SIZE];
    sha256_final_tile(&song->tps_module->state.hash_ctx, hash);
    if(0 != memcmp(hash, song->tps_module->state.sec.hash, SONG_HASH_SIZE))
    {
      /* Hash doesn't match */
      song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_HASH);
      return;
    }
    if (1 != tile_sign_verify(song->tps_module->pub_key, hash, song->tps_module->state.sec.sign))
    {
      /* Signatures don't match */
      song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_SIGNATURE);
      return;
    }

    /* Make sure song ends */
    if(song->tps_module->tileSongBuffer[song->tps_module->state.block_dataSize-1] != REST ||
      song->tps_module->tileSongBuffer[song->tps_module->state.block_dataSize-2] != REST)
    {
      song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_INVALID_DATA);
      return;
    }
    /* Let Application complete the song properly */
    song->tps_module->complete();
  }
  else /* Song is not complete yet. */
  {
    song->tps_module->state.buf_pos = 0;
    song->tps_module->state.state = SONG_PROGRAM_STATE_READY;

    uint8_t rsp[] = {TOA_SONG_RSP_BLOCK_OK};
    toa_send_response(song->tps_module->state.cached_cid, TOA_RSP_SONG, &rsp[0], sizeof(rsp));
  }
}

/**
 ****************************************************************************************
 * @brief Application has finished processing a complete song
 *
 * @param[in] error: 0 means OK, !=0 means MEM_WRITE error
 *
 ****************************************************************************************
 */
void song_complete_done(uint8_t error)
{
  if(0 == error)
  {
    uint8_t rsp[] = {TOA_SONG_RSP_PROGRAM_COMPLETE};
    toa_send_response(song->tps_module->state.cached_cid, TOA_RSP_SONG, &rsp[0], sizeof(rsp));
  }
  else
  {
    song_send_error(song->tps_module->state.cached_cid, TOA_SONG_CMD_DATA, TOA_ERROR_MEM_WRITE);
//    TILE_ASSERT_IGNORE(false);
  }

  // Clean stuff and free CID
  song_init();
}


/**
 ****************************************************************************************
 * @brief Initialize the state of the SONG feature.
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, song_init, (void))
{
  // init everything
  memset(&song->tps_module->state, 0, sizeof(song->tps_module->state));
  last_song_cid = TOA_CONNECTIONLESS_CID;
}

/**
 ****************************************************************************************
 * @brief Send an error for the SONG feature and clears Song contxt if needed
 *
 * @param[in] cmd The command which originated the error
 * @param[in] err The error code
 *
 ****************************************************************************************
 */
static void song_send_error(uint8_t cid, uint8_t cmd, uint8_t err)
{
  uint8_t rsp[] = {TOA_SONG_RSP_ERROR, cmd, err};
  toa_send_response(cid, TOA_RSP_SONG, rsp, sizeof(rsp));

  if(cid == song->tps_module->state.cached_cid)
  {
    song_init();
  }
}


/**
 ****************************************************************************************
 * @brief Send a TOA_SONG_RSP_QUALITY.
 *
 * @param[in] strength The strength the song was played at
 * @param[in] quality  The quality factor of the song, in millivolts difference between note and REST.
 *
 ****************************************************************************************
 */
void song_quality(uint8_t quality)
{
  if(TOA_CONNECTIONLESS_CID != last_song_cid)
  {
    uint8_t rsp[] = {
      TOA_SONG_RSP_QUALITY,
      quality,
    };
    toa_send_response(last_song_cid, TOA_RSP_SONG, rsp, sizeof(rsp));
  }
}


/**
 ****************************************************************************************
 * @brief Register the Song Module
 *
 * @param[in] module       The Song Module.
 *
 ****************************************************************************************
 */
int tile_song_register(struct tile_song_module *module)
{
  song = module;

  toa_set_feature(TOA_FEATURE_SONG);
  toa_set_feature(TOA_FEATURE_TSQ);

  TOA_EXTERN_LINK(toa_process_Song_command);

  if(NULL != song->tps_module)
  {
    toa_set_feature(TOA_FEATURE_TPS);
    TOA_EXTERN_LINK(song_init);
  }

  return TILE_SUCCESS;
}
