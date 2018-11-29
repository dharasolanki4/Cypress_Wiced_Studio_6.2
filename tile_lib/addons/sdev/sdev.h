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

/** @file sdev.h
 ** @brief Generic serial device interface
 */

#ifndef SDEV_H_
#define SDEV_H_

#include <stdint.h>

enum sdev_status {
  SDEV_DATA_READY,
};


/**
 * Read/write function. Behavior should be as follows:
 * - rd_len and wr_len are pointers to a value with the number of bytes to read
 *   and write. At the end of the function, they should be populated with the
 *   number of bytes read and written, respectively.
 * - If either rd_buf or wr_buf is NULL, only do the read or write operation
 *   for the non-NULL buffer.
 * - If both rd_buf and wr_buf are non-NULL, the behavior is implementation
 *   specific, and the read and write may happen in the order that makes sense
 *   for the application. For example, an I2C transaction probably would do
 *   the write first, then the read with a repeated start. A SPI transaction
 *   would probably do both the read and the write simultaneously.
 * - The return value should be zero on success, and -1 on failure.
 */
typedef int32_t (*sdev_rw_func_t)(uint8_t *rd_buf, int32_t *rd_len, uint8_t *wr_buf, int32_t *wr_len, void *context);
typedef void (*sdev_status_cb_t)(enum sdev_status status);

typedef struct sdev {
  sdev_rw_func_t rw;
  sdev_status_cb_t callback;
  void *context;
} sdev_t;

#define SDEV_INIT(rw_func, ctext) { \
  .rw      = rw_func,               \
  .context = ctext,                 \
}

void sdev_create(sdev_t *dev, sdev_rw_func_t rw_func, void *context);
void sdev_register_callback(sdev_t *dev, sdev_status_cb_t callback);
int32_t sdev_read(sdev_t *dev, uint8_t *dst, int32_t max_len);
int32_t sdev_write(sdev_t *dev, uint8_t *src, int32_t len);
int32_t sdev_read_write(sdev_t *dev, uint8_t *rd_buf, int32_t *rd_len, uint8_t *wr_buf, int32_t *wr_len);
void sdev_callback(sdev_t *dev, enum sdev_status status);

#endif
