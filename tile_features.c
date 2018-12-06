/*
 * NOTICE
 *
 * © 2016 Tile Inc.  All Rights Reserved.
 *
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
 * @file tile_features.h
 * @brief Support for features in Tile Lib
 */

// TileLib includes
#include "tile_lib/src/toa/toa.h"
#include "tile_config.h"
#include "tile_features.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_timer_driver.h"
#include "drivers/tile_random_driver.h"
#include "modules/tile_tdi_module.h"
#include "modules/tile_toa_module.h"
#include "modules/tile_tmd_module.h"
#include "modules/tile_tdg_module.h"
#include "modules/tile_song_module.h"
#include "tile_storage.h"
//#include "tile_assert/tile_assert.h"
#include "tile_version.h"

#include "hello_sensor.h"
#include "wiced_hal_rand.h"

#include "string.h"
#include "wiced_bt_trace.h"

/*******************************************************************************
 * Global variables
 ******************************************************************************/
extern hello_sensor_state_t hello_sensor_state;

tile_ble_env_t tile_ble_env;

wiced_timer_t tile_timer[TILE_MAX_TIMERS];

const uint8_t interim_tile_id[8]    = INTERIM_TILE_ID;
const uint8_t interim_tile_key[16]  = INTERIM_AUTH_KEY;

const char tile_model_number[]      = TILE_MODEL_NUMBER;
const char tile_hw_version[]        = TILE_HARDWARE_VERSION;

/*******************************************************************************
 * Local variables
 ******************************************************************************/
static toa_channel_t tile_toa_channels[NUM_TOA_CHANNELS]; //__attribute__((section("retention_mem_area0"), zero_init));
static uint8_t toa_queue_buffer[TOA_QUEUE_BUFFER_SIZE];

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
/* timer timeout callback function */
extern void tile_timer_timeout_handler(uint32_t arg);

/* send Toa Response */
extern void tile_send_message( uint8_t *data, uint16_t datalen );

/* send notification */
extern void hello_sensor_send_message( void );

/* gap module*/
static int tile_disconnect(void);

/* timer module*/
static int tile_timer_start(uint8_t timer_id, uint32_t duration);
static int tile_timer_cancel(uint8_t timer_id);

/* random module*/
static int tile_random_bytes(uint8_t *dst, uint8_t len);

/* toa module*/
static int tile_send_toa_response(uint8_t *data, uint16_t len);
static int tile_associate(uint8_t* aco, uint8_t* authorization_type);

/* tmd module*/
static int tile_mode_set(uint8_t mode);
static int tile_mode_get(uint8_t *mode);

/* tdg module*/
static int tile_get_diagnostics_cb(void);

/* song module*/
static int PlaySong(uint8_t number, uint8_t strength);
static int StopSong(void);

/* test module*/
static void test_process_reboot(uint8_t reboot_type);
static void test_process_storage(uint8_t test_type, uint8_t *payload, uint8_t payload_length);
static int test_process(uint8_t code, uint8_t *data, uint8_t datalen);

/*******************************************************************************
 * Defines & types
 ******************************************************************************/

/*******************************************************************************
 * Tile configuration structures
 ******************************************************************************/

/* gap register struct */
static struct tile_gap_driver gap_driver = 
{
  .gap_disconnect         = tile_disconnect,
  .auth_disconnect_count  = &tile_persist.unchecked.s.auth_disconnect_count,
};

/* timer driver struct */
struct tile_timer_driver timer_driver =
{
  .start  = tile_timer_start,
  .cancel = tile_timer_cancel,
};

/* random driver struct  */
static struct tile_random_driver random_driver =
{
  .random_bytes  = tile_random_bytes,
};

/* device information struct */
struct tile_tdi_module tdi_module =
{
  .tile_id                  = tile_persist.checked.s.tile_id,
  .model_number             = tile_persist.checked.s.model_number,
  .hardware_version         = tile_persist.checked.s.hardware_version,
  .bdaddr                   = tile_persist.checked.s.bdaddr,
  .firmware_version         = TILE_FIRMWARE_VERSION,
};

