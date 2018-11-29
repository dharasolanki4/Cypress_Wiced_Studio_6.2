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

 /** @file tdt.c
 ** @brief Tile Double Tap
 */



#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tile_lib.h"
#include "drivers/tile_timer_driver.h"
#include "drivers/tile_button_driver.h"
#include "modules/tile_tdt_module.h"
#include "modules/tile_song_module.h"
#include "song.h"
#include "toa.h"
#include "tdt.h"

uint8_t tdt_doubletapping, tdt_debouncing_STI, tdt_debouncing_STD, tdt_debouncing_DT, tdt_debouncing_LT;

struct tile_tdt_module *tdt;

extern struct tile_timer_driver *timer;
extern struct tile_gatt_server_driver *gatt_server;
extern struct tile_button_driver *button;
extern struct tile_song_module *song;

static void tdt_process_cmd_config(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void tdt_send_rsp_config(uint8_t cid);
static void tdt_send_rsp_error(uint8_t cid, uint8_t rsp, uint8_t param);
static bool tdt_is_ready(void);
static void tdt_send_rsp_readConfig(uint8_t cid);



/**
 ****************************************************************************************
 * @brief process incomming TDT command packets
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
    case TDT_CMD_CONFIG:
      tdt_process_cmd_config(cid, &data[1], (datalen-1));
      break;
    case TDT_CMD_READ_CONFIG:
      tdt_send_rsp_readConfig(cid);
      break;
    default:
      tdt_send_rsp_error(cid, TDT_RSP_ERROR_UNSUPPORTED, data[0]);
      break;
  }
}

/**
 ****************************************************************************************
 * @brief process an @ref TDT_CMD_CONFIG Packet
 *
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
static void tdt_process_cmd_config(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  if(datalen >= sizeof(tdt_config_t))
  {
    tdt_config_t tmp_config;

    memcpy((void*)&tmp_config, data, sizeof(tdt_config_t));

    /* Update config only if needed */
    if(0 != memcmp(&tmp_config, &tdt->config, sizeof(tdt_config_t)))
    {
      memcpy(&tdt->config, &tmp_config, sizeof(tdt_config_t));
      tdt->config_written(&tdt->config);
    }

    tdt_send_rsp_config(cid);
  }
  else
  {
    tdt_send_rsp_error(cid, TDT_RSP_ERROR_PARAMS, TDT_CMD_CONFIG);
  }
}

/**
 ****************************************************************************************
 * @brief send an @ref TDT_RSP_CONFIG Response
 *
 ****************************************************************************************
 */
static void tdt_send_rsp_config(uint8_t cid)
{
  uint8_t transaction[TOA_MPS];
  memset(&transaction, 0, sizeof(transaction));

  transaction[0]  = TDT_RSP_CONFIG;
  memcpy(&transaction[1], &tdt->config, sizeof(tdt_config_t));

  toa_send_response(cid, TOA_RSP_TDT, transaction, 5);
}

/**
 ****************************************************************************************
 * @brief send an @ref TDT_RSP_READ_CONFIG Response
 *
 ****************************************************************************************
 */
static void tdt_send_rsp_readConfig(uint8_t cid)
{
uint8_t transaction[TOA_MPS] = { 0 };

  transaction[0]  = TDT_RSP_READ_CONFIG;
  memcpy(&transaction[1], &tdt->config, sizeof(tdt_config_t));

  toa_send_response(cid, TOA_RSP_TDT, transaction, 5);
}

/**
 ****************************************************************************************
 * @brief Send a TDT ERROR Response
 *
 * @param[in] rsp   The TDT Error Code.
 * @param[in] param The param to send with the Error.
 *
 ****************************************************************************************
 */
static void tdt_send_rsp_error(uint8_t cid, uint8_t rsp, uint8_t param)
{
uint8_t transaction[TOA_MPS] = { 0 };

  transaction[0]  = rsp;
  transaction[1]  = param;

  toa_send_response(cid, TOA_RSP_TDT, transaction, 2);
}

/**
 ****************************************************************************************
 * @brief send an @ref TDT_RSP_NOTIFY Response
 *
 ****************************************************************************************
 */
