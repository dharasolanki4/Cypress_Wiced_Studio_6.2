/* NOTICE
 *
 * © 2016 Tile Inc.  All Rights Reserved.

 * All code or other information included in the accompanying files ("Tile Source Material")
 * is CONFIDENTIAL and PROPRIETARY information of Tile Inc. and your access and use is subject
 * to the terms of Tile's non-disclosure agreement as well as any other applicable agreement
 * with Tile.  The Tile Source Material may not be shared or disclosed outside your company,
 * nor distributed in or with any devices.  You may not use, copy or modify Tile Source
 * Material or any derivatives except for the purposes expressly agreed and approved by Tile.
 * All Tile Source Material is provided AS-IS without warranty of any kind.  Tile does not
 * warrant that the Tile Source Material will be error-free or fit for your purposes.
 * Tile will not be liable for any damages resulting from your use of or inability to use
 * the Tile Source Material.
 * You must include this Notice in any copies you make of the Tile Source Material.
 */

 /** @file toa.c
 ** @brief Tile Overtheair Api: defines Tile communication protocol over the air
 */



#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tile_lib.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_random_driver.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_toa_module.h"
#include "modules/tile_adv_int_module.h"
#include "modules/tile_tofu_module.h"
#include "modules/tile_tdt_module.h"
#include "toa.h"
#include "tmd.h"
#include "tka.h"
#include "tdt.h"
#include "tcu.h"
#include "tdg.h"
#include "tdi.h"
#include "queue.h"
#include "song.h"
#include "../tileHash.h"
#include "test.h"
#include "tiletime.h"
#include "tmf.h"
#include "tofu.h"
#include "tpc.h"
#include "trm.h"


bool            toaRspPending;
uint8_t         toaRspDropped;
bool            toaTransportReady;
queue_t         *toaQueue;

struct tile_toa_module *toa;
struct tile_adv_int_module *adv;
uint8_t         toa_features[TOA_FEATURE_BYTES];
uint8_t         toa_client_features[TOA_CLIENT_FEATURE_BYTES];
uint8_t         randT_fixed[TILE_RAND_T_FIXED_LEN];

static struct
{
  uint8_t  key[TILE_SESSION_KEY_LEN];
  uint32_t nonce;
} broadcast;

extern struct tile_gap_driver *gap;
extern struct tile_gatt_server_driver *gatt_server;
extern struct tile_random_driver *random;
extern struct tile_timer_driver *timer;

extern struct tile_tofu_module *tofu;

