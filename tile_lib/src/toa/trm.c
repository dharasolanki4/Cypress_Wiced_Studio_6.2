/*
 * NOTICE
 *
 * © 2017 Tile Inc.  All Rights Reserved.

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

 /** @file trm.c
 ** @brief Tile RSSI Measure
 */
#include "string.h"
#include "trm.h"
#include "toa.h"
#include "tile_lib.h"
#include "modules/tile_trm_module.h"

static uint8_t cached_cid = 0;
static uint8_t flag_notify;             // Used for periodic rssi notification
static uint8_t flag_read;               // Used to notify rssi value only one time
static uint8_t rssi_samples = 0;        // number of samples in each notification
static int8_t rssi_values[TOA_MPS-2];   // array of rssi samples to be notified
static uint8_t rssi_index;                   // rssi array index


/**
 ****************************************************************************************
 * @brief This fucntion notifies application of rssi values with specified sample size
 *        if it has previously requested for notification.
 *
 * @param[in] rssi_values       pointer to array of rssi values to be notified
 *            len               number of rssi samples that will be notified
 *
 ****************************************************************************************
 */
static void trm_send_rsp_notify(int8_t *rssi_values, uint8_t len)
{
  int8_t transaction[TOA_MPS] = { 0 };

  transaction[0]  = TRM_RSP_NOTIFY;
  transaction[1]  = len;

  if(len > TOA_MPS - 2)
  {
    // Todo take some action here?
    return;
  }
  else
  {
    memcpy(&transaction[2], rssi_values, len);
  }

  toa_send_response(cached_cid, TOA_RSP_TRM, (uint8_t*) transaction, len+2);
}


/**
 ****************************************************************************************
 * @brief This function should be called in the application every time there is a
 *        new rssi value to be read. Based on the flag set for rssi notification,
 *        it will notify the rssi values once the sample size is reached.
 *
 * @param[in] rssi       rssi value every connection event when there is no
 *                       packet error.
 *
 ****************************************************************************************
 */
void tile_trm_rssi(int8_t rssi)
{
  // nothing to do if TRM feature is not enabled
  if(!toa_get_feature(TOA_FEATURE_TRM))
  {
    return;
  }
  // if no flags are set, no need to notify rssi values
  if(1 != flag_read && 1 != flag_notify)
  {
    return;
  }
  else
  {
    // Fill rssi array
    rssi_values[rssi_index++] = rssi;
    if(rssi_index == rssi_samples)
    {
      // Notify rssi values when the sample size is reached
      trm_send_rsp_notify(rssi_values, rssi_samples);
      rssi_index = 0;
      if(1 == flag_read)
      {
        // Notify only one time
        flag_read = 0;
        cached_cid = 0;
      }
    }
  }
  return;
}


/**
 ****************************************************************************************
 * @brief Init TRM environment.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, trm_init, (void))
{
  flag_notify = 0;
  flag_read = 0;
  cached_cid = 0;
  rssi_index = 0;
}


/**
 ****************************************************************************************
 * @brief Process Incoming TRM Commands
 * @param[in] cid        channel id.
 * @param[in] data       pointer to data.
 * @param[in] datalen    number of bytes of data.
 *
 ****************************************************************************************
 */
TOA_EXTERN_IMPL(void, trm_process_command, (const uint8_t cid, const uint8_t* data, uint8_t datalen))
{
  switch(data[0])
  {
    case TRM_CMD_START_NOTIFY:
    {
      if(0 != cached_cid)
      {
        /* Send resource in use error */
        uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_RESOURCE_IN_USE};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
        return;
      }
      // RSSI sample size should be greater than 0 and less than TOA_MPS-2
      // TOA_MPS-2 because the response notification payload will have
      // 1 byte response code and 1 byte rssi_sample size that is being notified.
      if(datalen < 2 || 0 == data[1] || data[1] > TOA_MPS-2)
      {
        uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
      }
      else
      {
        flag_notify = 1;          // Flag set for periodic notification
        cached_cid = cid;         // cache channel id here
        rssi_samples = data[1];   // Number of samples in each notification
        uint8_t rsp[] = {TRM_RSP_START_NOTIFY};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
      }
    }
    break;
    case TRM_CMD_STOP_NOTIFY:
    {
      if(cached_cid != cid)
      {
        /* Send resource in use error */
        uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_RESOURCE_IN_USE};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
        return;
      }
      else
      {
        // Reset flag, cached_cid and index
        flag_notify = 0;
        cached_cid = 0;
        rssi_index = 0;
        uint8_t rsp[] = {TRM_RSP_STOP_NOTIFY};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
      }
    }
    break;
    case TRM_CMD_READ:
    {
      if(0 != cached_cid)
      {
        /* Send resource in use error */
        uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_RESOURCE_IN_USE};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
        return;
      }
      // RSSI sample size should be greater than 0 and less than TOA_MPS-2
      // TOA_MPS-2 because the response notification payload will have
      // 1 byte response code and 1 byte rssi_sample size that is being notified.
      if(datalen < 2 || 0 == data[1] || data[1] > TOA_MPS-2)
      {
        uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
      }
      else
      {
        flag_read = 1;          // Flag set for one time notification
        cached_cid = cid;       // Cache channel id here
        rssi_samples = data[1]; // Number of samples in the notification
        uint8_t rsp[] = {TRM_RSP_READ};
        toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
      }
    }
    break;
    default:
    {
      uint8_t rsp[] = {TRM_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TRM, rsp, sizeof(rsp));
    }
    break;
  }
}




int tile_trm_register(void)
{
  toa_set_feature(TOA_FEATURE_TRM);

  TOA_EXTERN_LINK(trm_process_command);
  TOA_EXTERN_LINK(trm_init);

  return TILE_SUCCESS;
}
