
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

#include <stdint.h>
#include <stdbool.h>

#include "tile_player/tile_player.h"
#include "schedule/tile_schedule.h"
#include "modules/tile_song_module.h"
#include "assert/tile_assert.h"
#include "tile_lib.h"

/**
 * @file tile_player.c
 * @brief Tile player, HW agnostic portion
 */


/* Some defines for accessing the note array properly */
#define NOTE_ARRAY_BASE_NOTE (C3)
#define NOTE_ARRAY_MAX_NOTE  (B8)
#define NOTE_ARRAY_INDEX(n) ((n) - NOTE_ARRAY_BASE_NOTE)  
#define SILENCE 0
#define TEN_MS  1

/* Values for setting the PWM to the correct frequency for each note.
 * For our implementation, we only store the notes useful to us, which
 * is the range from C3 to B8 */
static const uint16_t notes[] = {
  [NOTE_ARRAY_INDEX(C3)]  = 131,   // in units of Hz for all the entries in this table
  [NOTE_ARRAY_INDEX(CS3)] = 138,
  [NOTE_ARRAY_INDEX(D3)]  = 147,
  [NOTE_ARRAY_INDEX(DS3)] = 156,
  [NOTE_ARRAY_INDEX(E3)]  = 165,
  [NOTE_ARRAY_INDEX(F3)]  = 175,
  [NOTE_ARRAY_INDEX(FS3)] = 185,
  [NOTE_ARRAY_INDEX(G3)]  = 196,
  [NOTE_ARRAY_INDEX(GS3)] = 208,
  [NOTE_ARRAY_INDEX(A3)]  = 220,
  [NOTE_ARRAY_INDEX(AS3)] = 233,
  [NOTE_ARRAY_INDEX(B3)]  = 247,
  [NOTE_ARRAY_INDEX(C4)]  = 262,
  [NOTE_ARRAY_INDEX(CS4)] = 277,
  [NOTE_ARRAY_INDEX(D4)]  = 294,
  [NOTE_ARRAY_INDEX(DS4)] = 311,
  [NOTE_ARRAY_INDEX(E4)]  = 330,
  [NOTE_ARRAY_INDEX(F4)]  = 349,
  [NOTE_ARRAY_INDEX(FS4)] = 370,
  [NOTE_ARRAY_INDEX(G4)]  = 392,
  [NOTE_ARRAY_INDEX(GS4)] = 415,
  [NOTE_ARRAY_INDEX(A4)]  = 440,
  [NOTE_ARRAY_INDEX(AS4)] = 466,
  [NOTE_ARRAY_INDEX(B4)]  = 494,
  [NOTE_ARRAY_INDEX(C5)]  = 523,
  [NOTE_ARRAY_INDEX(CS5)] = 554,
  [NOTE_ARRAY_INDEX(D5)]  = 587,
  [NOTE_ARRAY_INDEX(DS5)] = 622,
  [NOTE_ARRAY_INDEX(E5)]  = 659,
  [NOTE_ARRAY_INDEX(F5)]  = 698,
  [NOTE_ARRAY_INDEX(FS5)] = 740,
  [NOTE_ARRAY_INDEX(G5)]  = 784,
  [NOTE_ARRAY_INDEX(GS5)] = 831,
  [NOTE_ARRAY_INDEX(A5)]  = 880,
  [NOTE_ARRAY_INDEX(AS5)] = 932,
  [NOTE_ARRAY_INDEX(B5)]  = 988,
  [NOTE_ARRAY_INDEX(C6)]  = 1047,
  [NOTE_ARRAY_INDEX(CS6)] = 1109,
  [NOTE_ARRAY_INDEX(D6)]  = 1175,
  [NOTE_ARRAY_INDEX(DS6)] = 1245,
  [NOTE_ARRAY_INDEX(E6)]  = 1319,
  [NOTE_ARRAY_INDEX(F6)]  = 1397,
  [NOTE_ARRAY_INDEX(FS6)] = 1480,
  [NOTE_ARRAY_INDEX(G6)]  = 1568,
  [NOTE_ARRAY_INDEX(GS6)] = 1661,
  [NOTE_ARRAY_INDEX(A6)]  = 1760,
  [NOTE_ARRAY_INDEX(AS6)] = 1865,
  [NOTE_ARRAY_INDEX(B6)]  = 1976,
  [NOTE_ARRAY_INDEX(C7)]  = 2093,
  [NOTE_ARRAY_INDEX(CS7)] = 2217,
  [NOTE_ARRAY_INDEX(D7)]  = 2349,
  [NOTE_ARRAY_INDEX(DS7)] = 2489,
  [NOTE_ARRAY_INDEX(E7)]  = 2637,
  [NOTE_ARRAY_INDEX(F7)]  = 2794,
  [NOTE_ARRAY_INDEX(FS7)] = 2960,
  [NOTE_ARRAY_INDEX(G7)]  = 3136,
  [NOTE_ARRAY_INDEX(GS7)] = 3322,
  [NOTE_ARRAY_INDEX(A7)]  = 3520,
  [NOTE_ARRAY_INDEX(AS7)] = 3729,
  [NOTE_ARRAY_INDEX(B7)]  = 3951,
  [NOTE_ARRAY_INDEX(C8)]  = 4186,
  [NOTE_ARRAY_INDEX(CS8)] = 4435,
  [NOTE_ARRAY_INDEX(D8)]  = 4699,
  [NOTE_ARRAY_INDEX(DS8)] = 4978,
  [NOTE_ARRAY_INDEX(E8)]  = 5274,
  [NOTE_ARRAY_INDEX(F8)]  = 5588,
  [NOTE_ARRAY_INDEX(FS8)] = 5920,
  [NOTE_ARRAY_INDEX(G8)]  = 6272,
  [NOTE_ARRAY_INDEX(GS8)] = 6645,
  [NOTE_ARRAY_INDEX(A8)]  = 7040,
  [NOTE_ARRAY_INDEX(AS8)] = 7459,
  [NOTE_ARRAY_INDEX(B8)]  = 7902,
};

