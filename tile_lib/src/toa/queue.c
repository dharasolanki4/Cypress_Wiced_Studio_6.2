/* NOTICE
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
#include "queue.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>


/**
 * @defgroup queue Tile Queue
 *
 * @brief This file provides a queue data structure for use by TOA.
 *
 * This queue is designed to allow the user to pass in an arbitrary-sized
 * buffer to be used as the queue data structure. The @ref initializeQueue
 * function initializes the buffer so that it can be used as a queue. When a
 * data element is placed on the queue, first the length of the data is added
 * onto the queue buffer followed by the data itself. Similarly, to remove an
 * element, first the length of the element is read off the queue and then that
 * amount of data is removed.
 *
 * In order to minimize the amount of data required to be stored in the
 * @ref queue_t struct, the queue maintains that the valid data is always
 * shifted to the beginning of the buffer, meaning that on each call to
 * @ref getItem, there will be a memmove to bring the remaining queue items to
 * the front of the buffer. If this behavior ends up being too detrimental to
 * performance, then this queue should be changed to be a circular fifo or
 * something like cbuf in tile_lib/addons.
 *
 * Example usage:
 * \code{.c}
 * static uint8_t queue_buffer[100];
 * queue_t *queue = initializeQueue(buffer, sizeof(buffer));
 * putItem(queue, (uint8_t[]){1,2,3,4}, 4);
 * \endcode
 *
 * @{
 */

/**
 * Initialize a queue from a given buffer.
 *
 * @param[in] buffer  Buffer to use to turn into a queue.
 * @param[in] size    Size of buffer.
 *
 * @return Pointer to buffer, but casted as an initialized @ref queue_t.
 */
queue_t *initializeQueue(uint8_t *buffer, uint16_t size)
{
  queue_t *queue = (queue_t*)buffer;
  queue->end = 0;
  queue->size = size - offsetof(queue_t, buffer);

  return queue;
}


/**
 * Check if the queue is empty.
 *
 * @param[in] queue  The queue to check.
 *
 * @return 1 if empty and 0 otherwise.
 */
int isEmpty(queue_t *queue)
{
  return 0 == queue->end;
}


/**
 * Put data onto the queue.
 *
 * @param[in] queue   The queue to add to.
 * @param[in] data    The data to add.
 * @param[in] datalen The length of the data.
 *
 * @return 0 on success, -1 if there's not enough space.
 */
int putItem(queue_t *queue, uint8_t *data, uint8_t datalen)
{
  if((datalen + 1) > (queue->size - queue->end))
  {
    return -1;
  }

  queue->buffer[queue->end++] = datalen;
  memcpy(queue->buffer + queue->end, data, datalen);
  queue->end += datalen;

  return 0;
}


/**
 * Get data from the queue.
 *
 * @param[in] queue    The queue to get data from.
 * @param[out] data    The buffer to store the data in (must be large enough!).
 * @param[out] datalen The length of the data that was removed from the queue.
 *
 * @return 0 on success, -1 if there's no data on the queue.
 */
int getItem(queue_t *queue, uint8_t *data, uint8_t *datalen)
{
  if(isEmpty(queue))
  {
    return -1;
  }

  *datalen = queue->buffer[0];
  memcpy(data, &queue->buffer[1], *datalen);
  queue->end -= *datalen + 1;
  memmove(queue->buffer, queue->buffer + *datalen + 1, queue->end);

  return 0;
}

/** @} */


#if 0 /* Test */

#include <stdio.h>
void print_hex(uint8_t *data, uint8_t len)
{
  for(int i = 0; i < len; i++)
    printf("%02x", data[i]);
  printf("\n");
}



int main(void)
{
  uint8_t data[100] = {0};
  uint8_t datalen = 1;

  uint8_t queue_buf[200];
  queue_t *queue = (queue_t*)queue_buf;
  initializeQueue(queue, sizeof(queue_buf));
  printf("queue->size: %d\n", queue->size);

  while(0 == putItem(queue, data, datalen))
  {
    print_hex(data, datalen);
    data[datalen] = datalen;
    ++datalen;
  }

  printf("isEmpty = %d\n", isEmpty(queue));
  printf("Size = %d\n", queue->end);

  while(0 == getItem(queue, data, &datalen)) print_hex(data, datalen);

  printf("isEmpty = %d\n", isEmpty(queue));
  printf("Size = %d\n", queue->end);

  return 0;
}

#endif
