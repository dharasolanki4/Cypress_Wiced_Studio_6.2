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


#include "tile_service.h"

#include "tile_config.h"
#include "tile_lib.h"
#include "tile_gatt_db.h"
#include "tile_player.h"
#include "tile_storage.h"
#include "drivers/tile_gap_driver.h"
#include "drivers/tile_gatt_server_driver.h"
#include "drivers/tile_random_driver.h"
#include "drivers/tile_timer_driver.h"
#include "modules/tile_toa_module.h"
#include "modules/tile_tmd_module.h"
#include "modules/tile_tdi_module.h"
#include "modules/tile_tmf_module.h"

#if TILE_ENABLE_BUTTON
#include "drivers/tile_button_driver.h"
#include "modules/tile_tdt_module.h"
#endif

#if TILE_ENABLE_PLAYER
#include "modules/tile_song_module.h"
#endif


#include "ble.h"
#include "ble_hci.h"
#include "nrf_drv_rng.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "boards.h"
#include "app_button.h"

#include <stdbool.h>

/************ TODO: Add TDG w/ basic diagnostics, maybe ***************/



/*******************************************************************************
 * Forward declarations
 ******************************************************************************/

static int tile_disconnect(void);
static int set_characteristic_value(uint8_t attr_id, uint8_t *data, uint16_t len);
static int notify_characteristic(uint8_t attr_id, uint8_t *data, uint16_t len);
static int random_bytes(uint8_t *dst, uint8_t len);
static void tile_timer_timeout_handler(void *p_context);
static int tile_timer_start(uint8_t timer_id, uint32_t duration);
static int tile_timer_cancel(uint8_t timer_id);
static void event_handler(void * data, uint16_t size);
static int mode_set(uint8_t mode);
static int mode_get(uint8_t *mode);
#if TILE_ENABLE_BUTTON
static int read_button(uint8_t *state);
static int tdt_config_written(tdt_config_t *config);
static void button_handler(uint8_t button_number, uint8_t button_action);
#endif
#if TILE_USE_MANUFACTURING_MODE
static int tile_tmf_ship(void);
#endif



/*******************************************************************************
 * Defines & types
 ******************************************************************************/

 /* Must be the same as APP_TIMER_PRESCALER in main.c!!! */
#define APP_TIMER_PRESCALER 0

enum CUSTOM_EVENTS
{
  NOTIFICATION_WRITTEN_EVT
};

struct my_evt
{
  uint8_t type;
};

/*******************************************************************************
 * Global variables
 ******************************************************************************/
tile_ble_env_t tile_ble_env;



/*******************************************************************************
 * Local variables
 ******************************************************************************/
static ble_gap_addr_t bdaddr;

static uint8_t tx_slots;
static bool confirm_tx = false;

app_timer_t    tile_timer_data[TILE_MAX_TIMERS] = { 0 };
app_timer_id_t tile_timer_id[TILE_MAX_TIMERS];


#if TILE_ENABLE_BUTTON
static app_button_cfg_t button_cfg = {
  .pin_no         = BUTTON_4,
  .active_state   = BUTTONS_ACTIVE_STATE,
  .pull_cfg       = BUTTON_PULL,
  .button_handler = button_handler,
};
#endif




/*******************************************************************************
 * Tile configuration structures
 ******************************************************************************/

/* GAP configuration */
static struct tile_gap_driver gap_driver = {
  .gap_disconnect = tile_disconnect
};

/* GATT configuration. Additional values set in tile_service_init() */
static struct tile_gatt_server_driver gatt_driver = {
  .set_attribute = set_characteristic_value,
  .notify_attribute = notify_characteristic
};

/* Random number generation configuration */
static struct tile_random_driver random_driver = {
  .random_bytes = random_bytes
};

/* Channel configurations used by Tile Over-the-air API */
static toa_channel_t toa_channels[NUM_TOA_CHANNELS];

/* Buffer used for TOA message queue */
static uint8_t toa_queue_buffer[TOA_QUEUE_BUFFER_SIZE];

/* Tile Over-the-air API configuration */
struct tile_toa_module toa_module = {
  .channels          = toa_channels,
  .num_channels      = NUM_TOA_CHANNELS,
  .queue             = toa_queue_buffer,
  .queue_size        = TOA_QUEUE_BUFFER_SIZE,
};

/* Timer configuration */
static struct tile_timer_driver timer_driver = {
  .start  = tile_timer_start,
  .cancel = tile_timer_cancel
};