const uint8_t FixedSong0[] = {  C3, 1, REST, REST }; // Click Song

const uint8_t FixedSong1[] = { // Find song
  D5, 3,
  FS5, 3, D5, 3, FS5, 3, D5, 3, FS5, 3, D5, 3, FS5, 3, D5, 3, FS5, 3, D5, 3, FS5, 3,
  D6, 13, FS5, 13, G5, 13,
  A5, 13, D6, 9, REST, 4, A5, 2, D6, 2, FS6, 2,
  A5, 2, D6, 2, FS6, 2, D6, 2, FS6, 2, A6, 4, REST, 4, A5, 2, D6, 2, FS6, 2, A5, 2, D6, 2, FS6, 2, D6, 2,
  FS6, 2, A6, 4, REST, 4, A5, 2, D6, 2, FS6, 2, A5, 2, D6, 2, FS6, 2, D6, 2, FS6, 2, A6, 4, REST, 30, G5, 3,
  B5, 3, G5, 3, B5, 3, G5, 3, B5, 3, G5, 3, B5, 3, G5, 3, B5, 3, G5, 3, B5, 3,
  G6, 13, B5, 13, C6, 13,
  D6, 13, G6, 9, REST, 4, D6, 2, G6, 2, B6, 2,
  D6, 2, G6, 2, B6, 2, G6, 2, B6, 2, D7, 4, REST, 4, D6, 2, G6, 2, B6, 2, D6, 2, G6, 2, B6, 2, G6, 2,
  B6, 2, D7, 4, REST, 4, D6, 2, G6, 2, B6, 2, D6, 2, G6, 2, B6, 2, G6, 2, B6, 2, D7, 4, REST, 30, A5, 3,
  CS6, 3, A5, 3, CS6, 3, A5, 3, CS6, 3, A5, 3, CS6, 3, A5, 3, CS6, 3, A5, 3, CS6, 3,
  A6, 13, CS6, 13, D6, 13,
  E6, 13, A6, 9, REST, 4, E6, 2, A6, 2, CS7, 2,
  E6, 2, A6, 2, CS7, 2, A6, 2, CS7, 2, E7, 4, REST, 4, E6, 2, A6, 2, CS7, 2, E6, 2, A6, 2, CS7, 2, A6, 2,
  CS7, 2, E7, 4, REST, 4, E6, 2, A6, 2, CS7, 2, E6, 2, A6, 2, CS7, 2, A6, 2, CS7, 2, E7, 4, REST, 30, D6, 3,
  FS6, 3, D6, 3, FS6, 3, D6, 3, FS6, 3, D6, 3, FS6, 3, D6, 3, FS6, 3, D6, 3, FS6, 3,
  D7, 13, FS6, 13, G6, 13,
  A6, 13, D7, 9, REST, 4, A6, 2, D7, 2, FS7, 2,
  A6, 2, D7, 2, FS7, 2, D7, 2, FS7, 2, A7, 4, REST, 4, A6, 2, D7, 2, FS7, 2, A6, 2, D7, 2, FS7, 2, D7, 2,
  FS7, 2, A7, 4, REST, 4, A6, 2, D7, 2, FS7, 2, A6, 2, D7, 2, FS7, 2, D7, 2, FS7, 2, A7, 4, REST, REST
};

