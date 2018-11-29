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
 
/**
 * @file tile_player.h
 * @brief Interface specification for the Tile Audio Player addon
 */


#ifndef TILE_MUSIC_PLAYER_H
#define TILE_MUSIC_PLAYER_H

#include <stdint.h>
#include <stdbool.h>

enum TILE_MUSIC_PLAYER_COMMANDS
{
  TMP_START,                     /**< Only called ONCE at the start of audio generation.
                                   *  configure anything that only needs to be configured ONCE 
                                   */
  
  TMP_STOP,                      /**< Only called at the END of audio generation.
                                   *  shut down any hardware, assume device is going to deep sleep 
                                   */
  
  TMP_AUDIO_PIN_CONSTANT_LOW,     /**< Set the drive pin to output constant low 
                                   *   But do not disable the actual timers responsible for calling 
                                   *   back tmp_note_done().
                                   */

  TMP_AUDIO_PIN_CONSTANT_HIGH,    /**< Set the drive pin to output constant high
                                   *   But do not disable the actual timers responsible for calling 
                                   *   back tmp_note_done().
                                   */

  TMP_AUDIO_PIN_PWM,              /**< Ensure PWM is being routed to drive pin */
};


/*************************************************
 * Functions for tile_platform code to IMPLEMENT
 *************************************************/

/**
 * @brief For tile_platforms to implement.
 * Initialize the hardware.
 */
void tmp_hardware_init(void);


/**
 * @brief For tile_platforms to implement.
 * Get the pointer to the programmable song.
 * @return Pointer to a song. NULL if it doesn't exist.
 */
uint8_t * tmp_get_programmable_song(void);


/**
 * @brief For tile_platforms to implement.
 * This function is called when playback of a song has just started.
 * @param[in] the song_index
 */
void tmp_song_starting(uint8_t song_index);


/**
 * @brief For tile_platforms to implement.
 * Play the note passed as an argument.
 * @param[in] freq       The note frequency.
 * @param[in] duration   The note duration in units of 10 ms.
 * @param[in] last_note  True if this is the last note.
 */
void tmp_play_note(uint16_t note, uint16_t duration, bool last_note);


/**
 * @brief For tile_platforms to implement.
 * Set the audio amplifier amp level.
 * @param[in] level  the level to set the amp, valid levels are 0, 1, 2, 3.
 */
void tmp_amp_set_volume(uint8_t level);


/**
 * @brief For tile_platforms to implement.
 * Configure the audio amp for the modes defined in 
 * @param[in] cmd   se @ref TILE_AUDIO_PLAYER_AMP_MODES for the modes the hardware must implement.
 */
void tmp_hw_command(enum TILE_MUSIC_PLAYER_COMMANDS cmd);


/************************************************************
 * Functions for tile_platform code to CALL / user interface
 ************************************************************/

/**
 * @brief For tile_platforms to call.
 * Initialize Tile Audio Player state.
 */
void tmp_init(void);


/**
 * @brief For tile_platforms to call.
 * Tell Tile Music Player when the timer finished expiring.
 */
void tmp_note_done(void);


/**
 * @brief For tile_platforms to call.
 * Play a song
 * @param[in] number    the song index number
 * @param[in] strength  the volume of the song e.g. 1, 2, 3
 * @return @ref TILE_SUCESS on success, else TILE_ERROR_ILLEGAL_PARAM
 */
int tmp_play_song(uint8_t number, uint8_t strength);


/**
 * @brief For tile_platforms to call.
 * Stop the currently playing song and any other queued songs.
 * @return error code see @ref TILE_ERROR_CODES
 */
int tmp_stop_song(void);


/**
 * @brief For tile_platforms to call.
 * Query if a song is playing.
 * @return true if the song is playing.
 */
bool tmp_song_playing(void);


/**
 * @brief For tile_platforms to call.
 * Test the audio.
 */
void tmp_test_audio(void);

#endif