static void tdt_send_rsp_notify(uint8_t type)
{
uint8_t transaction[TOA_MPS] = { 0 };

  transaction[0]  = TDT_RSP_NOTIFY;
  transaction[1]  = type;

  toa_send_broadcast(TOA_RSP_TDT, (uint8_t*) &transaction, 2);
}


/**
 ****************************************************************************************
 * @brief Check that TDT is in a state where it can successfully notify
 *
 * @return true is ready, false if not
 *
 ****************************************************************************************
 */
static bool tdt_is_ready(void)
{
  if(is_toa_authenticated())
  {
    return true;
  }
  return false;
}

/**
 ****************************************************************************************
 * @brief Init TDT environment.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_init, (void))
{
  tdt_debouncing_STI  = false;
  tdt_debouncing_STD  = false;
  tdt_debouncing_DT   = false;
  tdt_debouncing_LT   = false;

  // clear the timers too
  timer->cancel(TILE_TDT_STI_TIMER);
  timer->cancel(TILE_TDT_STD_TIMER);
  timer->cancel(TILE_TDT_DT_TIMER);
  timer->cancel(TILE_TDT_LT_TIMER);
}

/**
 ****************************************************************************************
 * @brief Try notify of the TDT event. Takes care of Notification Debouncing.
 *
 * @param[in] type     The type of Tap to notify
 *
 ****************************************************************************************
 */
static void tdt_process_notify(uint8_t type)
{
uint8_t DebounceDelay = tdt->config.NotifDebounceDelay;    // Improve readability

  switch(type)
  {
    case TDT_NOTIFY_STI:
        if(false == tdt_debouncing_STI)
        {
          if(DebounceDelay)
          {
            // apply notification debouncing
            tdt_debouncing_STI = true;
            timer->start(TILE_TDT_STI_TIMER, DebounceDelay * TDT_NOTIF_DEBOUNCE_DELAY_INCREMENT);
          }
          tdt_send_rsp_notify(type);
        }
        break;
    case TDT_NOTIFY_STD:
        if(false == tdt_debouncing_STD)
        {
          if(DebounceDelay)
          {
            // apply notification debouncing
            tdt_debouncing_STD = true;
            timer->start(TILE_TDT_STD_TIMER, DebounceDelay * TDT_NOTIF_DEBOUNCE_DELAY_INCREMENT);
          }
          tdt_send_rsp_notify(type);
        }
        break;
    case TDT_NOTIFY_DT:
        if(false == tdt_debouncing_DT)
        {
          if(DebounceDelay)
          {
            // apply notification debouncing
            tdt_debouncing_DT = true;
            timer->start(TILE_TDT_DT_TIMER, DebounceDelay * TDT_NOTIF_DEBOUNCE_DELAY_INCREMENT);
          }
          if(NULL != tdt->double_tap_notify)
          {
            (*tdt->double_tap_notify)++;
          }
          tdt_send_rsp_notify(type);
        }
        break;
    case TDT_NOTIFY_LT:
        if(false == tdt_debouncing_LT)
        {
          if(DebounceDelay)
          {
            // apply notification debouncing
            tdt_debouncing_LT = true;
            timer->start(TILE_TDT_LT_TIMER, DebounceDelay * TDT_NOTIF_DEBOUNCE_DELAY_INCREMENT);
          }
          tdt_send_rsp_notify(type);
        }
        break;
  }
}

