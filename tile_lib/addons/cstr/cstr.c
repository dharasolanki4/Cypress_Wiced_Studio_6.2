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

/** @file cstr.c
 ** @brief String helper functions
 */


#include "cstr.h"

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/**
 * integer to ascii
 * @param[out]  dst  the destination string buffer
 * @param[in]   num  the number to convert
 */
void itoa(char *dst, int32_t num)
{
  /* Handle special cases */
  if(num == 0)
  {
    *dst++ = '0';
    return;
  }
  else if(num < 0)
  {
    *dst++ = '-';
    num = -num;
  }

  /* Convert in reverse order */
  char *start = dst;
  while(num > 0)
  {
    *dst++ = '0' + (num % 10);
    num /= 10;
  }

  /* Add a null terminator */
  *dst-- = '\0';

  /* Reverse digits */
  while(dst > start)
  {
    char tmp = *dst;
    *dst-- = *start;
    *start++ = tmp;
  }
}


/**
 * sprintf replacement, since the built-in one uses > 4kB of code.
 *
 * NOTE: Only supports %s format specifier for now.
 *
 * @param[out] buf    Buffer to write to.
 * @param[in]  size   Size of the buffer.
 * @param[in]  fmt    printf-like format string.
 * @parma[in]  args   initialized va_list
 *
 * @return the number of characters written to @ref buf. If an encoding error occurs,
 * a negative number is returned.
 */
int cstr_printf_v(char *buf, int size, const char *fmt, va_list args)
{
  const char *start = buf;
  
  // end points to the second-to-last character in buf, leave the last
  // character for '\0'
  const char *end = buf + size - 1;
  
  bool encoding_error = false;
  
  // handle special cases
  if (size < 1) {
    return 0;
  }
  
  for(; '\0' != *fmt && buf < end; fmt++)
  {
    if('%' == *fmt && 's' == *(fmt+1))
    {
      char *p = va_arg(args, char *);

      if (strlen(p) <= end - buf)
      {
        while('\0' != *p)
        {
          *buf++ = *p++;
        }
      }
      else
      {
        // not enough room in buf for string
        encoding_error = true;
        break;
      }
      
      fmt++;
    }
    else if('%' == *fmt && 'd' == *(fmt+1))
    {
      // buffer for itoa.
      // largest digit possible is 2^64 = 1.8E19 
      // thats 20 digits long
      // +1 for null term
      char num_buf[21] = {0};
      
      int num = va_arg(args, int);
      itoa(num_buf, num);
      
      int len = strlen(num_buf);
      
      if (len < end - buf)
      {
        strncpy(buf, num_buf, len);
        buf += len;
      }
      else
      {
        // not enough room in buf for string
        encoding_error = true;
        break;
      }
      
      fmt++;
    }
    else
    {
      *buf++ = *fmt;
    }
  }
  *buf = '\0';
  return encoding_error ? -1 : buf - start;
}

/**
 * sprintf replacement, user friendly interface
 *
 * NOTE: Only supports %s format specifier for now.
 *
 * @param[out] buf   Buffer to write to.
 * @param[in]  fmt   printf-like format string.
 *
 * @return  the number of characters written to buf
 */
int cstr_printf(char *buf, int size, const char *fmt, ...)
{
  va_list args;
  int ret;

  va_start(args, fmt);
  
  ret = cstr_printf_v(buf, size, fmt, args);
  
  va_end(args);
  return ret;
}