static void toa_process_connectionless_command(const uint8_t *data, uint8_t datalen);
static void toa_process_channel_command(const uint8_t cid, const uint8_t *data, uint8_t datalen);
static void toa_send_response_ready(uint8_t cid);
static void toa_send_packet(uint8_t* data, uint8_t datalen);
static void toa_unassign_channel(uint8_t cid);
static void toa_process_open_channel_command(const uint8_t *token, const uint8_t *data, uint8_t datalen);
static void toa_process_authenticate_command(const uint8_t *token, const uint8_t *data, uint8_t datalen);
static void toa_process_associate_command(const uint8_t *token, const uint8_t *data, uint8_t datalen);
static void toa_process_ready_command(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void toa_process_advInt_command(const uint8_t cid, const uint8_t* data, uint8_t datalen);

extern struct tile_tdt_module *tdt;

/**
 ****************************************************************************************
 * @brief Register the TOA module.
 *
 * @param[in] module     Module structure.
 *
 ****************************************************************************************
 */
int tile_toa_register(struct tile_toa_module *module)
{
  if((NULL == module) || (NULL == module->queue))
  {
    return TILE_ERROR_ILLEGAL_PARAM;
  }
  
  toa = module;

  toa_set_feature(TOA_FEATURE_TMA);
  toa_set_feature(TOA_FEATURE_TKA);
#if TILE_SUPPORT_TEST
  toa_set_feature(TOA_FEATURE_TEST);
#endif

  return TILE_SUCCESS;
}


int tile_toa_authorized(uint8_t authorization_type, uint16_t authorization_time)
{
  uint8_t rsp[3];
  uint32_t token = TOA_BROADCAST_TOKEN;

  rsp[0] = authorization_type;
  memcpy(&rsp[1], &authorization_time, 2);

  toa_send_connectionless_response((uint8_t*)&token, TOA_RSP_AUTHORIZED, rsp, 3);

  return TILE_SUCCESS;
}


/**
 ****************************************************************************************
 * @brief Sets the value of a particular bit number in a byte array
 *
 * @param[in] bytes   Byte array for bitfield.
 * @param[in] bit     Bit number to set.
 * @param[in] enable  0 to disable bit and 1 to enable.
 *
 ****************************************************************************************
 */
static void set_bit(uint8_t *bytes, uint16_t bit)
{
  /* bit/8 is byte containing bit n. Set bit (bit mod 8) of this byte to value enable (0 or 1) */
  bytes[bit/8] |= 1 << (bit & 0x7);
}


/**
 ****************************************************************************************
 * @brief Gets a particular bit number in a byte array
 *
 * @param[in] bytes   Byte array for bitfield.
 * @param[in] bit     Bit number to get.
 *
 * @return            1 if bit is set and 0 if bit is not set.
 *
 ****************************************************************************************
 */
static uint8_t get_bit(uint8_t *bytes, uint16_t bit)
{
  return (bytes[bit/8] >> (bit & 0x7)) & 0x1;
}


/**
 ****************************************************************************************
 * @brief Clears a particular bit number in a byte array
 *
 * @param[in] bytes   Byte array for bitfield.
 * @param[in] bit     Bit number to clear.
 *
 ****************************************************************************************
 */
static void clear_bit(uint8_t *bytes, uint16_t bit)
{
  bytes[bit/8] &= ~(1 << (bit & 0x7));
}

/**
 ****************************************************************************************
 * @brief Sets a supported feature bit.
 *
 * @param[in] bit     Bit number to set.
 * @param[in] enable  0 to disable bit and 1 to enable.
 *
 ****************************************************************************************
 */
void toa_set_feature(uint16_t bit)
{
  set_bit(toa_features, bit);
}


/**
 ****************************************************************************************
 * @brief Gets a supported feature bit.
 *
 * @param[in] bit     Bit number to get.
 *
 * @return            1 if bit is set and 0 if bit is not set.
 *
 ****************************************************************************************
 */
uint8_t toa_get_feature(uint16_t bit)
{
  return get_bit(toa_features, bit);
}


/**
 ****************************************************************************************
 * @brief Clears a supported feature bit.
 *
 * @param[in] bit     Bit number to clear.
 *
 ****************************************************************************************
 */
void toa_clear_feature(uint16_t bit)
{
  clear_bit(toa_features, bit);
}


/**
 ****************************************************************************************
 * @brief A TOA response was successfully sent to the TOA Client (and an other one can be sent).
 *
 ****************************************************************************************
 */
void tile_toa_response_sent_ok(void)
{
  if(true == toaRspPending)
  {
    toaRspPending = false;

    toa_send_attempt();
  }
  else
  {
    // should not happen
  }
}

/**
 ****************************************************************************************
 * @brief The underlying TOA transport is ready.
 *  This is the case when TOA_RSP channel was enabled for notifications or indications.
 *
 * @param[in] ready       1 for ready, 0 for not ready.
 *
 ****************************************************************************************
 */
void tile_toa_transport_ready(bool ready)
{
  /* Only react when transport is turned from OFF to ON */
  if((false == toaTransportReady) && (true == ready))
  {
    /* TOA response channel open */
    toaTransportReady = true;

    /*
     * If a transport ready and a TOA command are received at the same
     * time, the command could be processed first and the response will be
     * placed on the queue. If this is the case, we should send the response
     * now.
     */
    toa_send_attempt();
  }
  else if(false == ready)
  {
    // for security reasons, we disconnect if TOA_RSP characteristic notification is otherwise modified (or closed)
    gap->gap_disconnect();
  }
  else
  {
    // ignore unchanged setting
  }
}

/**
 ****************************************************************************************
 * @brief Process Incomming TOA Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
void tile_toa_command_received(const uint8_t* data, uint8_t datalen)
{
  if(datalen > 0)
  {
    uint8_t cid = data[0];
    if(TOA_CONNECTIONLESS_CID == cid)
    {
      toa_process_connectionless_command(&data[1], datalen - 1);
    }
    else if(cid > TOA_BROADCAST_CID && cid <= TOA_BROADCAST_CID + toa->num_channels)
    {
      toa_process_channel_command(cid, &data[1], datalen - 1);
    }
    else
    {
      /* Ignore bad CID */
    }
  }
  else
  {
    /* Ignore zero length data */
  }
}


