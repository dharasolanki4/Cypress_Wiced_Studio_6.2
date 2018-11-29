
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
 
#ifndef TKA_H_
#define TKA_H_


#include <stdint.h>

/** \defgroup TKA
Tile Keep Alive Feature:
- TKA supports Configuration and Notifications.
- TKA uses TOA_CMD_TKA Code from client to server and TOA_RSP_TKA Code from server to client.
- Tile Keep Alive provides a mechanism for the Tile device (the Server), to check that the client Application is still responding.
- The Server will send a CHECK response after the CheckDelay expires and then expects an ACK command before the AckDelay expires.
- If the Client does not respond to the CHECK response within AckDelay, then the Server will disconnect the link.
*  @{
*/


/**
 * @brief TKA Command Codes
 */
enum
{
  TKA_CMD_CONFIG      = 0x01,
  /**< This Command is used to configure TKA <br>
  Response: @ref TKA_RSP_CONFIG or an ERROR <br>  
  The Configuration will NOT persist after disconnection.<br>  
  After receiving a @ref TKA_CMD_CONFIG command, the CheckDelay timer is restarted.<br>

  Format for TKA Feature:
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)

  Format for TKA_OLD Feature (Obsolete):
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  RFU         | 3 Bytes   | Reserved and Ignored (Set to 0)
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)
  Count       | 2 Bytes   | Little Endian uint16_t, Count parameter shall be set to 0 in config requests and will be ignored
  RFU         | 2 Bytes   | Reserved and Ignored (Set to 0)
  */

  TKA_CMD_READ_CONFIG = 0x02,
  /**< This Command is used to read TKA configuration <br>
  Response: @ref TKA_RSP_READ_CONFIG or an ERROR <br> 

  Format: there is no parameter.
  */ 
 
  TKA_CMD_ACK         = 0x03,
  /**< This Command is used to acknowledge TKA Notifications <br>
  If the server does not receive a TKA_RSP_ACK command before the AckDelay timer expires, 
  then the server will disconnect the link.

  Format: there is no parameter.
  */ 
};

/**
 * @brief TKA Response Codes
 */
enum
{
  TKA_RSP_CONFIG      = 0x01,
  /**< This response is the successful response to @ref TKA_CMD_CONFIG Command. <br>
  The response contains the actual configuration (should be similar than the one in Command). <br>

  Format for TKA Feature:
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  Count       | 2 Bytes   | Little Endian uint16_t, Count contains the number of times a disconnect due to TKA occured since last reset.
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)

  Format for TKA_OLD Feature (Obsolete):
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  RFU         | 3 Bytes   | Reserved and Ignored (Set to 0)
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)
  Count       | 2 Bytes   | Little Endian uint16_t, Count parameter shall be set to 0 in config requests and will be ignored
  RFU         | 2 Bytes   | Reserved and Ignored (Set to 0)
  */
  
  TKA_RSP_READ_CONFIG = 0x02,
  /**< This response is the successful response to @ref TKA_CMD_READ_CONFIG Command. <br>
  The response contains the actual configuration being used. <br>

  Format for TKA Feature:
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  Count       | 2 Bytes   | Little Endian uint16_t, Count contains the number of times a disconnect due to TKA occured since last reset.
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)

  Format for TKA_OLD Feature (Obsolete):
  Param       | Size      | Description
  ----------- | --------- | -----------
  Activate    | 1 Byte    | 1 to activate, 0 to deactivate, other values reserved
  RFU         | 3 Bytes   | Reserved and Ignored (Set to 0)
  CheckDelay  | 4 Bytes   | Little Endian uint32_t, delay for triggering Keep Alive Check (in Seconds)
  AckDelay    | 4 Bytes   | Little Endian uint32_t, delay for allowing the app to Ack (in Seconds)
  Count       | 2 Bytes   | Little Endian uint16_t, Count parameter shall be set to 0 in config requests and will be ignored
  RFU         | 2 Bytes   | Reserved and Ignored (Set to 0)
  */

  TKA_RSP_CHECK       = 0x03,
  /**< The Server Shall send a @ref TKA_RSP_CHECK response when the CheckDelay timer expires. <br>
  At that time, the server restarts the AckDelay timer. <br>

  Format: there is no parameter.
  */

  TKA_RSP_ERROR_UNSUPPORTED = 0x10,
  /**< This error is send by the TKA Server when an unsupported TKA_CMD is received. <br>
  */
  
  TKA_RSP_ERROR_PARAMS      = 0x11,
  /**< This error is send by the TKA Server when a TKA_CMD with bad parameters is received. <br>
  */
};


/** @}*/


#define TKA_DEFAULT_CHECK_DELAY  17*60   ///< 17 minutes
#define TKA_DEFAULT_ACK_DELAY    20      ///< 20 seconds
#define TKA_DEFAULT_START_DELAY  5       ///< 5 seconds

void tka_process_command(const uint8_t cid, const uint8_t* data, uint8_t datalen);
void tka_init(void);
void tka_check_time(uint8_t index);
void tka_start_default(uint8_t cid);
    
#endif  // TKA_H_