/* tile over the air struct  */
struct tile_toa_module toa_module =
{
  .tile_id                  = tile_persist.checked.s.tile_id,
  .auth_key                 = tile_persist.checked.s.tile_auth_key,
  .channels                 = tile_toa_channels,
  .queue                    = toa_queue_buffer,
  .queue_size               = TOA_QUEUE_BUFFER_SIZE,
  .num_channels             = NUM_TOA_CHANNELS,
  .mic_failure_count        = &tile_persist.unchecked.s.micFailures,
  .auth_failure_count       = &tile_persist.unchecked.s.auth_fail_count,
  .channel_open_count       = &tile_persist.unchecked.s.toa_channel_open_count,
  .authenticate_count       = &tile_persist.unchecked.s.toa_authenticate_count,
  .tka_closed_channel_count = &tile_persist.unchecked.s.tka_closed_channel_count,
  .send_response            = tile_send_toa_response,
  .associate                = tile_associate
};

/* tile mode struct */
struct tile_tmd_module tmd_module =
{
  .get  = tile_mode_get,
  .set  = tile_mode_set,
};

static struct tile_tdg_module tdg_module = {
  .get_diagnostics = tile_get_diagnostics_cb,
};

/* tile song module struct */
static struct tile_song_module song_module = 
{
  .play = PlaySong,
  .stop = StopSong
};

/* tile test module struct */
static struct tile_test_module test_module = {
  .process = test_process,
};

/*******************************************************************************
 * Functions
 ******************************************************************************/
void tile_features_init(void)
{
  /****************************************************************/
  /**** Minimum Features required for TileLib Interoperability ****/
  /****************************************************************/
  /* Initialize GAP driver */ 
  tile_gap_register(&gap_driver);
  
  /* Initialize timer driver */
  tile_timer_register(&timer_driver);
	
  /* Initialize random driver */ 
  tile_random_register(&random_driver);

  tdi_module.tile_id                  = tile_persist.checked.s.tile_id;
  tdi_module.model_number             = (char*)tile_model_number;
  tdi_module.hardware_version         = (char*)tile_hw_version;
  tdi_module.bdaddr                   = tile_persist.checked.s.bdaddr;
  tdi_module.firmware_version         = TILE_FIRMWARE_VERSION;
  /* Initialize device information module */
  tile_tdi_register(&tdi_module);
	
  /* Initialize tile over the air module */
  tile_toa_register(&toa_module);
	
  /* Initialize tile mode module */
  tile_tmd_register(&tmd_module);

	// trm// add later
	
  /****************************************************************/
  /**** Additional Features ****/
  /****************************************************************/
  /* Initialize tile diagnbostics module */
  tile_tdg_register(&tdg_module);

  /* Initialize song module */
  tile_song_register(&song_module);// add later

  /* Initialize test module */
  tile_test_register(&test_module);

  WICED_BT_TRACE( "tile features init DONE Successfully\r\n");
}



/*******************************************************************************
 * Callback functions for Tile Lib
 ******************************************************************************/

/***************************** gap module *******************************/
/**
 * @brief Disconnect current connection
 */
static int tile_disconnect(void)
{
  if(tile_ble_env.tile_gatt_connection_status == 0) // Already disconnected
  {
    return TILE_ERROR_ILLEGAL_OPERATION;
  }

  /* Initiating the gatt disconnect */
  wiced_bt_gatt_disconnect( hello_sensor_state.conn_id );

  return TILE_SUCCESS;
}
/************************************************************************/

/***************************** timer module *****************************/
/**
 * @brief Start a Tile timer
 *
 * @param[in] timer_id   ID for timer, as specified by Tile Lib
 * @param[in] duration   Duration (in 10ms increments) for the timer
 */