const uint8_t FixedSong2[] = { // Active Song
  A5, 5, REST, 7, A6, 2, REST, 11, A5, 2, REST, 23, A5, 2, REST, 11, A6, 2, REST, 11, A5, 2, REST, 23, D6, 13, FS5, 13, G5, 13, A5, 13, D5, 26, D6, 14, REST, REST
};

const uint8_t FixedSong3[] = { // Sleep Song
  A6, 38, D6, 13, G6, 13, FS6, 13, D6, 13, A5, 10, REST, 3, D5, 5, REST, 7, D6, 2, REST, 11, D5, 2, REST, 23, D5, 2, REST, 11, D6, 2, REST, 11, D3, 2, REST, 1, REST, REST
};

const uint8_t FixedSong4[] = { // Wake Song 
D5,38,
A5,13, 
FS5,13, 
G5,13, 
A5,13, 
D6,10, 

REST,3, 	A5,5, 
REST,7,		A6,2, 
REST,11,	A5,2, 

REST,23,	A5,2, 
REST,11, 	A6,2, 
REST,11,	A5,2, 
REST, REST };

const uint8_t FixedSong5[]  = { FS7,250, FS7,250, FS7,250, FS7,250, REST, REST };     /* Factory Song: For factory test song - 10 seconds of F#7 at 2960Hz */
const uint8_t FixedSong6[]  = { C8,1, CS8,1, D8,1, DS8,1, E8,1, F8,1, REST, REST };
const uint8_t FixedSong7[]  = { REST, REST };                                         /* Silent Song */
const uint8_t FixedSong8[]  = { REST, REST };                                         /* Button Song: currently silent */
const uint8_t FixedSong9[]  = { A5, 2, REST,11, A6, 2, REST, 11, A5, 2, REST,REST };  /* WakePart Song */
const uint8_t FixedSong10[] = { FS4, 3, REST,10, D5, 11, REST, 2, A5, 3, REST, 10, D6, 12, REST,REST };             /* double tap success Song */
const uint8_t FixedSong11[] = { GS4, 3, REST,10, GS5, 11, REST, 2, D4, 3, REST, 10, GS3, 12, REST,1, REST,REST };   /* double tap failure Song */
const uint8_t FixedSong12[] = { /*C3, 1, REST, 10, C3, 1,*/ REST, REST };                                           /* 2 clicks song */
const uint8_t FixedSong13[] = { D5, 30, REST, REST };                                                               /* 1 bip Song */
const uint8_t FixedSong14[] = { D5, 30, REST,11,	D5, 30, REST, REST };                                             /* 2 bip Song */
const uint8_t FixedSong15[] = { D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST, REST };                           /* 3 bip Song */
const uint8_t FixedSong16[] = { D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST, REST };         /* 4 bip Song */
const uint8_t FixedSong17[] = { D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST, REST };                                       /* 5 bip Song */
const uint8_t FixedSong18[] = { D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST, REST };                     /* 6 bip Song */
const uint8_t FixedSong19[] = { D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST,11,	D5, 30, REST, REST };   /* 7 bip Song */
const uint8_t FixedSong20[] = { A4, 6,  REST,6,	  A5, 6,  REST,6,	  A4, 6,  REST, REST };   /* double tap HeartBeat Song */
  
