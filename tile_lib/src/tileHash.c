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

 /** @file tileHash.c
 ** @brief Tile Security Functions
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tilehash.h"
#include "crypto/uECC.h"
#include "toa/toa.h"
#include "wiced_bt_trace.h"

/** \defgroup TILE_SECURITY

This section describes the Security Functions used in a Tile.

*  @{
*/


#if 0
  /* Activate signal debugging. For DevBoard only */
  #warning "Debug Signals enabled"
  #define SHA2_TYPES
  #include "gpio.h"
  #include "periph_setup.h"
  #define DBG_DEV_AUTH_ENTER  GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST0_PIN, OUTPUT, PID_GPIO, true);
  #define DBG_DEV_AUTH_EXIT   GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST0_PIN, OUTPUT, PID_GPIO, false);
  #define DBG_KEY_GEN_ENTER   GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST1_PIN, OUTPUT, PID_GPIO, true);
  #define DBG_KEY_GEN_EXIT    GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST1_PIN, OUTPUT, PID_GPIO, false);
  #define DBG_MSG_AUTH_ENTER  GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST2_PIN, OUTPUT, PID_GPIO, true);
  #define DBG_MSG_AUTH_EXIT   GPIO_ConfigurePin(GPIO_TEST_PORT, GPIO_TEST2_PIN, OUTPUT, PID_GPIO, false);
#elif 0
  /* Activate signal debugging. For Tile HW only */
  #warning "Debug Signals enabled"
  #define SHA2_TYPES
  #include "gpio.h"
  #include "periph_setup.h"
  #define DBG_DEV_AUTH_ENTER
  #define DBG_DEV_AUTH_EXIT
  #define DBG_KEY_GEN_ENTER   GPIO_ConfigurePin(GPIO_TILE_TEST_PORT, GPIO_TILE_TEST0_PIN, OUTPUT, PID_GPIO, true);
  #define DBG_KEY_GEN_EXIT    GPIO_ConfigurePin(GPIO_TILE_TEST_PORT, GPIO_TILE_TEST0_PIN, OUTPUT, PID_GPIO, false);
  #define DBG_MSG_AUTH_ENTER  GPIO_ConfigurePin(GPIO_TILE_TEST_PORT, GPIO_TILE_TEST1_PIN, OUTPUT, PID_GPIO, true);
  #define DBG_MSG_AUTH_EXIT   GPIO_ConfigurePin(GPIO_TILE_TEST_PORT, GPIO_TILE_TEST1_PIN, OUTPUT, PID_GPIO, false);
#else
  #define DBG_DEV_AUTH_ENTER
  #define DBG_DEV_AUTH_EXIT
  #define DBG_KEY_GEN_ENTER
  #define DBG_KEY_GEN_EXIT
  #define DBG_MSG_AUTH_ENTER
  #define DBG_MSG_AUTH_EXIT
#endif

#ifdef HMAC256_DOTTE
	#include "sha256_dotte.h"
	#include "hmac-sha256_dotte.h"
#else
	#include "crypto/sha256.h"
	#include "crypto/hmac_sha256.h"
#endif



/**@brief Tile authentication function. <br>
 * input: auth_key: 16 Bytes <br>
 * input: randA: 14 Bytes <br>
 * input: randT: 10 Bytes <br>
 * output: sresT: 4 bytes <br> (bytes 4:7 of hash)
 */
