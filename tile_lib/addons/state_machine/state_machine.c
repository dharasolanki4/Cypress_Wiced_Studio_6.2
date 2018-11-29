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

/** @file state_machine.c
 ** @brief State machine management
 */

#include "state_machine/state_machine.h"

#include <stdint.h>


/**
 * Launch a state machine on manager.
 *
 * @param[in] manager  Manager to use to launch this state machine
 * @param[in] sm       State machine to launch
 * @param[in] args     Arguments to pass to the state machine
 */
void sm_start(struct sm_manager *manager, StateMachine sm, const void *args)
{
  if(manager->stackptr >= SM_STACK_SIZE)
  {
    return;
  }

  /* Push new state machine onto the manager's stack */
  manager->stack[manager->stackptr++] = sm;
  emit(manager, args);
}

/**
 * Return to calling state machine, if one exists
 *
 * @param[in] manager  Manager to manage the return
 * @param[in] retval   Value to be returned to calling machine
 */
void sm_return(struct sm_manager *manager, const void *retval)
{
  if(manager->stackptr < 1)
  {
    return;
  }

  /* Pop from the stack */
  manager->stack[--manager->stackptr] = 0;
  emit(manager, retval);
}

/**
 * Send an event to the currently running state machine on manager
 *
 * @param[in] manager  The manager to emit the event on
 * @param[in] data     Data payload to be sent
 */
void emit(struct sm_manager *manager, const void *data)
{
  if(0 == manager->stackptr)
  {
    return;
  }

  manager->stack[manager->stackptr-1](data);
}

/**
 * Stop all state machines on manager
 *
 * @param[in] manager  Manager to clear state machines on
 */
void sm_clear(struct sm_manager *manager)
{
  manager->stackptr = 0;
}
