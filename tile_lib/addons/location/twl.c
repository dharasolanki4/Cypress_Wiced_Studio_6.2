/*
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

/** @file twl.c
 ** @brief Tile Wifi Location
 **   Every Interval, a Wifi Scan is triggered to collect MAC, RSSI pairs.
 **   After the list of SSIDS is received, it is sent over cellular and then we wait for a position from the server.
 */

// common
#include "tile_schedule/tile_schedule.h"
#include "tile_timers/tile_timers.h"
#include "tile_logs/tile_logs.h"

#include "wifi/wifi.h"
#include "networking/tna.h"
#include "cellular/cellular.h"

#include <stdint.h>
#include <string.h>


//#define TWL_SCAN_INTERVAL  1000*60*5  // in ms
#define TWL_SCAN_INTERVAL  1000*20      // in ms
#define TWL_SCAN_MAX_AP    5


struct twl_ap_report_t
{
  uint8_t mac[6];  ///< mac
  uint8_t channel; ///< channel
  int8_t  rssi;
};

struct twl_event_t
{
  uint16_t                CmdCode;    // TNA C0mmandCode
  uint16_t                CmdLength;
  struct twl_ap_report_t  ApReport[TWL_SCAN_MAX_AP];
};


static void twl_WifiScanCallback(wifi_op_status_t op_status, uint8_t number_aps, wifi_scan_result_t* res);
static void twl_launchScan(void);
static void twl_sendEvent(void);

TILE_TIMER_DEF(twl_timer);
static TILE_TIMER_HANDLER(twl_timerHandler);

static wifi_scan_result_t twl_scanRes;
static uint8_t twl_apCount;
static struct twl_event_t twl_event;


/******************************************************************************
 * Global functions
 *****************************************************************************/

/**
 * @brief Starts Wifi location tracking
 */
void twl_start(void)
{
  static uint8_t timer_inited = 0;

  if(timer_inited == 0)
  {
    // Call TILE_TIMER_INIT only one time as it creates issues otherwise (infinite loop)
    timer_inited = 1;
    TILE_TIMER_INIT(twl_timer, twl_timerHandler);
  }

  twl_launchScan();
  TILE_TIMER_START(twl_timer, TWL_SCAN_INTERVAL, NULL);
}


/******************************************************************************
 * Internal functions
 *****************************************************************************/

/**
 * Fill the buffer and send the TWL Packet
 */
static void twl_WifiScanCallback(wifi_op_status_t op_status, uint8_t number_aps, wifi_scan_result_t* res)
{
  LOG("%s op_status %d, number_aps %d\r\n", __FUNCTION__, op_status, number_aps);

  if(NULL != res)
  {
      LOG("[%d]  SSID:    %s\r\n", res->index, res->SSID);
      LOG("     Channel: %d\r\n", res->ch);
      LOG("     RSSI:    %d\r\n", res->rssi);
      LOG("     Auth:    %d\r\n", res->AuthType);
      LOG("     BSSID:   %02x:%02x:%02x:%02x:%02x:%02x\r\n", res->BSSID[0],
          res->BSSID[1], res->BSSID[2], res->BSSID[3], res->BSSID[4], res->BSSID[5]);
  }

  if(NULL != res && twl_apCount < TWL_SCAN_MAX_AP)
  {
    // copy the AP info in the packet.
    //TODO: we should probably select the APs with highest RSSI instead of just the first ones.
    twl_event.ApReport[twl_apCount].channel = res->ch;
    twl_event.ApReport[twl_apCount].rssi = res->rssi;
    memcpy(twl_event.ApReport[twl_apCount].mac, res->BSSID, 6);
    twl_apCount++;
  }

  if(WIFI_OP_STATUS_SUCCESS == op_status)
  {
    // send the packet if done
    twl_sendEvent();
  }
}

/**
 * Time to launch a Wifi Sniff
 */
static void twl_launchScan(void)
{
  LOG("TWL: Launching Wifi Sniff\r\n");

  //TODO: verify whether we are already scanning
  memset(&twl_event, 0, sizeof(twl_event));
  twl_apCount   = 0;
  memset(&twl_scanRes, 0, sizeof(twl_scanRes));
  wifi_scan(twl_WifiScanCallback, TWL_SCAN_MAX_AP, &twl_scanRes);
}

/**
 * Time to launch a Wifi Sniff
 */
static TILE_TIMER_HANDLER(twl_timerHandler)
{
  twl_launchScan();
  TILE_TIMER_START(twl_timer, TWL_SCAN_INTERVAL, NULL);
}

/**
 * Send a TWL Event over cellular
 */
static void twl_sendEvent(void)
{
  uint8_t buffer[150];
  uint32_t frameSize = 0;

  LOG("TWL: twl_sendEvent\r\n");

  twl_event.CmdCode   = TNA_CMD_WLAN_SNIFF_EVENT;
  twl_event.CmdLength = twl_apCount*sizeof(struct twl_ap_report_t);

  frameSize = TNA_formatFrame(buffer, twl_event.CmdLength+4, (uint8_t*) &twl_event);
  //cellular_send(NULL, buffer, frameSize);
  LOG("TWL: cellular is OFF\r\n");
}