const uint8_t *tile_song_array[] = { 
FixedSong0,
FixedSong1, 
FixedSong2, 
FixedSong3,
FixedSong4,
FixedSong5,
FixedSong6,
FixedSong7, 
FixedSong8, 
FixedSong9,
FixedSong10,
FixedSong11,
FixedSong12,
FixedSong13,
FixedSong14,
FixedSong15,
FixedSong16,
FixedSong17,
FixedSong18,
FixedSong19,
FixedSong20,
};

static const uint8_t *p_current_song = NULL;
static const uint8_t *p_next_song = NULL;
static uint8_t current_song_strength = 0;
static uint8_t next_song_strength = 0;
static uint8_t startup_sequence = 0;

/********************************************************************
 * LOCAL FUNCTIONS
 ********************************************************************/

/** 
 * @brief Ramp up amplifier to avoid really high current peaks
 */
static void StartupPlayer()
{
  static uint8_t cur_amp_level;

  switch(startup_sequence) 
  {
    case 1:
      cur_amp_level = 0;
      break;
    case 2:
      tmp_hw_command(TMP_AUDIO_PIN_CONSTANT_HIGH);
      break;
    case 3:
    case 4:
    case 5:
      if(current_song_strength > cur_amp_level) 
      {
        tmp_amp_set_volume(++cur_amp_level);
      }
      break;
  }
  if(startup_sequence >= 5)
  {
    startup_sequence = 0;
  }
  else
  {
    startup_sequence++;
  }
  
  // Don't do anything for 10 ms by playing a silent note
  tmp_play_note(SILENCE, TEN_MS, false);
}

/** 
 * @brief Function called at the end of a song.
 * Will start enqueued song if any, otherwise shut everything Off and notify application.
 */
void SongDone(void * arg)
{
  //unused
  (void)arg;
  
  if(NULL != p_next_song)
  {
    /* Start enqueued Song */
    p_current_song        = p_next_song;
    current_song_strength = next_song_strength;
    p_next_song           = NULL;
    startup_sequence      = 1;

    tmp_hw_command(TMP_AUDIO_PIN_CONSTANT_HIGH);
    
    // Don't do anything for 200 ms by playing a silent note
    tmp_play_note(SILENCE, 20 * TEN_MS, false);
  }
  else
  {
    /* No enqueued Song */
    /* Shut everything Off and notify application */
    /* Configure GPIO */
    tmp_hw_command(TMP_STOP);
    p_current_song = NULL;
  }
}


/**
 * @brief Start the next note.
 * This function is called from an interrupt context.
 */
