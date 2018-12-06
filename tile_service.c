/*
 * NOTICE
 *
 * Â© 2017 Tile Inc.  All Rights Reserved.
 *
 * All code or other information included in the accompanying files (â€œTile Source Materialâ€�)
 * is CONFIDENTIAL and PROPRIETARY information of Tile Inc. and your access and use is subject
 * to the terms of Tileâ€™s non-disclosure agreement as well as any other applicable agreement
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
 * @file tile_service.c
 * @brief Core functionality for Tile Lib
 */

#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"

#include "hello_sensor.h"
#include "tile_config.h"
#include "tile_features.h"
#include "tile_service.h"
#include "tile_storage.h"
//#include "tile_assert/tile_assert.h"

// TileLib includes
#include "tile_lib.h"
#include "tile_lib/src/toa/toa.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_tmd_module.h"

#include <stdbool.h>
#include <string.h> 

#ifdef  WICED_BT_TRACE_ENABLE
#include "wiced_bt_trace.h"
#endif
/*******************************************************************************
 * Forward declarations
 ******************************************************************************/

void tile_timer_timeout_handler(uint32_t arg);

/*******************************************************************************
 * Defines & types
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
#if OLD_CODE
app_timer_t    tile_timer_data[TILE_MAX_TIMERS] = { 0 };
#endif

/*******************************************************************************
 * Local variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

/**
 * @brief Initialize Tile BLE service
 */
void tile_service_init(void)
{ 
  wiced_bt_gatt_status_t tile_gatt_status;

  tile_storage_init(); // Initialize storage before initializing features
	
  tile_features_init();
#if OLD_CODE
  /* Initialize Tile timers */
  for(int i=0; i < TILE_MAX_TIMERS; i++)
  {
    tile_timer_id[i] = &tile_timer_data[i];
    /* We use one universal timeout handler with p_context containing the timer ID for dispatch to the shim */
    app_timer_create(&tile_timer_id[i], APP_TIMER_MODE_SINGLE_SHOT, tile_timer_timeout_handler);
  }
#endif

  /* Initialize timer driver */
  for(int i = 0; i < TILE_MAX_TIMERS; i++)
  {
    wiced_init_timer(&tile_timer[i], &tile_timer_timeout_handler, i, WICED_MILLI_SECONDS_TIMER);
    // Add Tile assert if init timer does not return WICED_SUCCESS
  }

  ///* Register Tile Service Characteristics: Added at another place*/
  //tile_gatt_db_init(&tile_ble_env.service);
  
  /* Register with stack to receive GATT callback for Tile service */
 // tile_gatt_status = wiced_bt_gatt_register( tile_gatts_callback );

  WICED_BT_TRACE( "TILE wiced_bt_gatt_register: %d\r\n", tile_gatt_status );

}

/**
 * @brief Handle Tile BLE GATT events
 *
 * @param[in] p_evt    Event forwarded from BLE stack.
 */
wiced_bt_gatt_status_t tile_gatts_callback( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data) // tile_on_ble_evt
{
    wiced_result_t result = WICED_BT_GATT_INVALID_PDU;
    struct tile_conn_params params;

    switch(event)
    {
    case GATT_CONNECTION_STATUS_EVT:
        if (p_data->connection_status.connected) // Connected
        {
            WICED_BT_TRACE("Tile: CONNECTED\r\n");
            /* Save connection status */
            tile_ble_env.tile_gatt_connection_status = p_data->connection_status.connected;
            //tile_ble_env.conn_handle = p_evt->evt.gap_evt.conn_handle;
            //tile_unchecked->connection_count++;
            if(TILE_MODE_ACTIVATED != tile_checked->mode)
            {
              //  when the Tile is not activated, the Interim TileID, Key is used.
              memcpy(tile_checked->tile_id, interim_tile_id, 8);
              memcpy(tile_checked->tile_auth_key, interim_tile_key, 16);
            }
            /* Tell Tile Lib about the connection */
            params.conn_interval    = BTM_BLE_CONN_INTERVAL_MAX_DEF;  // 40: 40*1.25 = 50 ms
            params.slave_latency    = BTM_BLE_CONN_SLAVE_LATENCY_DEF; // 0
            params.conn_sup_timeout = BTM_BLE_CONN_TIMEOUT_DEF;       // 2000

            // To Do: Don't use default, figure out how to obtain existing conn params
            tile_gap_connected(&params);
        }
        else // Disconnected
        {
            WICED_BT_TRACE("Tile: DISCONNECTED\r\n");
            //tile_unchecked->disconnect_count++;
            tile_gap_disconnected();
        }
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        if (p_data->attribute_request.request_type == GATTS_REQ_TYPE_WRITE)
        {
            WICED_BT_TRACE("Tile: ATTRIBUTE_REQUEST WRITE\r\n");
            wiced_bt_gatt_write_t * p_gatt_data = &(p_data->attribute_request.data.write_req);
            // TO DO: Add queue mechanism
            if (p_gatt_data->handle == HANDLE_HSENS_TILE_SERVICE_CHAR_MEP_TOA_CMD_VAL)
            {
                tile_toa_command_received(p_gatt_data->p_val,p_gatt_data->val_len); // Tell tile_lib about Toa Cmd
            }
            else if (p_gatt_data->handle == HANDLE_HSENS_TILE_SERVICE_CHAR_MEP_TOA_RSP_CFG_DESC)
            {
                tile_toa_transport_ready(p_gatt_data->p_val[0]); // initialite RSP
            }
        }
        else if (p_data->attribute_request.request_type == GATTS_REQ_TYPE_CONF)
        {
            WICED_BT_TRACE("Tile: ATTRIBUTE_REQUEST CONF\r\n");
            tile_toa_response_sent_ok();
        }
        break;
    default:
        break;
    }

    return WICED_BT_GATT_SUCCESS;
}


/*******************************************************************************
 * Local functions
 ******************************************************************************/


/**
 * @brief Timer handler for Tile timers
 */
void tile_timer_timeout_handler(uint32_t timer_id)
{
  tile_timer_expired(timer_id & 0xFF);
  WICED_BT_TRACE( "tile_timer_expired: %d\r\n", timer_id);
}


/**
 * @brief Retrieve correct 16-bit UUID to advertise, depending on the Tile mode
 */
uint16_t tile_get_adv_uuid(void)
{
  if(TILE_MODE_ACTIVATED == tile_checked->mode)
  {
    return TILE_ACTIVATED_UUID;
    WICED_BT_TRACE( "tile return tile UUID ACTIVATED\r\n");
  }
  else
  {
    return TILE_SHIPPING_UUID;
    WICED_BT_TRACE( "tile return tile UUID SHIPPING\r\n");
  }
}