static int tile_timer_start(uint8_t timer_id, uint32_t duration)
{
  if(duration < 1)
  {
    duration++;
  }

  /* The new timer takes priority, so stop any existing timer */
  wiced_stop_timer(&(tile_timer[timer_id]));

  if ( wiced_start_timer( &(tile_timer[timer_id]), duration*10 ) == WICED_SUCCESS )
  {
    return TILE_SUCCESS;
  }
  else
  {
    return TILE_ERROR_NOT_INITIALIZED;
  }
}

/**
 * @brief Cancel a Tile timer
 *
 * @param[in] timer_id    ID for timer, as specified by Tile Lib
 */
static int tile_timer_cancel(uint8_t timer_id)
{
  if (wiced_stop_timer(&(tile_timer[timer_id])) == WICED_SUCCESS)
  {
    return TILE_SUCCESS;
  }
  else
  {
    return TILE_ERROR_NOT_INITIALIZED;
  }
}
/************************************************************************/

/****************************random module *******************************/
/**
 * @brief Generate some random bytes
 *
 * @param[out] dst    Destination address for the random bytes
 * @param[in]  len    Number of bytes requested
 */
static int tile_random_bytes(uint8_t *dst, uint8_t len)
{
  uint8_t  num = 0;// number of random bytes obtained
  uint32_t rand;

  while(num < len)
  {
    if ((num%4) == 0)
    {
      rand = wiced_hal_rand_gen_num();
    }
    *dst++ = rand & 0xFF;
    rand   = rand >> 8;
    num++;
  }

  return TILE_SUCCESS;
}
/************************************************************************/

/***************************** toa module *******************************/

/**
 * @brief Send notification on a characteristic in TOA_RSP
 * 
 * @param[in] data       Data to set attribute to.
 * @param[in] len        Length of data.
 */
static int tile_send_toa_response(uint8_t *data, uint16_t len)
{
  uint8_t *p_data  = data;
  uint16_t datalen = len;
  WICED_BT_TRACE("Tile: tile_send_toa_response \r\n");
  WICED_BT_TRACE("TOA_RSP: ");
  while (len--)
  {
    WICED_BT_TRACE("%x ",*p_data++);
  }
  WICED_BT_TRACE("\r\n");
  //wiced_bt_gatt_send_indication( hello_sensor_state.conn_id, HANDLE_HSENS_TILE_SERVICE_CHAR_MEP_TOA_RSP_VAL, len, data );
  tile_send_message(data, datalen);
  return TILE_SUCCESS;
};

static int tile_associate(uint8_t* aco, uint8_t* authorization_type)
{
  int retcode = TOA_ERROR_OK;
  if(TILE_MODE_ACTIVATED != tile_checked->mode)
  {	
      memcpy(tile_checked->tile_id, aco,  sizeof(tile_checked->tile_id));
      memcpy(tile_checked->tile_auth_key, aco+8, sizeof(tile_checked->tile_auth_key));

      WICED_BT_TRACE("tile_checked->tile_id[0] : %0x\r\n",tile_checked->tile_id[0]);
      WICED_BT_TRACE("tile_checked->tile_id[1] : %0x\r\n",tile_checked->tile_id[1]);
      WICED_BT_TRACE("tile_checked->tile_id[2] : %0x\r\n",tile_checked->tile_id[2]);
      WICED_BT_TRACE("tile_checked->tile_id[3] : %0x\r\n",tile_checked->tile_id[3]);

      WICED_BT_TRACE("tile_checked->tile_auth_key[0] : %0x\r\n",tile_checked->tile_auth_key[0]);
      WICED_BT_TRACE("tile_checked->tile_auth_key[1] : %0x\r\n",tile_checked->tile_auth_key[1]);
      WICED_BT_TRACE("tile_checked->tile_auth_key[2] : %0x\r\n",tile_checked->tile_auth_key[2]);
      WICED_BT_TRACE("tile_checked->tile_auth_key[3] : %0x\r\n",tile_checked->tile_auth_key[3]);
  }
  else
  {
    retcode = TOA_RSP_SERVICE_UNAVAILABLE;
  }
  return retcode;
}
/************************************************************************/