/**
 ****************************************************************************************
 * @brief Process Incomming TOA Commands on the connectionless CID
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_connectionless_command(const uint8_t *data, uint8_t datalen)
{
  if(datalen < TOA_TOKEN_LEN + 1)
  {
    /* Invalid packet. Ignore. */
    return;
  }

  /* Grab the token and command */
  const uint8_t *token  = &data[0];
  const uint8_t command = data[TOA_TOKEN_LEN];

  data    += TOA_TOKEN_LEN + 1;
  datalen -= TOA_TOKEN_LEN + 1;

  switch(command)
  {
    case TOA_CMD_OPEN_CHANNEL:
      toa_process_open_channel_command(token, data, datalen);
      break;

    case TOA_CMD_TDI:
      tdi_process_command(token, data, datalen);
      break;

    case TOA_CMD_AUTHENTICATE:
      toa_process_authenticate_command(token, data, datalen);
      break;

    case TOA_CMD_TMF:
      tmf_process_command(token, data, datalen);
      break;

    case TOA_CMD_ASSOCIATE:
      toa_process_associate_command(token, data, datalen);
      break;

    default:
      toa_send_connectionless_response_error((uint8_t*)token, TOA_RSP_ERROR_UNSUPPORTED, command);
      break;
  }
}


/*
 * Macro to simplify the switch in toa_process_channel_command. This checks if a feature
 * is enabled, and if so will use func to process the message. Otherwise, send an error
 * back to the client.
 */
#define TOA_PROCESS_IF(feature, func) do { \
  if(toa_get_feature(feature))             \
  {                                        \
    (func)(cid, pdata, datalen);           \
  }                                        \
  else                                     \
  {                                        \
    toa_send_response_error(cid,           \
      TOA_RSP_ERROR_UNSUPPORTED, command); \
  }                                        \
} while(0)


/**
 ****************************************************************************************
 * @brief Process Incomming TOA Commands on an open channel
 *
 * @param[in] cid        channel ID.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_channel_command(const uint8_t cid, const uint8_t *data, uint8_t datalen)
{
  toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

  if(!(TOA_CHANNEL_ASSIGNED & channel->state))
  {
    /* Ignore messages on unassigned channels */
    return;
  }

  if(0 == datalen)
  {
    /* There should not be any zero length messages. Ignore. */
    return;
  }

  uint8_t command             = data[0];
  const uint8_t* pdata        = &data[0];
  uint8_t res                 = 0;
  uint8_t micA[TILE_MIC_LEN]  = {0};

  if(datalen < (TILE_MIC_LEN +1))
  {
    res = 1;
    if(NULL != toa->mic_failure_count)
    {
      (*toa->mic_failure_count)++;
    }
  }
  else
  {
    channel->nonceA++;
    tile_mic_hash(channel->session_key, channel->nonceA, 1, data, datalen-TILE_MIC_LEN, micA);
    if(0 != memcmp(micA, &data[datalen-TILE_MIC_LEN], TILE_MIC_LEN))
    {
      res = 1;
      if(NULL != toa->mic_failure_count)
      {
        (*toa->mic_failure_count)++;
      }
    }

    pdata+=1;                   // skip the command
    datalen-=(1+TILE_MIC_LEN);  // remove the command and the MIC
  }
  if(0 != res)
  {
    // Update auth_failure_count when the MIC check fails for the first time.
    if (!(TOA_CHANNEL_AUTHENTICATED & channel->state) && NULL != toa->auth_failure_count) {
        (*toa->auth_failure_count)++;
    }
    /* Close channel on authentication failure */
    uint8_t rsp[sizeof(channel->nonceT) + TILE_MIC_LEN];
    memcpy(rsp, &channel->nonceT, sizeof(channel->nonceT));
    memcpy(rsp+sizeof(channel->nonceT), micA, TILE_MIC_LEN);
    toa_channel_close(cid, CLOSE_REASON_MIC_FAILURE, rsp, sizeof(rsp));
    return;
  }

  /* Channel is authenticated if a valid MIC has been received */
  channel->state |= TOA_CHANNEL_AUTHENTICATED;
  timer->cancel(TILE_AUTHENTICATION_TIMER);

  /* Only forward to subfeatures if there is data to forward. Otherwise, return error */
  if(datalen > 0)
  {
    switch(command)
    {
      case TOA_CMD_TOFU_CTL:
        TOA_PROCESS_IF(TOA_FEATURE_TOFU, tofu_process_control_command);
        break;
      case TOA_CMD_TOFU_DATA:
        TOA_PROCESS_IF(TOA_FEATURE_TOFU, tofu_process_data);
        break;
      case TOA_CMD_TEST:
        TOA_PROCESS_IF(TOA_FEATURE_TEST, test_process_command);
        break;
      case TOA_CMD_TDT:
        TOA_PROCESS_IF(TOA_FEATURE_TDT, tdt_process_command);
        break;
      case TOA_CMD_SONG:
        TOA_PROCESS_IF(TOA_FEATURE_SONG, toa_process_Song_command);
        break;
      case TOA_CMD_ADV_INT:
        TOA_PROCESS_IF(TOA_FEATURE_ADV_INT, toa_process_advInt_command);
        break;
      case TOA_CMD_TKA:
        TOA_PROCESS_IF(TOA_FEATURE_TKA, tka_process_command);
        break;
      case TOA_CMD_TDG:
        TOA_PROCESS_IF(TOA_FEATURE_TDG, tdg_process_command);
        break;
      case TOA_CMD_TMD:
        TOA_PROCESS_IF(TOA_FEATURE_TMD, tmd_process_command);
        break;
      case TOA_CMD_TCU:
        TOA_PROCESS_IF(TOA_FEATURE_TCU, tcu_process_command);
        break;
      case TOA_CMD_TIME:
        TOA_PROCESS_IF(TOA_FEATURE_TIME, time_process_command);
        break;
      case TOA_CMD_READY:
        toa_process_ready_command(cid, pdata, datalen);
        break;
      case TOA_CMD_CLOSE_CHANNEL:
        toa_unassign_channel(cid);
        break;
      case TOA_CMD_TPC:
        TOA_PROCESS_IF(TOA_FEATURE_TPC, tpc_process_command);
        break;
      case TOA_CMD_TRM:
        TOA_PROCESS_IF(TOA_FEATURE_TRM, trm_process_command);
        break;
      default:
        toa_send_response_error(cid, TOA_RSP_ERROR_UNSUPPORTED, command);
        break;
    }
  }
  else
  {
    toa_send_response_error(cid, TOA_RSP_ERROR_PARAMETERS, command);
  }
}