/* Mode getting/setting configuration */
static struct tile_tmd_module tmd_module = {
  .set = mode_set,
  .get = mode_get
};

#if TILE_ENABLE_BUTTON
/* Button driver configuration */
static struct tile_button_driver button_driver = {
  .read_state = read_button,
};

/* Tile Double Tap module configuration */
static struct tile_tdt_module tdt_module = {
  .config_written = tdt_config_written,
};
#endif

#if TILE_ENABLE_PLAYER
/* Tile Song configuration */
static struct tile_song_module song_module = {
  .play   = PlaySong,
  .stop   = StopSong
};
#endif

/* Tile Device Information module configuration */
static struct tile_tdi_module tdi_module;

#if TILE_USE_MANUFACTURING_MODE
static struct tile_tmf_module tmf_module;
#endif


/*******************************************************************************
 * Functions
 ******************************************************************************/

/**
 * @brief Initialize Tile BLE service
 */
void tile_service_init(void)
{
  /* Initialize Tile data storage for Nordic */
  tile_storage_init();

  sd_ble_gap_addr_get(&bdaddr);

  /* Initialize random number generator */
  nrf_drv_rng_init(NULL);

  /* Initialize Tile timers */
  for(int i=0; i < TILE_MAX_TIMERS; i++)
  {
    tile_timer_id[i] = &tile_timer_data[i];
    /* We use one universal timeout handler with p_context containing the timer ID for dispatch to the shim */
    app_timer_create(&tile_timer_id[i], APP_TIMER_MODE_SINGLE_SHOT, tile_timer_timeout_handler);
  }

  /* Initialize Tile GATT service */
  tile_gatt_db_init(&tile_ble_env.service);

  /* Configure each Tile module */
  tile_gap_register(&gap_driver);

  gatt_driver.tile_id                    = tile_data.tile_id,
  gatt_driver.auth_key                   = tile_data.auth_key,
  gatt_driver.set_attribute              = set_characteristic_value,
  gatt_driver.notify_attribute           = notify_characteristic,
  tile_gatt_server_register(&gatt_driver);

  tile_random_register(&random_driver);
  tile_timer_register(&timer_driver);
  tile_toa_register(&toa_module);

  tdi_module.tile_id            = tile_data.tile_id,
  tdi_module.bdaddr             = bdaddr.addr,
  tdi_module.firmware_version   = TILE_FIRMWARE_VERSION,
  tdi_module.model_number       = tile_data.model_number,
  tdi_module.hardware_version   = tile_data.hardware_version,
  tile_tdi_register(&tdi_module);

#if TILE_ENABLE_PLAYER
  InitPlayer();
  tile_song_register(&song_module);
#endif

#if TILE_USE_MANUFACTURING_MODE
  if(TILE_MODE_MANUFACTURING == tile_data.mode)
  {
    tmf_module.ship               = tile_tmf_ship,
    tmf_module.tile_id            = tile_data.tile_id,
    tmf_module.tile_auth_key      = tile_data.auth_key,
    tmf_module.bdaddr             = bdaddr.addr,
    tmf_module.model_number       = tile_data.model_number,
    tmf_module.hardware_version   = tile_data.hardware_version,
    tile_tmf_register(&tmf_module);
  }
#endif

  tile_tmd_register(&tmd_module);

#if TILE_ENABLE_BUTTON
  app_button_init(&button_cfg, 1, 20);
  app_button_enable();
  tile_button_register(&button_driver);

  memcpy(&tdt_module.config, &tile_data.tdt_configuration, sizeof(tdt_config_t));
  tile_tdt_register(&tdt_module);
#endif
}


/**
 * @brief Handle Tile BLE events
 *
 * @param[in] p_evt    Event forwarded from BLE stack.
 */
