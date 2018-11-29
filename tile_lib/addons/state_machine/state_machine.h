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

/** @file state_machine.h
 ** @brief State machine management
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdint.h>

/** Size of state machine stacks */
#define SM_STACK_SIZE 10

/** Macro for declaring a state machine function */
#define STATE_MACHINE(_name) void _name(const void *_input)

/** Macro for assigning the void * input to a typed variable */
#define SM_ASSIGN_INPUT(_var) _var = _input

/** Macro for pairing two integers */
#define PAIR(a, b) (((a) << 16) | (b))

/** Function pointer to a state machine */
typedef void(*StateMachine)(const void *input);

/** Manager for a subsystem's state machine use */
struct sm_manager {
  StateMachine stack[SM_STACK_SIZE];
  uint8_t stackptr;
};

/** Macro for initializing the manager, e.g. static struct sm_manager = SM_MANAGER_INIT; */
#define SM_MANAGER_INIT {0};

void sm_start(struct sm_manager *manager, StateMachine sm, const void *args);
void sm_return(struct sm_manager *manager, const void *retval);
void emit(struct sm_manager *manager, const void *data);
void sm_clear(struct sm_manager *manager);

#endif
