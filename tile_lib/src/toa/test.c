/*
 * NOTICE
 *
 * © 2014 Tile Inc.  All Rights Reserved.

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


#include "test.h"
#include "toa.h"

#if TILE_SUPPORT_TEST

#include "modules/tile_test_module.h"
#include "tile_lib.h"
#include "tdt.h"
#include "../tileHash.h"
#include "drivers/tile_button_driver.h"
#include "modules/tile_adv_int_module.h"
#include "modules/tile_tofu_module.h"
#include "drivers/tile_timer_driver.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

//#warning "===============================> Test code enabled "

static void test_send_response(uint8_t cid, uint8_t response, uint8_t* data, uint8_t datalen);
static void test_cmd_toa_rsp(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_tofu(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_button_press(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_number(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_security(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_adv_int(const uint8_t cid, const uint8_t *data, uint8_t datalen);
static void test_cmd_rand_get(const uint8_t cid, const uint8_t* data, uint8_t datalen);
static void test_cmd_broadcast(const uint8_t cid, const uint8_t *data, uint8_t datalen);
static void test_cmd_start_timer(const uint8_t cid, const uint8_t * data, uint8_t datalen);
struct tile_test_module *test = NULL;

static uint8_t cached_cid; /* Save CID for commands forwarded to application */


int tile_test_register(struct tile_test_module *module)
{
  test = module;

  return TILE_SUCCESS;
}


int tile_test_response(uint8_t code, uint8_t *response, uint8_t length)
{
  if(code < TILE_TEST_MODULE_CODE_BASE)
  {
    return TILE_ERROR_ILLEGAL_PARAM;
  }

  test_send_response(cached_cid, code, response, length);

  return TILE_SUCCESS;
}


/* @brief: process incomming test packets */
void test_process_command(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  if(data[0] >= TILE_TEST_MODULE_CODE_BASE && NULL != test)
  {
    /* Non-Tile Lib test commands */
    cached_cid = cid;
    uint8_t ret = test->process(data[0], (uint8_t*)&data[1], datalen-1);
    if(TILE_ERROR_ILLEGAL_OPERATION == ret)
    {
      uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    }
    return;
  }

  switch(data[0])
  {
    case TEST_CMD_TOA_RSP:
      test_cmd_toa_rsp(cid, &data[1], datalen-1);
    break;

    case TEST_CMD_BUTTON_PRESS:
      test_cmd_button_press(cid, &data[1], datalen-1);
    break;

    case TEST_CMD_NUMBER:
      test_cmd_number(cid, &data[1], datalen-1);
    break;

    case TEST_CMD_TOFU:
      test_cmd_tofu(cid, &data[1], datalen-1);
    break;

    case TEST_CMD_SECURITY:
      test_cmd_security(cid, &data[1], datalen-1);
    break;

    case TEST_CMD_ADV_INT:
      test_cmd_adv_int(cid, &data[1], datalen - 1);
    break;

    case TEST_CMD_RAND_GET:
      test_cmd_rand_get(cid, &data[1], datalen - 1);
    break;

    case TEST_CMD_BROADCAST:
      test_cmd_broadcast(cid, &data[1], datalen - 1);
    break;

    case TEST_CMD_START_TIMER:
      test_cmd_start_timer(cid, &data[1], datalen - 1);
    break;

    default:
    {
      uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
      toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    }
    break;
  }
}


static void test_send_response(uint8_t cid, uint8_t response, uint8_t* data, uint8_t datalen)
{
  uint8_t  transaction[TOA_MPS + 6];
  memset(&transaction, 0, sizeof(transaction));

  transaction[0]  = response;
  if((0 != datalen) && (NULL != data))
  {
    memcpy(&transaction[1], data, datalen);
  }
  toa_send_response(cid, TOA_RSP_TEST, (uint8_t*) &transaction, datalen+1);
}


static void test_cmd_toa_rsp(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  typedef struct
  {
    uint32_t numberOfPackets;
    uint32_t packetsCounter;

  }test_cmd_toa_rsp_transaction;

  static test_cmd_toa_rsp_transaction mytest;

  memcpy(&mytest, data, 8);

  for(int i = 0; i < mytest.numberOfPackets; i++)
  {
    mytest.packetsCounter = i;
    test_send_response(cid, TEST_RSP_TOA_RSP, (uint8_t*)&mytest, MIN(0+i, TOA_MPS + 5));
  }
}