void tile_device_auth_hash(const uint8_t* auth_key, const uint8_t* randA, const uint8_t* randT, uint8_t* sresT, uint8_t* aco)
{
	uint8_t mac[32];
	uint8_t msg[32];

  DBG_DEV_AUTH_ENTER

	/* prepare the input message. Pad zeros at ends to make randA and randT 32 bytes */
  memset(msg, 0, 32);
	memcpy(msg, randA, 14);
	memcpy(msg+16, randT, 10);

	/* process */
#ifdef HMAC256_DOTTE
	hmac_sha256(mac, auth_key, 128, msg, 256); /* DOtte source */
#else
	hmac_sha256_ogay(auth_key, 16, msg, 32, mac, 32); /* OGay source */
#endif
    WICED_BT_TRACE("\r\n\r\n\r\nOGAY SHA256 TILELIB \r\n");
    WICED_BT_TRACE("auth_key :");
    for (int i=0;i<16;i++)
    {
      WICED_BT_TRACE("%02x",auth_key[i]);
    }
    WICED_BT_TRACE("\r\n");
    WICED_BT_TRACE("msg :");
    for (int i=0;i<32;i++)
    {
      WICED_BT_TRACE(" %02x",msg[i]);
    }
    WICED_BT_TRACE("\r\n");
    WICED_BT_TRACE("mac :");

    for (int i=0;i<32;i++)
    {
      WICED_BT_TRACE("%02x",mac[i]);
    }
    WICED_BT_TRACE("\r\n");

	memcpy(sresT, mac+4, 4);
  memcpy(aco, mac+8, 24);

  DBG_DEV_AUTH_EXIT

	return;
}

/**
 * @brief Tile session key generation function. <br>
 * input: auth_key: 16 Bytes <br>
 * input: randA: 14 bytes random number from app
 * input: randT: 13 bytes random number from Tile
 * input: cid:   1 byte channel ID assigned by Tile
 * input: token: 4 bytes token used in open channel message
 * output: session_key (16 bytes)
 */
void tile_gen_session_key(const uint8_t *auth_key, const uint8_t *randA, const uint8_t *randT,
  const uint8_t cid, const uint8_t *token, uint8_t *session_key)
{
  WICED_BT_TRACE("\r\n\r\ntile_gen_session_key :");
  uint8_t msg[32];
  uint8_t *pMsg = &msg[0];

  memcpy(pMsg, randA, 14);
  pMsg += 14;
  memcpy(pMsg, randT, 13);
  pMsg += 13;
  *pMsg++ = cid;
  memcpy(pMsg, token, 4);
 /* process */
#ifdef HMAC256_DOTTE
 hmac_sha256(mac, auth_key, 128, msg, 256); /* DOtte source */
#else

 WICED_BT_TRACE("\r\n\r\nauth_key :");
 for (int i=0;i<16;i++)
 {
   WICED_BT_TRACE("%02x",auth_key[i]);
 }
 WICED_BT_TRACE("\r\n");
 WICED_BT_TRACE("msg :");
 for (int i=0;i<32;i++)
 {
   WICED_BT_TRACE(" %02x",msg[i]);
 }
 WICED_BT_TRACE("\r\n");
 hmac_sha256_ogay(auth_key, 16, msg, 32, session_key, 16); /* OGay source */
 WICED_BT_TRACE("\r\n");
 WICED_BT_TRACE("session_key :");
 for (int i=0;i<16;i++)
 {
   WICED_BT_TRACE(" %02x",msg[i]);
 }
 WICED_BT_TRACE("\r\n");
#endif
}


/**
 * @brief Tile broadcast key generation function. <br>
 * input: auth_key: 16 Bytes <br>
 * input: randT:    10 bytes random number from Tile
 * input: tile_id:  8 Tile ID
 * output: broadcast_key (16 bytes)
 */
void tile_gen_broadcast_key(const uint8_t *auth_key, const uint8_t *randT, const uint8_t *tile_id, uint8_t *broadcast_key)
{
  uint8_t msg[32];

  memset(msg, 0, 32);
  memcpy(msg, randT, 10);
  memcpy(&msg[10], tile_id, 8);

#ifdef HMAC256_DOTTE
 hmac_sha256(mac, auth_key, 128, msg, 256); /* DOtte source */
#else
 hmac_sha256_ogay(auth_key, 16, msg, 32, broadcast_key, 16); /* OGay source */
#endif
}