#undef TOA_PROCESS_IF


/**
 ****************************************************************************************
 * @brief Send a response on the connectionless channel
 * The Response is queued and then we try send it.
 *
 * @param[in] token      token for this transaction.
 * @param[in] response   TOA response code.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
void toa_send_connectionless_response(uint8_t *token, uint8_t response, uint8_t *data, uint8_t datalen)
{
  uint8_t transaction[TOA_MPS+TOA_TOKEN_LEN+2];

  datalen = datalen < TOA_MPS ? datalen : TOA_MPS;

  transaction[0] = TOA_CONNECTIONLESS_CID;
  memcpy(&transaction[1], token, TOA_TOKEN_LEN);
  transaction[1 + TOA_TOKEN_LEN] = response;
  if(datalen > 0 && NULL != data)
  {
    memcpy(&transaction[TOA_TOKEN_LEN+2], data, datalen);
  }

  // enqueue
  if(0 != putItem(toaQueue, &transaction[0], datalen+TOA_TOKEN_LEN+2))
  {
    // packet drop
    toaRspDropped = response;
  }

  toa_send_attempt();
}

/**
 ****************************************************************************************
 * @brief Send a TOA Response<br>
 * The Response is queued and then we try send it.
 *
 * @param[in] cid        channel ID.
 * @param[in] response   TOA response code.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
void toa_send_response(uint8_t cid, uint8_t response, uint8_t* data, uint8_t datalen)
{
  uint8_t transaction[TOA_MPS+TILE_MIC_LEN+2];

  datalen = datalen < TOA_MPS ? datalen : TOA_MPS;

  transaction[0] = cid;
  transaction[1] = response;

  if((0 != datalen) && (NULL != data))
  {
    memcpy(&transaction[2], data, datalen);
  }

  // enqueue
  if (0 != putItem(toaQueue, &transaction[0], datalen + 2))
  {
    // packet drop
    toaRspDropped = response;
  }

  toa_send_attempt();
}


/**
 ****************************************************************************************
 * @brief Send a TOA broadcast response.
 *
 * @param[in] response   TOA response code.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
void toa_send_broadcast(uint8_t response, uint8_t *data, uint8_t datalen)
{
  toa_send_response(TOA_BROADCAST_CID, response, data, datalen);
}


/**
 ****************************************************************************************
 * @brief Attempt to send a TOA Packet<br>
 * This function takes care of the flow control
 *
 ****************************************************************************************
 */
