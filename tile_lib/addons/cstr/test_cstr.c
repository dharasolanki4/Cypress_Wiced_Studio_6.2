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

/** @file test_cstr.c
 ** @brief Test cstr.c
 */
#include "assert/tile_assert.h"
#include "logs/tile_logs.h"
#include "cstr/test_cstr.h"
#include "cstr/cstr.h"

#include <string.h>

#if 0 /* Code is disabled since it was only used for testing purposes */
static void test_percent_s(char buf[], int size, int test_num, int expected_result)
{
  char test[128] = {0};
  
  int cnt;

  memset(test, 'A', test_num);
  test[test_num] = '\0';

  cnt = cstr_printf(buf, size, "%s", test);

  TILE_ASSERT(cnt == expected_result);
}

/**
 * Only for development purposes. For actual production, this
 * code should not be called and left unlinked from final build.
 */
void test_cstr(void)
{
  char buf[64] = {0};

  int cnt;
  
  // should return -1, since
  // buf needs to be 10 bytes because
  // 123456789 is 9 characters, + 1 for null term
  cnt = cstr_printf(buf, 9, "%d", 123456789);

  TILE_ASSERT(cnt < 0);

  test_percent_s(buf, sizeof(buf), 0, 0);
  test_percent_s(buf, sizeof(buf), 1, 1);
  test_percent_s(buf, sizeof(buf), 63, 63);
  test_percent_s(buf, sizeof(buf), 64, -1);

  char smbuf[9] = {0};

  cnt = cstr_printf(smbuf, sizeof(smbuf), "012345%d",6789);

  TILE_ASSERT(cnt < 0);
  
  cnt = cstr_printf(smbuf, sizeof(smbuf)-1, "012345%d",6789);
  
  TILE_ASSERT(cnt < 0);
}
#endif