/**@brief Tile session key generation function. <br>
 * input: auth_key: 16 Bytes <br>
 * input: aco: 24 Bytes (generated during authentication) <br>
 * input: TileID: 8 Bytes <br>
 * output: session_key (16 bytes)
 */
void tile_session_key_gen_hash(const uint8_t* auth_key, const uint8_t* aco, const uint8_t* tileID, uint8_t* session_key)
{
	uint8_t msg[32];

  DBG_KEY_GEN_ENTER

	/* prepare the input message */
	memcpy(msg, aco, 24);
	memcpy(msg+24, tileID, 8);

	WICED_BT_TRACE("auth_key[0]: %0x\r\n",auth_key[0]);
	WICED_BT_TRACE("auth_key[1]: %0x\r\n",auth_key[1]);
	WICED_BT_TRACE("auth_key[2]: %0x\r\n",auth_key[2]);
	WICED_BT_TRACE("auth_key[3]: %0x\r\n",auth_key[3]);

    WICED_BT_TRACE("auth_key[8]: %0x\r\n",auth_key[8]);
    WICED_BT_TRACE("auth_key[9]: %0x\r\n",auth_key[9]);
    WICED_BT_TRACE("auth_key[10]: %0x\r\n",auth_key[10]);
    WICED_BT_TRACE("auth_key[11]: %0x\r\n",auth_key[11]);
	/* process */
#ifdef HMAC256_DOTTE
	hmac_sha256(mac, auth_key, 128, msg, 256); /* DOtte source */
#else
	hmac_sha256_ogay(auth_key, 16, msg, 32, session_key, 16); /* OGay source */
#endif

  WICED_BT_TRACE("session_key[0]: %0x\r\n",session_key[0]);
  WICED_BT_TRACE("session_key[1]: %0x\r\n",session_key[1]);
  WICED_BT_TRACE("session_key[2]: %0x\r\n",session_key[2]);
  WICED_BT_TRACE("session_key[3]: %0x\r\n",session_key[3]);

  DBG_KEY_GEN_EXIT

	return;
}

/**@brief Tile MIC hash. <br>
 * input: session_key: 16 Bytes <br>
 * input: nonce: uint64_t <br>
 * input: direction: uint8_t (0 for Tile originated, 1 for App originated) <br>
 * input: plaintext: up to 22 Bytes <br>
 * input: text_length in bytes (max 22) <br>
 * output: mic (4 bytes)
 */
void tile_mic_hash(const uint8_t* session_key, const uint32_t nonce, const uint8_t direction, const uint8_t* plaintext, const uint8_t text_length, uint8_t* mic)
{
	uint8_t msg[32];
  uint8_t length;

  DBG_MSG_AUTH_ENTER

  length = text_length;
  if (length > 22)
  {
    length = 22;
  }

	/* prepare the input message */
  memset(msg, 0, sizeof(msg));
	WRLE32(msg, nonce);
	/* We leave four bytes as zero, to emulate a 64-bit nonce. */
  msg[8] = direction;
  msg[9] = text_length;
	memcpy(msg+10, plaintext, length);

	/* process */
#ifdef HMAC256_DOTTE
	hmac_sha256(mac, session_key, 128, msg, 256); /* DOtte source */
#else
	hmac_sha256_ogay(session_key, 16, msg, 32, mic, 4); /* OGay source */
#endif

  DBG_MSG_AUTH_EXIT

	return;
}


/**
 * @brief Tile broadcast MIC hash. <br>
 * input: broadcast_key: 16 Bytes <br>
 * input: nonce: uint32_t <br>
 * input: plaintext: up to 27 Bytes <br>
 * input: text_length in bytes (max 27) <br>
 * output: mic (4 bytes)
 */