static void NextNote()
{
  uint8_t note      = *p_current_song++;
  uint8_t duration  = *p_current_song++;

  if(REST == note && REST == duration)
  {
    /* End of song reached */
    tile_schedule(SongDone, NULL);
    return;
  }

  uint8_t next_note     = p_current_song[0];
  uint8_t next_duration = p_current_song[1];

  bool last_note = ((next_note == REST) && (next_duration == REST));
  
  if(REST == note)
  {
    /* Set buzzer pin high so audio amp doesn't shut off. Keeps peaks low. */
    tmp_hw_command(TMP_AUDIO_PIN_CONSTANT_HIGH);
    
    tmp_play_note(SILENCE, duration, last_note);
  }
  else
  {
    tmp_hw_command(TMP_AUDIO_PIN_PWM);
    
    tmp_play_note(notes[NOTE_ARRAY_INDEX(note)], duration, last_note);
  }
}


/**
 * @brief Initialize the Tile player
 */
void tmp_init(void)
{
  TILE_ASSERT(NULL == p_current_song);
  TILE_ASSERT(NULL == p_next_song);
  TILE_ASSERT(0 == current_song_strength);
  TILE_ASSERT(0 == next_song_strength);
  TILE_ASSERT(0 == startup_sequence);
  tmp_hardware_init();
}


/**
 * @brief Play a song. Queue song if necessary
 */
void tmp_play_song_handler(void * arg)
{
  uint8_t number = ((uint32_t) arg & 0xff);
  uint8_t strength = (((uint32_t) arg & 0xff00) >> 8);
  
  if(NULL != p_current_song && NULL == p_next_song)
  {
    /* A song is currently playing but there is NO enqueued song */
    /* SO enqueue the song */
    if(TILE_SONG_FIND == number && tmp_get_programmable_song())
    {
      p_next_song = tmp_get_programmable_song();
    }
    else
    {
      p_next_song = tile_song_array[number];
    }
    next_song_strength = strength;
  }
  else if(NULL == p_current_song)
  {
    /* NO song is currently playing so play the song now*/
    current_song_strength = strength;
    startup_sequence = 1;

    /* Tell the driver what song is about to start */
    tmp_song_starting(number);
    
    /* Should be use the programmable find song or not? */
    /* tmp_get_programmable_song() returns NULL if we don't want to use the programmable song */
    if(TILE_SONG_FIND == number && tmp_get_programmable_song())
    {
      p_current_song = tmp_get_programmable_song();
    }
    else 
    {
      p_current_song = tile_song_array[number];
    }
    
    tmp_hw_command(TMP_START);
    tmp_play_note(SILENCE, TEN_MS, false);
  }
  else 
  {
    /* A song is playing AND the queue is full, ignore */
  }
  return;
}

static void tmp_stop_song_handler(void * arg)
{
  /* Destroy the queue */
  p_next_song = NULL;

  /* Turn off the songs */
  if(p_current_song != NULL)
  {
    SongDone(NULL);
  }
}

/********************************************************************
 * Interface to tile_player (Global functions)
 ********************************************************************/

/**
 * @brief Stop currently playing song and remove enqueued songs
 */
int tmp_stop_song(void)
{
  tile_schedule(tmp_stop_song_handler, NULL);
  return TILE_SUCCESS;
}

/**
 * @brief Called from tile_platform implementation's interrupt context
 */
void tmp_note_done(void)
{
  /* don't do anything if the p_current_song is null,
   * we're at the end of a song and waiting for SongDone
   * to finish */
  if (p_current_song == NULL)
  {
    return;
  }
  
  if(startup_sequence)
  {
    StartupPlayer();
  }
  else if (tmp_song_playing())
  {
    NextNote();
  }
}

bool tmp_song_playing(void)
{
  return p_current_song != NULL;
}

int tmp_play_song(uint8_t number, uint8_t strength)
{
  if (number >= TILE_SONG_MAX) 
  {
    return TILE_ERROR_ILLEGAL_PARAM;
  }
  
  /* TODO: Test on 16 bit platform e.g. CSR */
  uint32_t arg = (strength << 8) | number;

  tile_schedule(tmp_play_song_handler, (void *) arg);
  
  return TILE_SUCCESS;
}

void tmp_test_audio(void)
{
  tmp_play_song(2,3);
}