void toa_send_attempt(void)
{
  uint8_t toaBuff[TOA_MPS + TILE_MIC_LEN + 2];
  uint8_t datalen = 0;
  if((true == toaTransportReady) && (false == toaRspPending))
  {
    if(0 != toaRspDropped)
    {
      /*
       * If some packet was dropped, broadcast the error to everybody. The MIC will
       * prevent any further activity on the channels which had packets dropped.
       * In case the dropped packet was on the Broadcast CID, the recommendation is
       * that Apps close and reopen their channel to get the up to date nonce_B.
       */
      uint8_t rsp[4+TILE_MIC_LEN] = {
        TOA_BROADCAST_CID,
        TOA_RSP_ERROR,
        TOA_RSP_ERROR_DROPPED_RSP,
        toaRspDropped
      };
      toa_send_packet(rsp, 4);
      toaRspDropped = 0;
      toaRspPending = true;
    }
    else if (0 == getItem(toaQueue, toaBuff, &datalen))
    {
      toa_send_packet(toaBuff, datalen);
    }
    else
    {
      // nothing to send
    }
  }
  else
  {
    // wait for flow to be ON
  }
}

/**
 ****************************************************************************************
 * @brief Send a TOA Packet<br>
 * The Authentication Signature is added before sending.
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_send_packet(uint8_t* data, uint8_t datalen)
{
  uint8_t cid = data[0];

  if(TOA_CONNECTIONLESS_CID == cid)
  {
    /* No MIC for connectionless channel */
  }
  else if(TOA_BROADCAST_CID == cid)
  {
    broadcast.nonce++;
    tile_broadcast_mic_hash(broadcast.key, broadcast.nonce, &data[1], datalen-1, &data[datalen]);
    datalen += TILE_MIC_LEN;
  }
  else
  {
    toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

    if(TOA_CHANNEL_AUTHENTICATED & channel->state)
    {
      if(TOA_RSP_READY == data[1])
      {
        /* Copy in the most up-to-date nonce value */
        // TODO: remove Magic Numbers
        memcpy(&data[6], &broadcast.nonce, 4);
      }
      else if(TOA_RSP_CLOSE_CHANNEL == data[1])
      {
        toa_unassign_channel(cid);
      }
      channel->nonceT++;
      tile_mic_hash(channel->session_key, channel->nonceT, 0, &data[1], datalen-1, &data[datalen]);
      datalen += TILE_MIC_LEN;
    }
    else
    {
      /* Unassign channel here */
      toa_unassign_channel(cid);
      /* We should not send responses on unauthenticated channels */
      return;
    }
  }

  toaRspPending = true;

  toa->send_response(data, datalen);
}

/**
 ****************************************************************************************
 * @brief Init TOA environment.
 * This should be done at the time the TOA channel is open
 *
 ****************************************************************************************
 */
void toa_init(void)
{
  toaRspPending     = false;
  toaTransportReady = false;
  toaRspDropped     = 0;
  toaQueue          = initializeQueue(toa->queue, toa->queue_size);

  /* Clear all the TOA channels */
  for(uint8_t i = 0; i < toa->num_channels; i++)
  {
    toa->channels[i].state = 0;
  }

  /* Generate a random number for this connection and generate the broadcast key */
  random->random_bytes(randT_fixed, TILE_RAND_T_FIXED_LEN);

  tile_gen_broadcast_key(toa->auth_key, randT_fixed, toa->tile_id, broadcast.key);
  broadcast.nonce = 0;

  tka_init();

  /* Here seems to be the best place to init TOA features that need to be inited at each BLE connection */
  if(toa_get_feature(TOA_FEATURE_TDT))
  {
    tdt_init();
  }
  if(toa_get_feature(TOA_FEATURE_TOFU))
  {
    tofu_init();
  }
  if(toa_get_feature(TOA_FEATURE_TPS))
  {
    song_init();
  }
  if(toa_get_feature(TOA_FEATURE_TRM))
  {
    trm_init();
  }
}


/**
 ****************************************************************************************
 * @brief Process disconnection
 *
 ****************************************************************************************
 */
void toa_disconnect(void)
{
  toaTransportReady = false;

  /* Clear all TOA channels */
  for(uint8_t i = 0; i < toa->num_channels; i++)
  {
    toa->channels[i].state = 0;
  }

  /* Clear TKA as well */
  tka_init();
}


/**
 ****************************************************************************************
 * @brief Check if there is an authenticated TOA channel
 *
 * @return true if at least one channel is authenticated. false otherwise.
 *
 ****************************************************************************************
 */
