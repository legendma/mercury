#pragma once

#include "Global.hpp"

#include "VknArenaTypes.hpp"


/*********************************************************************
*
*   VKN_arena_allocate_array
*
*********************************************************************/

#define VKN_arena_allocate_array( _type, _count, _arena ) \
    (_type*)VKN_arena_allocate( (_count) * sizeof( _type ), _arena )


/*********************************************************************
*
*   VKN_arena_allocate_struct
*
*********************************************************************/

#define VKN_arena_allocate_struct( _type, _arena ) \
    (_type*)VKN_arena_allocate( sizeof( _type ), _arena )


/*********************************************************************
*
*   VKN_arena_mem_size
*
*********************************************************************/

#define VKN_arena_mem_size() \
    ( (u64)sizeof( *( (VKN_arena_type*)NULL )->memory ) )


/*********************************************************************
*
*   VKN_arena_round_size
*
*********************************************************************/

#define VKN_arena_round_size( _bytes ) \
    ( VKN_arena_mem_size() * ( ( (_bytes) + VKN_arena_mem_size() - 1 ) / VKN_arena_mem_size() ) )


void VKN_arena_align
    (
    const u16           align,      /* alignment in bytes           */
    VKN_arena_type     *arena       /* arena allocator              */
    );

void * VKN_arena_allocate
    (
    const u64           size,       /* sizeof of memory             */
    VKN_arena_type     *arena       /* arena allocator              */
    );

void VKN_arena_create
    (
    const u64          size,       /* sizeof of memory             */
    VKN_arena_word_type
                       *memory,     /* memory to manage             */
    VKN_arena_type     *out         /* new arena                    */
    );

VKN_arena_token_type VKN_arena_get_token
    (
    VKN_arena_type     *arena       /* arena allocator              */
    );

void VKN_arena_rewind
    (
    VKN_arena_type     *arena       /* arena allocator              */
    );

void VKN_arena_rewind_by_token
    (
    const VKN_arena_token_type
                        token,      /* rewind token                 */
    VKN_arena_type     *arena       /* arena allocator              */
    );
