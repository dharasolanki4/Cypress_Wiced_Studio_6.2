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

/** @file sdev.c
 ** @brief Generic serial device interface
 */

#include "sdev/sdev.h"

#include <stdint.h>
#include <stddef.h>


/**
 * Initialize an sdev_t structure.
 */
void sdev_create(sdev_t *dev, sdev_rw_func_t rw_func, void *context)
{
  dev->rw = rw_func;
  dev->context = context;
  dev->callback = NULL;
}


/**
 * Register a callback to receive sdev status updates.
 */
void sdev_register_callback(sdev_t *dev, sdev_status_cb_t callback)
{
  dev->callback = callback;
}


/**
 * Read bytes from sdev.
 *
 * @param[in] dev     sdev to use.
 * @param[in] dst     Destination buffer.
 * @param[in] max_len Maximum length to read in.
 *
 * @return Number of bytes read, or negative number on error.
 */
int32_t sdev_read(sdev_t *dev, uint8_t *dst, int32_t max_len)
{
  int32_t wr_len = 0;
  int32_t rd_len = max_len;
  int32_t ret = dev->rw(dst, &rd_len, NULL, &wr_len, dev->context);
  return ret < 0 ? ret : rd_len;
}


/**
 * Write bytes to sdev.
 *
 * @param[in] dev   sdev to use.
 * @param[in] src   Source buffer to write from.
 * @param[in] len   Length of write buffer.
 *
 * @return Number of bytes written, or negative number on error.
 */
int32_t sdev_write(sdev_t *dev, uint8_t *src, int32_t len)
{
  int32_t rd_len = 0;
  int32_t wr_len = len;
  int32_t ret = dev->rw(NULL, &rd_len, src, &wr_len, dev->context);
  return ret < 0 ? ret : wr_len;
}


/**
 * Read and write sdev.
 *
 * @param[in] dev        sdev to use.
 * @param[in] rd_buf     Buffer to read data into.
 * @param[in,out] rd_len Length on rd_buf on input and total read bytes on output.
 * @param[in] wr_buf     Buffer to write data from.
 * @param[in,out] wr_len Length of data to write on input and total written bytes on output.
 *
 * @return 0 on success and negative on failure.
 */
int32_t sdev_read_write(sdev_t *dev, uint8_t *rd_buf, int32_t *rd_len, uint8_t *wr_buf, int32_t *wr_len)
{
  return dev->rw(rd_buf, rd_len, wr_buf, wr_len, dev->context);
}


void sdev_callback(sdev_t *dev, enum sdev_status status)
{
  if(NULL != dev->callback)
  {
    dev->callback(status);
  }
}
