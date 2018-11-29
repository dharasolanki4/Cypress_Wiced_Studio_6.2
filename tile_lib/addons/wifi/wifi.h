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

/** @file wifi.h
 ** @brief Wifi module header
 */


#ifndef PERIPHERALS_WIFI_WIFI_H_
#define PERIPHERALS_WIFI_WIFI_H_

#include <stdint.h>

typedef enum
{
  WIFI_OP_STATUS_SUCCESS, ///> The operation has successfully finished.
  WIFI_OP_STATUS_PENDING, ///> The operation is not finished yet.
  WIFI_OP_STATUS_FAILURE, ///> The operation failed.
} wifi_op_status_t;

// TODO: This structure is exactly taken from Microchip API so it can conveniently be memcopied.
// We should optimize this struct later based on our needs.
typedef struct
{
  uint8_t   index;
  /*!< AP index in the scan result list.
  */
  int8_t   rssi;
  /*!< AP signal strength.
  */
  uint8_t   AuthType;
  /*!< AP authentication type.
  */
  uint8_t   ch;
  /*!< AP RF channel.
  */
  uint8_t BSSID[6];
  /*!< BSSID of the AP.
  */
  uint8_t   SSID[33]; // FIXME: same as M2M_MAX_SSID_LEN
  /*!< AP ssid.
  */
  uint8_t   _PAD8_;
  /*!< Padding bytes for forcing 4-byte alignment
  */
}wifi_scan_result_t;


/**
* @brief callback function to return the result of a Scan
 *
 * @param[in]  op_status the callback will be called with status PENDING to deliver an AP info and then SUCCESS when finished.
 * @param[in]  number_aps number of APs discovered (may be bigger than the number of APs requested).
 * @param[in]  res pointer to AP infos (is NULL in case of failure or no data).
 */
typedef void (*wifi_scan_callback_t)(wifi_op_status_t op_status, uint8_t number_aps, wifi_scan_result_t* res);


/******************************************************************************
 * Wifi Module API functions
 *****************************************************************************/
void wifi_init(void);
void wifi_deinit(void);
void wifi_scan(wifi_scan_callback_t callback, uint8_t max_ap_num, wifi_scan_result_t* res);


#endif /* PERIPHERALS_WIFI_WIFI_H_ */

