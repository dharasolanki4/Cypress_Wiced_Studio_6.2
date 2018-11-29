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

 /** @file tka.c
 ** @brief Tile Keep Alive Feature
 */

#include "tka.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_toa_module.h"
#include "toa.h"

extern struct tile_timer_driver *timer;
extern struct tile_gap_driver *gap;
extern struct tile_toa_module *toa;


static void tka_process_cmd_config(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void tka_process_cmd_ack(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void tka_send_rsp_config(uint8_t cid);
static void tka_send_rsp_error(uint8_t cid, uint8_t rsp, uint8_t param);
static void tka_send_rsp_readConfig(uint8_t cid);
static void tka_send_rsp_check(uint8_t cid);


/**
 ****************************************************************************************
 * @brief process incomming TKA command packets
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
void tka_process_command(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  switch(data[0])
  {
    case TKA_CMD_CONFIG:
      tka_process_cmd_config(cid, &data[1], (datalen-1));
    break;
    case TKA_CMD_READ_CONFIG:
      tka_send_rsp_readConfig(cid);
    break;
    case TKA_CMD_ACK:
      tka_process_cmd_ack(cid, &data[1], (datalen-1));
    break;
    default:
      tka_send_rsp_error(cid, TKA_RSP_ERROR_UNSUPPORTED, data[0]);
    break;
  }
}

/**
 ****************************************************************************************
 * @brief process an @ref TKA_CMD_CONFIG Packet
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void tka_process_cmd_config(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  if(datalen >= 9 && data[0] <= 1)
  {
    toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];
    if(1 == data[0])
    {
      /* We are enabling TKA */
      channel->state       |= TOA_CHANNEL_TKA_ENABLED;
      channel->check_delay  = RDLE32(&data[1]);
      channel->ack_delay    = RDLE32(&data[5]);

      timer->start(TILE_TKA_TIMER1 + TOA_CHANNEL_IDX(cid), channel->check_delay * TILE_TICKS_PER_SEC);
    }
    else
    {
      channel->state &= ~TOA_CHANNEL_TKA_ENABLED;
      timer->cancel(TILE_TKA_TIMER1 + TOA_CHANNEL_IDX(cid));
    }

    tka_send_rsp_config(cid);
  }
  else
  {
    tka_send_rsp_error(cid, TKA_RSP_ERROR_PARAMS, TKA_CMD_CONFIG);
  }
}

/**
 ****************************************************************************************
 * @brief process an @ref TKA_CMD_ACK Packet
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void tka_process_cmd_ack(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];
  if(TOA_CHANNEL_TKA_ENABLED & channel->state)
  {
    /* Clear ack bit and restart timer */
    channel->state &= ~TOA_CHANNEL_WAIT4ACK;
    timer->start(TILE_TKA_TIMER1 + TOA_CHANNEL_IDX(cid), channel->check_delay * TILE_TICKS_PER_SEC);
  }
}

/**
 ****************************************************************************************
 * @brief send an @ref TKA_RSP_CONFIG Response
 *
 ****************************************************************************************
 */
static void tka_send_rsp_config(uint8_t cid)
{
  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

  uint8_t activate = (TOA_CHANNEL_TKA_ENABLED & channel->state) ? 1 : 0;
  uint8_t transaction[] = {
      TKA_RSP_CONFIG,
      activate,
      MKLE16(0),
      MKLE32(channel->check_delay),
      MKLE32(channel->ack_delay)
  };

  toa_send_response(cid, TOA_RSP_TKA, transaction, sizeof(transaction));
}

/**
 ****************************************************************************************
 * @brief send an @ref TKA_RSP_READ_CONFIG Response
 *
 ****************************************************************************************
 */
static void tka_send_rsp_readConfig(uint8_t cid)
{
  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

  uint8_t activate = (TOA_CHANNEL_TKA_ENABLED & channel->state) ? 1 : 0;
  uint8_t transaction[] = {
      TKA_RSP_READ_CONFIG,
      activate,
      MKLE16(0),
      MKLE32(channel->check_delay),
      MKLE32(channel->ack_delay)
  };

  toa_send_response(cid, TOA_RSP_TKA, transaction, sizeof(transaction));
}

/**
 ****************************************************************************************
 * @brief Send a TKA ERROR Response
 *
 * @param[in] rsp   The TKA Error Code.
 * @param[in] param The param to send with the Error.
 *
 ****************************************************************************************
 */
static void tka_send_rsp_error(uint8_t cid, uint8_t rsp, uint8_t param)
{
  uint8_t transaction[] = { rsp, param };

  toa_send_response(cid, TOA_RSP_TKA, transaction, sizeof(transaction));
}

/**
 ****************************************************************************************
 * @brief send an @ref TKA_RSP_CHECK Response
 *
 ****************************************************************************************
 */
static void tka_send_rsp_check(uint8_t cid)
{
  uint8_t transaction[] = { TKA_RSP_CHECK };

  toa_send_response(cid, TOA_RSP_TKA, transaction, sizeof(transaction));
}

/**
 ****************************************************************************************
 * @brief Init TKA environment.
 * This should be done at connection since config is not kept accross connections
 *
 ****************************************************************************************
 */
void tka_init(void)
{
  for(int i = 0; i < 8; i++)
  {
    timer->cancel(TILE_TKA_TIMER1 + i);
  }
}

/**
 ****************************************************************************************
 * @brief Check if AckDelay or CheckDelay have expired
 *
 * @param[in] current_time     Time in ticks
 *
 ****************************************************************************************
 */
void tka_check_time(uint8_t index)
{
  uint8_t cid = index + TOA_BROADCAST_CID + 1;

  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

  if(!(TOA_CHANNEL_TKA_ENABLED & channel->state))
  {
    /* Something strange happened. Ignore. */
    return;
  }

  if(TOA_CHANNEL_WAIT4ACK & channel->state)
  {
    /* AckDelay expired. No ack received. Clear the channel. */
    if(NULL != toa->tka_closed_channel_count)
    {
      (*toa->tka_closed_channel_count)++;
    }
    toa_channel_close(cid, CLOSE_REASON_TKA_MISSING, NULL, 0);
  }
  else
  {
    /* CheckDelay expired */
    channel->state |= TOA_CHANNEL_WAIT4ACK;
    timer->start(TILE_TKA_TIMER1 + index, channel->ack_delay * TILE_TICKS_PER_SEC);
    tka_send_rsp_check(cid);
  }
}


void tka_start_default(uint8_t cid)
{
  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

  channel->check_delay  = TKA_DEFAULT_CHECK_DELAY;
  channel->ack_delay    = TKA_DEFAULT_ACK_DELAY;
  channel->state       |= TOA_CHANNEL_TKA_ENABLED;

  timer->start(TILE_TKA_TIMER1 + TOA_CHANNEL_IDX(cid), TKA_DEFAULT_START_DELAY * TILE_TICKS_PER_SEC);
}
