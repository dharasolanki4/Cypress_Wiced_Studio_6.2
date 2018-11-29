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



#ifndef TEST_H_
#define TEST_H_


#include <stdint.h>

/** \defgroup TEST
Tile testing features:
  - Send \ref TOA responses
  - Simulate button press
  - Reboot Tile
  - Set \ref TOFU block length
  - Magic
	
*@{
*/

/**
 * @brief TEST response codes
 */
enum TEST_RSP
{
  TEST_RSP_TOA_RSP        = 0x01,
	/**< Response sent by TOA test */
  TEST_RSP_NUMBER         = 0x02,
	/**< Response sent by Number test. The format of this response will
	     vary depending on the configuration of the Number test */
  TEST_RSP_SECURITY       = 0x03,
	/**< Response sent by SECURITY test: @ref TEST_RSP_SECURITY */
  TEST_RSP_ADV_INT        = 0x04,
  /**< Response sent by ADV_INT test command */
  TEST_RSP_RAND_GET_CONT  = 0x05,
  /**< Response sent by RAND_GET test command when there are more responses to come */
  TEST_RSP_RAND_GET_END   = 0x06,
  /**< Response sent by RAND_GET test command when there are no more responses to come */
  TEST_RSP_BROADCAST      = 0x07,
  /**< Response sent by BROADCAST test command. Payload is arbitrary data. */
  TEST_RSP_START_TIMER    = 0x08,
  /**< Response sent by START_TIMER test command <br>
  Params | Size | Description
  -------|------|------------
  Timer id | 1 byte | test timer ID number
  Tile time | 4 bytes | the time at which the timer was started (in Tile time units)
  */
  TEST_RSP_TIMER_DONE     = 0x09,
  /**< Response sent by START_TIMER test command when timer has finished <br>
  Params | Size | Description
  -------|------|------------
  Timer id | 1 byte | test timer ID number
  Tile time | 4 bytes | the time at which the timer stopped (in Tile time units)
  */
  TEST_RSP_ERROR          = 0x20,
  /**< This response is returned when there is an error processing a TEST_CMD. Please see @ref TOA_FEATURE_ERROR_CODES
  for format of these error messages */
};

/**
 * @brief TEST command codes
 */
enum TEST_CMD
{  
  TEST_CMD_TOA_RSP          = 0x01,
	/**< This test triggers several TOA responses to be sent. The parameters
	are<br>
	
	Params | Size | Description
	-------|------|------------
	Number of packets | 4 bytes | The number of packets to send as TOA responses
	Packets counter | 4 bytes | Counter of how many packets have been sent. Ignored.
	*/
  TEST_CMD_BUTTON_PRESS     = 0x02,
	/**< This test simulates a button press. The parameters are<br>
	
	Params | Size | Description
	-------|------|------------
	Button press type | 1 byte | Single or long tap. See \ref TEST_BUTTON
	*/
  TEST_CMD_NUMBER           = 0x03,
	/**< This test is magic, only to be known by the firmware team. */
  TEST_CMD_TOFU             = 0x04,
	/**< This just sets the block length for TOFU. Parameters:<br>
	Params | Size | Description
    -------|------|------------
	Block size | 4 bytes | Set block length to this size
	*/
  TEST_CMD_SECURITY         = 0x05,
	/**< This test security features <br>
	@ref TEST_CMD_SECURITY | 1 byte | The Security Test Command Code
	Payload | varies | depends on the command
	*/
  TEST_CMD_ADV_INT          = 0x06,
  /**< This allows writes to advertising interval beyond the range allowed
  by @ref ADV_INT
  @ref TEST_CMD_ADV_INT | 1 byte | The test command code
  Payload | 2 bytes | Advertising inteval, as specified in @ref ADV_INT
  */
  TEST_CMD_RAND_GET         = 0x07,
  /**< Get xx random bytes
  This comman will generate as many random bytes as requested (up to 100).
  It will then send its buffer filled with the random bytes.
  The full buffer of 100 Bytes is always provided in responses.
  The responses used are @ref TEST_RSP_RAND_GET_CONT and @ref TEST_RSP_RAND_GET_END
  @ref TEST_CMD_RAND_GET | 1 byte | The test command code
  Payload | 1 byte | Number of Bytes to get
  */
  TEST_CMD_BROADCAST        = 0x08,
  /**< Generate N broadcast messages of size K. Format:
  @ref TEST_CMD_BROADCAST | Number N | Size K
  ----------------------- | -------- | ------
  1 byte                  | 1 byte   | 1 byte
  */
  TEST_CMD_START_TIMER      = 0x09,
  /**< This tests the timers <br>
  @ref TEST_CMD_START_TIMER | 1 byte | the start timer command code
  Payload | 4 bytes | Timer duration in milliseconds
  */
};


/**
 * @brief TOFU testing commands
 */
enum TEST_TOFU
{
  TEST_CMD_TOFU_BLOCK_LEN  = 0x00,
};

/**
 * @brief Security Test Commands
 */
enum TEST_CMD_SECURITY
{
  TEST_CMD_SECURITY_PUB0  = 0x00,
  TEST_CMD_SECURITY_PUB1  = 0x01,
  TEST_CMD_SECURITY_PUB2  = 0x02,
  TEST_CMD_SECURITY_PUB3  = 0x03,
  TEST_CMD_SECURITY_PUB4  = 0x04,
  TEST_CMD_SECURITY_HASH0 = 0x05,
  TEST_CMD_SECURITY_HASH1 = 0x06,
  TEST_CMD_SECURITY_HASH2 = 0x07,
  TEST_CMD_SECURITY_SIGN0 = 0x08,
  TEST_CMD_SECURITY_SIGN1 = 0x09,
  TEST_CMD_SECURITY_SIGN2 = 0x0a,
  TEST_CMD_SECURITY_SIGN3 = 0x0b,
  TEST_CMD_SECURITY_SIGN4 = 0x0c,
  TEST_CMD_SECURITY_VERIFY_SIGNATURE = 0x0d,
};

/**
 * @brief Security Test Responses
 */
enum TEST_RSP_SECURITY
{
  TEST_RSP_SECURITY_VERIFY_SIGNATURE = 0x00,
};

/**
 * @brief Types of button presses which can be triggered by \ref TEST_CMD_BUTTON_PRESS
 */
enum TEST_BUTTON
{  
  TEST_CMD_BUTTON_SINGLE  = 0x00,
  TEST_CMD_BUTTON_LONG    = 0x01,
};
/**@}*/


void test_process_command(const uint8_t cid, const uint8_t* data, uint8_t datalen);
int test_cmd_start_timer_handler(int id);
    
#endif  // TESTS_H_