bool is_toa_authenticated(void)
{
  for(int i = 0; i < toa->num_channels; i++)
  {
    if(TOA_CHANNEL_AUTHENTICATED & toa->channels[i].state)
    {
      return true;
    }
  }

  return false;
}


/**
 ****************************************************************************************
 * @brief Close a TOA channel
 *
 * @param[in] cid     Channel ID of channel to close
 *
 ****************************************************************************************
 */
void toa_channel_close(uint8_t cid, uint8_t reason, uint8_t *payload, uint8_t payload_length)
{
  uint8_t rsp[TOA_MPS];

  rsp[0] = reason;
  if(payload_length > 0 && NULL != payload)
  {
    memcpy(&rsp[1], payload, payload_length);
  }

  /* Setting the channel state to unassigned is performed at the point when the packet is sent out */

  toa_send_response(cid, TOA_RSP_CLOSE_CHANNEL, rsp, payload_length + 1);
}


/**
 ****************************************************************************************
 * @brief Set a CID to unassigned. Prepare to issue a disconnection if no channels are open
 *
 * @param[in] cid     Channel ID of channel to close
 *
 ****************************************************************************************
 */
static void toa_unassign_channel(uint8_t cid)
{
  if(TOA_BROADCAST_CID >= cid || TOA_CHANNEL_IDX(cid) >= toa->num_channels)
  {
    /* Should not happen */
    /* TODO: Assert! */
    return;
  }

  toa->channels[TOA_CHANNEL_IDX(cid)].state = 0;

  /* Stop TKA for this channel */
  timer->cancel(TILE_TKA_TIMER1 + TOA_CHANNEL_IDX(cid));

  /* Tell TOFU about it */
  if(toa_get_feature(TOA_FEATURE_TOFU))
  {
    tofu_channel_unassigned(cid);
  }

  /* If there are not more active channels, prepare a disconnection */
  for(int i = 0; i < toa->num_channels; i++)
  {
    if(TOA_CHANNEL_ASSIGNED & toa->channels[i].state)
    {
      /* There's still an active channel */
      return;
    }
  }

  timer->start(TILE_AUTHENTICATION_TIMER, TOA_DISCONNECT_TIME * TILE_TICKS_PER_SEC);
}



/********************* TOA RESPONSES **********************/

/**
 ****************************************************************************************
 * @brief Send a TOA READY Response<br>
 * This should be done at the time the TOA Chanel is open.
 *
 ****************************************************************************************
 */
static void toa_send_response_ready(uint8_t cid)
{
  uint8_t data[] = {
    TOA_MPS,
    toa_features[0],
    toa_features[1],
    toa_features[2],
    MKLE32(broadcast.nonce)
  };
  toa_send_response(cid, TOA_RSP_READY, data, sizeof(data));
}

/**
 ****************************************************************************************
 * @brief Send a TOA ERROR Response<br>
 * This is used when there is NO parameter to send with the error
 *
 * @param[in] error   The Error Code.
 * @param[in] command The TOA Command that triggered the Error.
 *
 ****************************************************************************************
 */
void toa_send_response_error(uint8_t cid, uint8_t error, uint8_t command)
{
  uint8_t data[2] = {error, command};
  toa_send_response(cid, TOA_RSP_ERROR, data, 2);
}

/**
 ****************************************************************************************
 * @brief Send a TOA ERROR Response<br>
 * This is used when there is a parameter to send with the error
 *
 * @param[in] error   The Error Code.
 * @param[in] command The TOA Command that triggered the Error.
 * @param[in] param   The parameter associated with the Error.
 *
 ****************************************************************************************
 */
void toa_send_response_error_param(uint8_t cid, uint8_t error, uint8_t command, uint8_t param)
{
  uint8_t data[3] = {error, command, param};
  toa_send_response(cid, TOA_RSP_ERROR, data, 3);
}


/**
 ****************************************************************************************
 * @brief Send a TOA ERROR Response on connectionless channel<br>
 * This is used when there is NO parameter to send with the error
 *
 * @param[in] token   The token used for this transaction.
 * @param[in] error   The Error Code.
 * @param[in] command The TOA Command that triggered the Error.
 *
 ****************************************************************************************
 */
void toa_send_connectionless_response_error(uint8_t *token, uint8_t error, uint8_t command)
{
  uint8_t data[2] = {error, command};
  toa_send_connectionless_response(token, TOA_RSP_ERROR, data, 2);
}


/********************* TOA COMMANDS **********************/