void tile_on_ble_evt(ble_evt_t *p_evt)
{
  struct tile_conn_params params;
  uint16_t handle;

  switch (p_evt->header.evt_id)
  {
    case BLE_GAP_EVT_DISCONNECTED:
      tile_gap_disconnected();
      break; // BLE_GAP_EVT_DISCONNECTED

    case BLE_GAP_EVT_CONNECTED:
      /* Save connection handle */
      tile_ble_env.conn_handle = p_evt->evt.gap_evt.conn_handle;

      /* Track number of packets that can be buffered */
      sd_ble_tx_packet_count_get(tile_ble_env.conn_handle, &tx_slots);

      /* Tell Tile Lib about the connection */
      params.conn_interval = p_evt->evt.gap_evt.params.connected.conn_params.max_conn_interval;
      params.slave_latency = p_evt->evt.gap_evt.params.connected.conn_params.slave_latency;
      params.conn_sup_timeout = p_evt->evt.gap_evt.params.connected.conn_params.conn_sup_timeout;
      tile_gap_connected(&params);
      break; // BLE_GAP_EVT_CONNECTED

    case BLE_GAP_EVT_CONN_PARAM_UPDATE:
      params = (struct tile_conn_params){
        .conn_interval    = p_evt->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval,
        .slave_latency    = p_evt->evt.gap_evt.params.conn_param_update.conn_params.slave_latency,
        .conn_sup_timeout = p_evt->evt.gap_evt.params.conn_param_update.conn_params.conn_sup_timeout,
      };
      tile_gap_params_updated(&params);
      break;

    case BLE_GATTS_EVT_WRITE:
      /* Find which ID is associated with the handle */
      handle = p_evt->evt.gatts_evt.params.write.handle;

      for(int i = 0; i < TILE_NUM_ATTRS; i++)
      {
        if(handle == tile_ble_env.service.characteristic_handles[i])
        {
          /* Tell Tile Lib about the write */
          tile_gatt_server_write(i, p_evt->evt.gatts_evt.params.write.data,
            p_evt->evt.gatts_evt.params.write.len);
          break; /* Break from the loop */
        }
      }
      break;

    case BLE_EVT_TX_COMPLETE:
      tx_slots += p_evt->evt.common_evt.params.tx_complete.count;
      if(confirm_tx)
      {
        confirm_tx = false;
        /* If we were waiting for the queue to clear before confirming, confirm now */
        tile_gatt_server_write(TILE_TOA_RSP_CHAR, NULL, 0);
      }
      break;

    default:
      break;
  }
}


/**
 * @brief Retrieve correct 16-bit UUID to advertise, depending on the Tile mode
 */
uint16_t tile_get_adv_uuid(void)
{
  if(TILE_MODE_ACTIVATED == tile_data.mode)
  {
    return TILE_ACTIVATED_UUID;
  }
  else
  {
    return TILE_SHIPPING_UUID;
  }
}


/*******************************************************************************
 * Local functions
 ******************************************************************************/

#if TILE_USE_MANUFACTURING_MODE
/**
 * @brief Handler for TMF_SHIP Command (Manufacturing Command to Ship).
 */
static int tile_tmf_ship(void)
{
  tile_data.mode = TILE_MODE_SHIPPING;
  tile_store_app_data();

#if TILE_ENABLE_PLAYER
  PlaySong(TILE_SONG_5_BIP, 2);
#endif
  tile_tmf_unregister();
  return TILE_SUCCESS;
}
#endif


/**
 * @brief Timer handler for Tile timers
 */
static void tile_timer_timeout_handler(void *p_context)
{
  tile_timer_expired((uint32_t)p_context & 0xFF);
}


/**
 * @brief Handler for custom events.
 */
static void event_handler(void * data, uint16_t size)
{
  struct my_evt *evt = data;

  switch(evt->type)
  {
    case NOTIFICATION_WRITTEN_EVT:
      /*
       * If there's still queue space available, confirm notification write.
       * Otherwise, do not confirm until after queued packets have been shipped off.
       */
      tx_slots--;
      if(tx_slots > 0)
      {
        tile_gatt_server_write(TILE_TOA_RSP_CHAR, NULL, 0);
      }
      else
      {
        confirm_tx = true;
      }
      break;
  }
}


#if TILE_ENABLE_BUTTON
/**
 * @brief Handler for button events.
 */
static void button_handler(uint8_t button_number, uint8_t button_action)
{
  if(APP_BUTTON_PUSH == button_action)
  {
    if(TILE_MODE_ACTIVATED == tile_data.mode)
    {
      tile_button_pressed();
    }
    else
    {
#if TILE_ENABLE_PLAYER
      PlaySong(TILE_SONG_WAKEUP_PART, 2);
#endif
    }
  }
}
#endif



/*******************************************************************************
 * Callback functions for Tile Lib
 ******************************************************************************/

/**
 * @brief Disconnect current connection
 */