/**
 ****************************************************************************************
 * @brief Triggers required actions based on settings when a Tap is detected.
 *
 * @param[in] type     The type of Tap to notify
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_process_tap, (uint8_t type))
{
  switch(type)
  {
    case TDT_NOTIFY_STI:
      if(NULL != tdt->single_tap)
      {
        (*tdt->single_tap)++;
      }
      if(tdt->config.EN_STI)
      {
        if(true == tdt_is_ready())
        {
          tdt_process_notify(type);
          if(true == tdt->config.SE_STIS && toa_get_feature(TOA_FEATURE_SONG))
          {
            song->play(TILE_SONG_DT_SUCCESS, tdt->config.SS_Strength);
          }
        }
        else if(true == tdt->config.SE_STIF && toa_get_feature(TOA_FEATURE_SONG))
        {
          song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
        }
      }
      break;
    case TDT_NOTIFY_STD:
      if(tdt->config.EN_STD)
      {
        if(true == tdt_is_ready())
        {
          tdt_process_notify(type);
          if(true == tdt->config.SE_STDS && toa_get_feature(TOA_FEATURE_SONG))
          {
            song->play(TILE_SONG_DT_SUCCESS, tdt->config.SS_Strength);
          }
        }
        else if(true == tdt->config.SE_STDF && toa_get_feature(TOA_FEATURE_SONG))
        {
          song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
        }
      }
      break;
    case TDT_NOTIFY_DT:
      if(NULL != tdt->double_tap_detect)
      {
        (*tdt->double_tap_detect)++;
      }
      if(tdt->config.EN_DT)
      {
        if(true == tdt_is_ready())
        {
          tdt_process_notify(type);
          if(true == tdt->config.SE_DTS && toa_get_feature(TOA_FEATURE_SONG))
          {
            song->play(TILE_SONG_DT_SUCCESS, tdt->config.SS_Strength);
          }
        }
        else
        {
          if(true == tdt->config.SE_DTF) {
             if(NULL != tdt->hdc_cb)
             {
               tdt->hdc_status = TDT_HDC_STATUS_IBEACON;
               tdt->hdc_cb();
               timer->cancel(TILE_TDT_HDC_TIMER);
               timer->start(TILE_TDT_HDC_TIMER, TDT_HDC_IBEACON_DURATION);
               if(toa_get_feature(TOA_FEATURE_SONG))
               {
                 song->play(TILE_SONG_DT_HB, tdt->config.FS_Strength);
               }
             }
             else if(toa_get_feature(TOA_FEATURE_SONG))
             {
               song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
             }
          }
        }
      }
      break;
    case TDT_NOTIFY_LT:
      if(NULL != tdt->long_tap)
      {
        (*tdt->long_tap)++;
      }
      if(tdt->config.EN_LT)
      {
        if(true == tdt_is_ready())
        {
          tdt_process_notify(type);
          if(true == tdt->config.SE_LTS && toa_get_feature(TOA_FEATURE_SONG))
          {
            song->play(TILE_SONG_DT_SUCCESS, tdt->config.SS_Strength);
          }
        }
        else if(true == tdt->config.SE_LTF && toa_get_feature(TOA_FEATURE_SONG))
        {
          song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
        }
      }
      break;
  }
}

/**
 ****************************************************************************************
 * @brief Tap detection function. Called on Button press.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_process_button_press, (void))
{
  /* single tap immediate detected */
  tdt_process_tap(TDT_NOTIFY_STI);

  /* check if we are under time detection threshold since last push */
  if(tdt_doubletapping)
  {
    /* double tap detected */
    tdt_doubletapping = false;
    /* clear timer */
    timer->cancel(TILE_TDT_DOUBLETAP_TIMER);
    tdt_process_tap(TDT_NOTIFY_DT);
  }
  else
  {
    /* start timer */
    timer->start(TILE_TDT_DOUBLETAP_TIMER,
      tdt->config.Delay*2 + TDT_DELAY_OFFSET);

    tdt_doubletapping = true;
  }
}

/**
 ****************************************************************************************
 * @brief DoubleTap timer callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_doubletap_timer_handler, (void))
{
  /* TODO: Re-enable long tapping */
  if(true == tdt_doubletapping)
  {
    uint8_t button_state;
    button->read_state(&button_state);
    if(button_state == TILE_BUTTON_PRESSED)
    {
      /* LongTap detected */
      tdt_process_tap(TDT_NOTIFY_LT);
    }
    else
    {
      /* SingleTap delayed detected */
      tdt_process_tap(TDT_NOTIFY_STD);
    }
  }
  tdt_doubletapping = false;
}