/**
 ****************************************************************************************
 * @brief Process Incomming Ipen Channel Commands
 *
 * @param[in] token      pointer to the command token.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_open_channel_command(const uint8_t *token, const uint8_t *data, uint8_t datalen)
{
  if(datalen < TILE_RAND_A_LEN)
  {
    uint8_t rsp[] = {TOA_RSP_ERROR_PARAMETERS, TOA_CMD_OPEN_CHANNEL};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
    return;
  }

  /* Find a free CID */
  uint8_t i;
  uint8_t cid = TOA_CONNECTIONLESS_CID;
  for(i = TOA_BROADCAST_CID + 1; i <= TOA_BROADCAST_CID + toa->num_channels; i++)
  {
    if(!(TOA_CHANNEL_ASSIGNED & toa->channels[TOA_CHANNEL_IDX(i)].state))
    {
      toa->channels[TOA_CHANNEL_IDX(i)].state |= TOA_CHANNEL_ASSIGNED;
      cid = i;
      break;
    }
  }

  /* If we have found an unassigned channel */
  if(TOA_CONNECTIONLESS_CID != cid)
  {
    uint8_t randT[TILE_RAND_T_LEN];
    toa_channel_t *channel = &toa->channels[TOA_CHANNEL_IDX(cid)];

    channel->nonceA = 0;
    channel->nonceT = 0;

    memcpy(randT, randT_fixed, TILE_RAND_T_FIXED_LEN);
    // generate random number
    random->random_bytes(&randT[TILE_RAND_T_FIXED_LEN], TILE_RAND_T_LEN - TILE_RAND_T_FIXED_LEN);

    tile_gen_session_key(toa->auth_key, data, randT, cid, token, channel->session_key);

    uint8_t rsp[1 + TILE_RAND_T_LEN];
    rsp[0] = cid;
    memcpy(&rsp[1], randT, TILE_RAND_T_LEN);
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_OPEN_CHANNEL, rsp, sizeof(rsp));

    /* Start TKA */
    tka_start_default(cid);
  }
  else
  {
    uint8_t rsp[] = {TOA_RSP_ERROR_NO_CID_AVAILABLE, TOA_CMD_OPEN_CHANNEL};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
  }
}

/**
 ****************************************************************************************
 * @brief Process Incomming Associate Commands
 *
 * @param[in] token      pointer to the command token.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_associate_command(const uint8_t *token, const uint8_t *data, uint8_t datalen)
{
  uint8_t aco[24];
  uint8_t retcode = TOA_ERROR_OK;
  uint8_t authorization_type = 0;
  
  if(datalen < TILE_AUTH_RAND_A_LEN)
  {
    uint8_t rsp[] = {TOA_RSP_ERROR_PARAMETERS, TOA_CMD_ASSOCIATE};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
    return;
  }

  uint8_t randT[TILE_AUTH_RAND_T_LEN];
  uint8_t sresT[TILE_SRES_LEN];
  random->random_bytes(randT, TILE_AUTH_RAND_T_LEN);

  tile_device_auth_hash(toa->auth_key, data, randT, sresT, aco);
  if(NULL != toa->associate)
  {
    retcode = toa->associate(aco, &authorization_type);
  }
  
  if(TOA_ERROR_OK == retcode)
  {
    uint8_t rsp[TILE_AUTH_RAND_T_LEN+TILE_SRES_LEN];
    memcpy(rsp, randT, TILE_AUTH_RAND_T_LEN);
    memcpy(rsp + TILE_AUTH_RAND_T_LEN, sresT, TILE_SRES_LEN);
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ASSOCIATE, rsp, sizeof(rsp));
  }
  else if(TOA_RSP_ERROR_AUTHORIZATION == retcode)
  {
    uint8_t rsp[] = {TOA_RSP_ERROR_AUTHORIZATION, TOA_CMD_ASSOCIATE, authorization_type};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
  }
  else
  {
    uint8_t rsp[] = {retcode, TOA_CMD_ASSOCIATE};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
  }
}


/**
 ****************************************************************************************
 * @brief Process Incomming Ready Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_ready_command(const uint8_t cid, const uint8_t* pdata, uint8_t datalen)
{
  if(NULL != toa->channel_open_count)
  {
    (*toa->channel_open_count)++;
  }

  if(datalen > 1)
  {
    toa_client_features[0] = pdata[1];
  }
  toa_send_response_ready(cid);

  // TODO: move this to TDT probably
  if(tdt && (TDT_HDC_STATUS_NORMAL != tdt->hdc_status))
  {
    tdt->hdc_status = TDT_HDC_STATUS_NOTIFY;
    timer->cancel(TILE_TDT_HDC_TIMER);
    timer->start(TILE_TDT_HDC_TIMER, 2);
  }
}

/**
 ****************************************************************************************
 * @brief Process Incomming Authenticate Commands
 *
 * @param[in] token      pointer to the command token.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_authenticate_command(const uint8_t *token, const uint8_t *data, uint8_t datalen)
{
  uint8_t aco[24];

  if(datalen < TILE_AUTH_RAND_A_LEN)
  {
    uint8_t rsp[] = {TOA_RSP_ERROR_PARAMETERS, TOA_CMD_AUTHENTICATE};
    toa_send_connectionless_response((uint8_t*)token, TOA_RSP_ERROR, rsp, sizeof(rsp));
    return;
  }

  if(NULL != toa->authenticate_count)
  {
    (*toa->authenticate_count)++;
  }

  uint8_t randT[TILE_AUTH_RAND_T_LEN];
  uint8_t sresT[TILE_SRES_LEN];
  random->random_bytes(randT, TILE_AUTH_RAND_T_LEN);

  tile_device_auth_hash(toa->auth_key, data, randT, sresT, aco);

  uint8_t rsp[TILE_AUTH_RAND_T_LEN+TILE_SRES_LEN];
  memcpy(rsp, randT, TILE_AUTH_RAND_T_LEN);
  memcpy(rsp + TILE_AUTH_RAND_T_LEN, sresT, TILE_SRES_LEN);

  toa_send_connectionless_response((uint8_t*)token, TOA_RSP_AUTHENTICATE, rsp, sizeof(rsp));
}


/**
 ****************************************************************************************
 * @brief Register ADV_INT Module
 *
 * @param[in] module       pointer to the ADV_INT module. Setting NULL will declare support without actually supporting it.
 *
 ****************************************************************************************
 */