static int tile_disconnect(void)
{
  if(BLE_CONN_HANDLE_INVALID == tile_ble_env.conn_handle)
  {
    return TILE_ERROR_ILLEGAL_OPERATION;
  }

  sd_ble_gap_disconnect(tile_ble_env.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  return TILE_SUCCESS;
}


/**
 * @brief Set characteristic to specified value
 *
 * @param[in] attr_id    ID for attribute to set.
 * @param[in] data       Data to set attribute to.
 * @param[in] len        Length of data.
 */
static int set_characteristic_value(uint8_t attr_id, uint8_t *data, uint16_t len)
{
  uint16_t handle = tile_ble_env.service.characteristic_handles[attr_id];

  ble_gatts_value_t newval = { len, 0, data };
  sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, handle, &newval);

  return TILE_SUCCESS;
}


/**
 * @brief Send notification on a characteristic
 *
 * @param[in] attr_id    ID for attribute to notify.
 * @param[in] data       Data to set attribute to.
 * @param[in] len        Length of data.
 */
static int notify_characteristic(uint8_t attr_id, uint8_t *data, uint16_t len)
{
  if(BLE_CONN_HANDLE_INVALID == tile_ble_env.conn_handle)
  {
    return TILE_ERROR_ILLEGAL_OPERATION;
  }

  if(0 == tx_slots)
  {
    /* Should never happen */
    return TILE_ERROR_BUFFER_TOO_SMALL;
  }

  uint16_t handle = tile_ble_env.service.characteristic_handles[attr_id];

  ble_gatts_hvx_params_t hvx_params = {
    .handle   = handle,
    .type     = BLE_GATT_HVX_NOTIFICATION,
    .offset   = 0,
    .p_len    = &len,
    .p_data   = data
  };

  sd_ble_gatts_hvx(tile_ble_env.conn_handle, &hvx_params);

  /* Schedule event to see if more messages can be queued */
  struct my_evt evt = {
    .type = NOTIFICATION_WRITTEN_EVT
  };
  app_sched_event_put(&evt, sizeof(evt), event_handler);

  return TILE_SUCCESS;
}


/**
 * @brief Generate some random bytes
 *
 * @param[out] dst    Destination address for the random bytes
 * @param[in]  len    Number of bytes requested
 */
static int random_bytes(uint8_t *dst, uint8_t len)
{
  uint8_t num;

  /* Check if enough random bytes are available */
  nrf_drv_rng_bytes_available(&num);
  while(num < len)
  {
    /* Wait for enough random bytes to be available */
    nrf_delay_us(200);
    nrf_drv_rng_bytes_available(&num);
  }

  /* Copy over random bytes */
  nrf_drv_rng_rand(dst, len);

  return TILE_SUCCESS;
}


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
  app_timer_stop(tile_timer_id[timer_id]);
  app_timer_start(tile_timer_id[timer_id], APP_TIMER_TICKS(duration*10, APP_TIMER_PRESCALER), (void *)timer_id);

  return TILE_SUCCESS;
}


/**
 * @brief Cancel a Tile timer
 *
 * @param[in] timer_id    ID for timer, as specified by Tile Lib
 */
static int tile_timer_cancel(uint8_t timer_id)
{
  app_timer_stop(tile_timer_id[timer_id]);

  return TILE_SUCCESS;
}


/**
 * @brief Set the mode of the device.
 *
 * @param[in] mode  Mode, as specified by the TMD module.
 */
static int mode_set(uint8_t mode)
{
  tile_data.mode = mode;
  tile_store_app_data();

  return TILE_SUCCESS;
}


/**
 * @brief Get the current mode of the device.
 *
 * @param[out] mode  Mode, as specified by the TMD module.
 */
static int mode_get(uint8_t *mode)
{
  *mode = tile_data.mode;

  return TILE_SUCCESS;
}


#if TILE_ENABLE_BUTTON
/**
 * @brief Read the current state of the Tile button.
 *
 * @param[out] state  The current state of the button, as specified in the Tile button module.
 */
static int read_button(uint8_t *state)
{
  *state = app_button_is_pushed(0) ? TILE_BUTTON_PRESSED : TILE_BUTTON_RELEASED;

  return TILE_SUCCESS;
}


/**
 * @brief Value for TDT configuration written by app.
 *
 * This function should persist the configuration, if possible.
 *
 * @param[in] config  TDT configuration that was written.
 */
static int tdt_config_written(tdt_config_t *config)
{
  memcpy(&tile_data.tdt_configuration, config, sizeof(tdt_config_t));
  tile_store_app_data();

  return TILE_SUCCESS;
}
#endif


