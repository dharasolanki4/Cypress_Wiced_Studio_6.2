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

#ifndef TILE_HASH_H
#define TILE_HASH_H

void tile_device_auth_hash(const uint8_t* auth_key, const uint8_t* randA, const uint8_t* randT, uint8_t* sresT, uint8_t* aco);
void tile_gen_session_key(const uint8_t *auth_key, const uint8_t *randA, const uint8_t *randT,
  const uint8_t cid, const uint8_t *token, uint8_t *session_key);
void tile_gen_broadcast_key(const uint8_t *auth_key, const uint8_t *randT, const uint8_t *tile_id, uint8_t *broadcast_key);
void tile_session_key_gen_hash(const uint8_t* auth_key, const uint8_t* aco, const uint8_t* tileID, uint8_t* session_key);
void tile_mic_hash(const uint8_t* session_key, const uint32_t nonce, const uint8_t direction, const uint8_t* plaintext, const uint8_t text_length, uint8_t* mic);
void tile_broadcast_mic_hash(const uint8_t* session_key, const uint32_t nonce, const uint8_t* plaintext, const uint8_t text_length, uint8_t* mic);
int tile_sign_verify(const uint8_t* key, const uint8_t* hash, const uint8_t* signature);

#ifdef TEST_HMAC

int test_tile_hash(void);

#endif

#endif    // TILE_HASH_H