static void test_cmd_tofu(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  extern struct tile_tofu_module *tofu;

  if(!toa_get_feature(TOA_FEATURE_TOFU))
  {
    /* Only do this test if TOFU is supported */
    uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    return;
  }

  switch(data[0]) {
    case TEST_CMD_TOFU_BLOCK_LEN:
    {
      memcpy(&tofu->state.block_len, &data[1], 4);
    }
    break;
  }
}

static void test_cmd_rand_get(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  uint8_t buff[100];
  uint8_t index = 0;
  uint8_t size  = 0;

#include "drivers/tile_random_driver.h"
extern struct tile_random_driver *random;

  size = data[0];
  if(size > 100)
  {
    size = 100;
  }

  memset(buff, 0, 100);
  random->random_bytes(buff, size);

  while((100 - index) > (TOA_MPS-1))
  {
    test_send_response(cid, TEST_RSP_RAND_GET_CONT, &buff[index], (TOA_MPS-1));
    index += (TOA_MPS-1);
  }
  test_send_response(cid, TEST_RSP_RAND_GET_END, &buff[index], 100 - index);
}

static void test_cmd_button_press(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  if(!toa_get_feature(TOA_FEATURE_TDT))
  {
    /* Only do this test if TDT is supported */
    uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    return;
  }

  switch(data[0])
  {
    case TEST_CMD_BUTTON_SINGLE:
      tile_button_pressed();
    break;

    case TEST_CMD_BUTTON_LONG:
      tdt_process_tap(TDT_NOTIFY_LT);
    break;

    default:
    {
      uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_PARAMETERS};
      toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    }
    break;
  }
}

static void test_cmd_security(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
#if 0 /* Removed this to get rid of excess static variables */
  /* TODO: Make these less static */
  static uint8_t   public_key[64];
  static uint8_t   hash[32];
  static uint8_t   signature[64];

  switch(data[0])
  {
    case TEST_CMD_SECURITY_PUB0:
      memcpy(public_key, data+1, 13);
    break;
    case TEST_CMD_SECURITY_PUB1:
      memcpy(public_key+13, data+1, 13);
    break;
    case TEST_CMD_SECURITY_PUB2:
      memcpy(public_key+26, data+1, 13);
    break;
    case TEST_CMD_SECURITY_PUB3:
      memcpy(public_key+39, data+1, 13);
    break;
    case TEST_CMD_SECURITY_PUB4:
      memcpy(public_key+52, data+1, 12);
    break;
    case TEST_CMD_SECURITY_HASH0:
      memcpy(hash, data+1, 13);
    break;
    case TEST_CMD_SECURITY_HASH1:
      memcpy(hash+13, data+1, 13);
    break;
    case TEST_CMD_SECURITY_HASH2:
      memcpy(hash+26, data+1, 6);
    break;
    case TEST_CMD_SECURITY_SIGN0:
      memcpy(signature, data+1, 13);
    break;
    case TEST_CMD_SECURITY_SIGN1:
      memcpy(signature+13, data+1, 13);
    break;
    case TEST_CMD_SECURITY_SIGN2:
      memcpy(signature+26, data+1, 13);
    break;
    case TEST_CMD_SECURITY_SIGN3:
      memcpy(signature+39, data+1, 13);
    break;
    case TEST_CMD_SECURITY_SIGN4:
      memcpy(signature+52, data+1, 12);
    break;
    case TEST_CMD_SECURITY_VERIFY_SIGNATURE:
    {
      /* TODO: Add back in when ECC is available */
//      uint8_t rsp[2];

//      rsp[0] = TEST_RSP_SECURITY_VERIFY_SIGNATURE;
//
//      if(1 == tile_sign_verify(public_key, hash, signature))
//      {
//        rsp[1] = 1;
//      }
//      else
//      {
//        rsp[1] = 0;
//      }
//      test_send_response(cid, TEST_RSP_SECURITY, rsp, 2);
    }
    break;

    default:
    {
      uint8_t rsp[] = {TEST_RSP_ERROR, data[0], ERROR_PARAMETERS};
      toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    }
    break;
  }
#endif
}

static void test_cmd_adv_int(const uint8_t cid, const uint8_t *data, uint8_t datalen)
{
  extern struct tile_adv_int_module *adv;
  uint16_t adv_interval;
  memcpy(&adv_interval, data, 2);

  if(!toa_get_feature(TOA_FEATURE_ADV_INT))
  {
    /* Only do this if ADV_INT is supported */
    uint8_t rsp[] = {TEST_RSP_ERROR, data[0], TOA_ERROR_UNSUPPORTED};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    return;
  }

  if((adv_interval >= 0x20) && (adv_interval <= 0x4000))
  {
    uint8_t rsp[2];

    adv->set(adv_interval);
    /* Respond with OK */
    memcpy(rsp, &adv_interval, 2);
    test_send_response(cid, TEST_RSP_ADV_INT, &rsp[0], 2);
  }
}