void tile_broadcast_mic_hash(const uint8_t* broadcast_key, const uint32_t nonce, const uint8_t* plaintext, const uint8_t text_length, uint8_t* mic)
{
  uint8_t msg[32];
  uint8_t length;

  length = text_length;
  if(length > 27)
  {
    length = 27;
  }

  memset(msg, 0, 32);
  memcpy(msg, &nonce, 4);
  memcpy(msg+4, &length, 1);
  memcpy(msg+5, plaintext, length);

#ifdef HMAC256_DOTTE
  hmac_sha256(mac, broadcast_key, 128, msg, 256); /* DOtte source */
#else
  hmac_sha256_ogay(broadcast_key, 16, msg, 32, mic, 4); /* OGay source */
#endif
}


/**@brief Verify Tile FW signature.
 * Usage: Compute the hash of the signed data using the same hash as the signer and
 * pass it to this function along with the signer's public key and the signature values (r and s).
 * input: public_key - The signer's public key (32 bytes)
 * input: hash       - The hash of the signed data (32 bytes)
 * input: signature  - The signature value  (64 bytes)
* return: 1 if the signature is valid, 0 if it is invalid.
*/
int tile_sign_verify(const uint8_t* key, const uint8_t* hash, const uint8_t* signature)
{
#ifndef TILE_NO_ECC
  return Tile_uECC_verify(key, hash, signature);
#else
  return 1;
#endif
}


#ifdef TEST_HMAC

/**@brief function for testing Tile authentication function.
 * input: none
 * output: time to complete in microseconds, -1 if result is wrong
 */
int test_tile_hash(void)
{
  int i = 0;
  int	res = 0;
  uint32_t tickstart, tickstop;
  uint8_t sresA[4];
  uint8_t sresT[4];

  /* example from h5 function from Bluetooth Core 4.1 */
  uint8_t key[16]         = {0xb0, 0x89, 0xc4, 0xe3, 0x9d, 0x7c, 0x19, 0x2c, 0x3a, 0xba, 0x3c, 0x21, 0x09, 0xd2, 0x4c, 0x0d};
  uint8_t randA[16]       = {0xd5, 0xcb, 0x84, 0x54, 0xd1, 0x77, 0x73, 0x3e, 0xff, 0xff, 0xb2, 0xec, 0x71, 0x2b, 0xae, 0xab};
  uint8_t randT[16]       = {0xa6, 0xe8, 0xe7, 0xcc, 0x25, 0xa7, 0x5f, 0x6e, 0x21, 0x65, 0x83, 0xf7, 0xff, 0x3d, 0xc4, 0xcf};
  uint8_t sresAresult[4]  = {0x74, 0x6a, 0xf8, 0x7e};
  uint8_t sresTresult[4]  = {0x1e, 0xeb, 0x11, 0x37};

  /* init the timer */
  NRF_TIMER1->MODE      = TIMER_MODE_MODE_Timer;
  NRF_TIMER1->BITMODE   = TIMER_BITMODE_BITMODE_16Bit;
  NRF_TIMER1->PRESCALER = 9;

  /* Clear and start the timer */
  NRF_TIMER1->TASKS_CLEAR = 1;
  nrf_delay_ms(1);
  NRF_TIMER1->TASKS_START = 1;
  nrf_delay_ms(1);

  /* capture start */
  NRF_TIMER1->TASKS_CAPTURE[0] = 1;

  /* THE operation we are testing */
  tile_device_auth_hash(key, randA, randT, sresA, sresT);

  /* capture stop */
  NRF_TIMER1->TASKS_CAPTURE[1] = 1;
  nrf_delay_ms(1);

  /* calculate time diff */
  tickstart = NRF_TIMER1->CC[0];
  nrf_delay_ms(1);
  tickstop = NRF_TIMER1->CC[1];

  res = TIMER_US16((tickstop - tickstart), 9);

  for(i = 0; i < 4; i++)
  {
    if(   (sresA[i] != sresAresult[i])
      ||  (sresT[i] != sresTresult[i])  )
    {
      res = -1;
      break;
    }
  }

	return (res);
}

#endif