int tile_adv_int_register(struct tile_adv_int_module *module)
{
  adv = module;

  /* Enable feature in TOA */
  toa_set_feature(TOA_FEATURE_ADV_INT);

  return TILE_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Process Incomming ADV_INT Commands
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void toa_process_advInt_command(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  uint16_t adv_interval;

  if((NULL == data) || (datalen < 1))
  {
    toa_send_response_error(cid, TOA_RSP_ERROR_PARAMETERS, TOA_CMD_ADV_INT);
  }
  else
  {
    switch(data[0])
    {
      case ADV_INT_CMD_READ_FEATURES:
      {
        uint8_t rsp[2] = {ADV_INT_RSP_READ_FEATURES_OK, 3};  // 3 stands for Read and Write
        toa_send_response(cid, TOA_RSP_ADV_INT, &rsp[0], 2);
      }
      break;
      case ADV_INT_CMD_READ_VAL:
      {
        uint8_t rsp[3];
        uint16_t adv_int;

        adv->get(&adv_int);

        rsp[0] = ADV_INT_RSP_READ_VAL_OK;
        memcpy(&rsp[1], &adv_int, 2);
        toa_send_response(cid, TOA_RSP_ADV_INT, &rsp[0], 3);
      }
      break;
      case ADV_INT_CMD_WRITE_VAL:
        if(datalen < 3)
        {
          toa_send_response_error_param(cid, TOA_RSP_ERROR_PARAMETERS, TOA_CMD_ADV_INT, data[0]);
        }
        else
        {
          memcpy(&adv_interval, &data[1], 2);

          if((adv_interval >= 800) && (adv_interval <= 8000))
          {
            uint8_t rsp[3];

            uint16_t current_adv_int;
            adv->get(&current_adv_int);

            /* save it in persistent memory if needed */
            if(current_adv_int != adv_interval)
            {
              adv->set(adv_interval);
            }
            /* Respond with OK */
            rsp[0] = ADV_INT_RSP_WRITE_VAL_OK;
            memcpy(&rsp[1], &adv_interval, 2);
            toa_send_response(cid, TOA_RSP_ADV_INT, &rsp[0], 3);
          }
          else
          {
            toa_send_response_error(cid, TOA_RSP_ERROR_PARAMETERS, TOA_CMD_ADV_INT);
          }
        }
      break;
      default:
        toa_send_response_error_param(cid, TOA_RSP_ERROR_UNSUPPORTED, TOA_CMD_ADV_INT, data[0]);
      break;
    }
  }
}