/* TODO: get tile time! */
static uint32_t get_tileTime()
{
  return 0;
}


/**
 * @brief Keeps track of the cid who started a timer.
 *
 * Length should equal the maximumum number of test_id's allocated to the timer test interface (see tile_timer_driver.h)
 * which is 8 as of writing this.
 */
static uint8_t test_timer_cids[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/**
 * @brief: test timer callback handler called in tile_timer.c.
 *
 * When called with the expired timer's timer_id, a test response is sent to the caller's cid
 * indicating that the timer has finished. The response message includes the tile time at expiration.
 */
int test_cmd_start_timer_handler(int id)
{
  /* Find the cid that started this timer */
  uint8_t cid = test_timer_cids[id - TILE_TEST_TIMER1];

  /* Delete the cid record */
  test_timer_cids[id - TILE_TEST_TIMER1] = 0xff;

  /* Prepare the response */
  uint8_t rsp[] = {id - TILE_TEST_TIMER1, 0, 0, 0, 0};

  uint32_t tile_time = get_tileTime();
  memcpy(&rsp[1], &tile_time, 4);

  test_send_response(cid, TEST_RSP_TIMER_DONE, rsp, 5);
  return 0;
}

/**
 * @brief: start a test timer.
 *
 * @param data holds the timer duration time in ms.
 *
 * Upon success, a response is sent, with its payload containing the allocated timer id, and
 * the tile time at which it was started.
 */
static void test_cmd_start_timer(const uint8_t cid, const uint8_t * data, uint8_t datalen)
{
  extern struct tile_timer_driver * timer;

  if (datalen < sizeof(uint32_t))
  {
    uint8_t rsp[] = {TEST_RSP_ERROR, TEST_CMD_START_TIMER, TOA_ERROR_PARAMETERS};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    return;
  }

  /* Find available test timer */
  int k;
  for (k = 0; k < sizeof(test_timer_cids); k++)
  {
    if (test_timer_cids[k] == 0xff)
    {
      test_timer_cids[k] = cid;
      break;
    }
  }

  if (k == sizeof(test_timer_cids))
  {
    uint8_t rsp[] = {TEST_RSP_ERROR, TEST_CMD_START_TIMER, TOA_ERROR_INVALID_STATE};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
    return;
  }

  uint32_t duration;
  memcpy(&duration, &data[0], sizeof(duration));

  timer->start(TILE_TEST_TIMER1 + k, duration);

  uint8_t rsp[] = {k, 0, 0, 0, 0};

  uint32_t tile_time = get_tileTime();
  memcpy(&rsp[1], &tile_time, 4);

  test_send_response(cid, TEST_RSP_START_TIMER, rsp, sizeof(rsp));
  return;
}

static void test_cmd_number(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  switch(data[0])
  {
    case 1:
    break;

    case 2:
    break;

    case 3:
    break;

    default:
    break;
  }
}


static void test_cmd_broadcast(const uint8_t cid, const uint8_t *data, uint8_t datalen)
{
  if(datalen < 2)
  {
    uint8_t rsp[] = {TEST_RSP_ERROR, TEST_CMD_BROADCAST, TOA_ERROR_PARAMETERS};
    toa_send_response(cid, TOA_RSP_TEST, rsp, sizeof(rsp));
  }

  uint8_t rsp[TOA_MPS];
  rsp[0] = TEST_RSP_BROADCAST;
  uint8_t number = data[0];
  uint8_t size = data[1];

  if(size > TOA_MPS - 1)
  {
    size = TOA_MPS - 1;
  }

  for(int i = 0; i < number; i++)
  {
    toa_send_broadcast(TOA_RSP_TEST, rsp, size+1);
  }
}


#else

void test_process_command(const uint8_t cid, const uint8_t* data, uint8_t datalen)
{
  toa_send_response_error(cid, TOA_RSP_ERROR_UNSUPPORTED, TOA_CMD_TEST);
}

void test_flashReady(void)
{
}

int test_cmd_start_timer_handler(int id)
{
  return 0;
}

#endif  // TILE_SUPPORT_TEST