/***************************** mode module *******************************/
/**
 * @brief Set the mode of the device.
 *
 * @param[in] mode  Mode, as specified by the TMD module.
 */
static int tile_mode_set(uint8_t mode)
{
  if(TILE_MODE_ACTIVATED != mode)
  {
    /* Disregard any mode besides Shipping and Activated
     * If mode being set is not Activated, Make it Shipping
     */
    mode = TILE_MODE_SHIPPING;
    /* When the Tile is not activated, the Interim TileID, Key is used. */
    memcpy(tile_checked->tile_id, interim_tile_id, 8);
    memcpy(tile_checked->tile_auth_key, interim_tile_key, 16);
  }
  tile_checked->mode = mode;
  //tile_store_app_data(); // Without this, we cannot power cycle and expect tile to work
  return TILE_SUCCESS;
}

/**
 * @brief Get the current mode of the device.
 *
 * @param[out] mode  Mode, as specified by the TMD module.
 */
static int tile_mode_get(uint8_t *mode)
{
  *mode = tile_checked->mode;

  return TILE_SUCCESS;
}
/************************************************************************/

/***************************** tdg module *******************************/
static int tile_get_diagnostics_cb(void)
{
  uint8_t version = DIAGNOSTIC_VERSION;
  
  tdg_add_data(&version, 1);
  tdg_add_data(&tile_checked->mode, 1);
  tdg_add_data(&tile_unchecked->reset_count, 1);
  tdg_add_data(&tile_unchecked->connection_count, 3);
  tdg_add_data(&tile_unchecked->auth_fail_count, 1);
  tdg_add_data(&tile_unchecked->micFailures, 1);
  tdg_add_data(&tile_unchecked->disconnect_count, 3);
  tdg_add_data(&tile_unchecked->toa_channel_open_count, 3);
  tdg_add_data(&tile_unchecked->toa_authenticate_count, 3);
  tdg_add_data(&tile_unchecked->tka_closed_channel_count, 2);
  tdg_add_data(&tile_unchecked->auth_disconnect_count, 2);

  tdg_finish();

  return TILE_SUCCESS;
}
/************************************************************************/

/***************************** song module ******************************/

int PlaySong(uint8_t number, uint8_t strength)
{
  return TILE_SUCCESS;
}

int StopSong(void)
{
	return TILE_SUCCESS;
}
/************************************************************************/

/***************************** test module ******************************/

static int test_process(uint8_t code, uint8_t *data, uint8_t datalen)
{
  switch(code) {
    case TEST_CMD_REBOOT:
      test_process_reboot(data[0]);
      break;
    case TEST_CMD_STORAGE:
      test_process_storage(data[0], data+1, datalen-1);
      break;
    default:
      break;
  }

  return TILE_SUCCESS;
}

static void test_process_reboot(uint8_t reboot_type)
{
  switch(reboot_type) {
    case TEST_CMD_REBOOT_RESET:
      // SwReset Function Call
      break;
    case TEST_CMD_REBOOT_WATCHDOG:
      while(1);
      break;
    case TEST_CMD_REBOOT_MEMORY_FAULT:
      *((uint8_t*)0xFFFFFFFF) = 0;
      break;
    case TEST_CMD_REBOOT_OTHER:
      /* ? */
      break;
    case TEST_CMD_REBOOT_ASSERT:
      //TILE_ASSERT(0);
      ASSERT_FATAL(0);
      /* TODO */
      break;
    case TEST_CMD_REBOOT_DURING_FLASH:
      /* TODO */
      break;
  }
}

static void test_process_storage(uint8_t test_type, uint8_t *payload, uint8_t payload_length)
{
  /* TODO */
}
/************************************************************************/