/**
 ****************************************************************************************
 * @brief STI Notification Debouncing Timer Callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_STI_timer_handler, (void))
{
  tdt_debouncing_STI  = false;
}

/**
 ****************************************************************************************
 * @brief STD Notification Debouncing Timer Callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_STD_timer_handler, (void))
{
  tdt_debouncing_STD  = false;
}

/**
 ****************************************************************************************
 * @brief LT Notification Debouncing Timer Callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_LT_timer_handler, (void))
{
  tdt_debouncing_LT  = false;
}

/**
 ****************************************************************************************
 * @brief DT Notification Debouncing Timer Callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_DT_timer_handler, (void))
{
  tdt_debouncing_DT  = false;
}

/**
 ****************************************************************************************
 * @brief HDC Notification Timer Callback.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, tdt_HDC_timer_handler, (void))
{
  switch(tdt->hdc_status)
  {
    case TDT_HDC_STATUS_NORMAL:
      tdt->hdc_status = TDT_HDC_STATUS_NORMAL;
      tdt->hdc_cb();
      break;
    case TDT_HDC_STATUS_IBEACON:
      // TDT HDC iBeacon
      song->play(TILE_SONG_DT_HB, tdt->config.FS_Strength);

      // switch to fast advertising
      tdt->hdc_status = TDT_HDC_STATUS_FAST_ADV;

      /* start timer */
      timer->start(TILE_TDT_HDC_TIMER, TDT_HDC_ADVERTISING_STEP_DURATION);
      tdt->hdc_cb();
      break;
    case TDT_HDC_STATUS_FAST_ADV:
    case TDT_HDC_STATUS_FAST_ADV2:
    case TDT_HDC_STATUS_FAST_ADV3:
      // TDT HDC advertising
      song->play(TILE_SONG_DT_HB, tdt->config.FS_Strength);
      timer->start(TILE_TDT_HDC_TIMER, TDT_HDC_ADVERTISING_STEP_DURATION);
      tdt->hdc_status++;
      break;
    case TDT_HDC_STATUS_FAST_ADV4:
      // TDT HDC advertising
      /*  Last heartbeat before end of HDC Advertising, so timer duration is different. */
      song->play(TILE_SONG_DT_HB, tdt->config.FS_Strength);
      timer->start(TILE_TDT_HDC_TIMER, TDT_HDC_ADVERTISING_LAST_STEP_DURATION);
      tdt->hdc_status++;
      break;
    case TDT_HDC_STATUS_FAST_ADV5:
      // go back to normal advertising
      tdt->hdc_status = TDT_HDC_STATUS_NORMAL;
      if(NULL != tdt->double_tap_failure2)
      {
        (*tdt->double_tap_failure2)++;
      }
      song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
      tdt->hdc_cb();
      break;
    case TDT_HDC_STATUS_NOTIFY:
      // Connected
      // notify and go back to normal
      tdt->hdc_status = TDT_HDC_STATUS_NORMAL;
      if(true == tdt_is_ready())
      {
        tdt_process_notify(TDT_NOTIFY_DT);
        song->play(TILE_SONG_DT_SUCCESS, tdt->config.FS_Strength);
      }
      else
      {
        if(NULL != tdt->double_tap_failure2)
        {
          (*tdt->double_tap_failure2)++;
        }
        song->play(TILE_SONG_DT_FAILURE, tdt->config.FS_Strength);
      }
      break;
   }
}



/**
 ****************************************************************************************
 * @brief Register for TDT
 *
 * @param[in] module     Pointer to the module info.
 *
 ****************************************************************************************
 */
int tile_tdt_register(struct tile_tdt_module *module)
{
  /* TODO: Error checking? */
  tdt = module;

  toa_set_feature(TOA_FEATURE_TDT);

  TOA_EXTERN_LINK(tdt_process_command);
  TOA_EXTERN_LINK(tdt_process_button_press);
  TOA_EXTERN_LINK(tdt_init);
  TOA_EXTERN_LINK(tdt_process_tap);
  TOA_EXTERN_LINK(tdt_DT_timer_handler);
  TOA_EXTERN_LINK(tdt_LT_timer_handler);
  TOA_EXTERN_LINK(tdt_STD_timer_handler);
  TOA_EXTERN_LINK(tdt_STI_timer_handler);
  TOA_EXTERN_LINK(tdt_doubletap_timer_handler);
  TOA_EXTERN_LINK(tdt_HDC_timer_handler);

  return TILE_SUCCESS;
}
