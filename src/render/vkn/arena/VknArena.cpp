#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknArena.hpp"
#include "VknArenaTypes.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       min_token
*
*********************************************************************/

static __inline VKN_arena_token_type min_token
    (
    const VKN_arena_token_type
                        a,          /* first token                  */
    const VKN_arena_token_type
                        b           /* second token                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_arena_token_type    ret;        /* return min token             */

ret = a;
if( b < a )
    {
    ret = b;
    }

return( b );

}   /* min_token() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_align
*
*   DESCRIPTION:
*       Make a small allocation to align the arena to the requested
*       number of bytes.
*
*********************************************************************/

void VKN_arena_align
    (
    const u16           align,      /* alignment in bytes           */
    VKN_arena_type     *arena       /* arena allocator              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
uptr                    address;    /* address in integer form      */

address = (uptr)&arena->memory[ arena->caret ];
if( address % align )
    {
    arena->caret += ( align - address % align ) / VKN_arena_mem_size();
    }

}   /* VKN_arena_align() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_allocate
*
*   DESCRIPTION:
*       Allocate from an arena.
*
*********************************************************************/

void * VKN_arena_allocate
    (
    const u64           size,       /* byte size of allocation      */
    VKN_arena_type     *arena       /* arena allocator              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_arena_token_type    new_token;  /* new position token           */
void                   *ret;        /* return memory                */

new_token = arena->caret + (VKN_arena_token_type)( VKN_arena_round_size( size ) / VKN_arena_mem_size() );
if( new_token > arena->size / sizeof( *arena->memory ) )
    {
    return( NULL );
    }

ret = &arena->memory[ arena->caret ];
arena->caret = new_token;

return( ret );

}   /* VKN_arena_allocate() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_create
*
*   DESCRIPTION:
*       Create a memory arena.
*
*********************************************************************/

void VKN_arena_create
    (
    const u64           size,       /* sizeof of memory             */
    VKN_arena_word_type
                       *memory,     /* memory to manage             */
    VKN_arena_type     *out         /* new arena                    */
    )
{
clr_struct( out );
out->size   = size;
out->memory = memory;

memset( out->memory, 0, size );

}   /* VKN_arena_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_get_token
*
*   DESCRIPTION:
*       Obtain a rewind token.
*
*********************************************************************/

VKN_arena_token_type VKN_arena_get_token
    (
    VKN_arena_type     *arena       /* arena allocator              */
    )
{
return( arena->caret );

}   /* VKN_arena_get_token() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_rewind_by_token
*
*   DESCRIPTION:
*       Rewind to position of token.
*
*********************************************************************/

void VKN_arena_rewind
    (
    VKN_arena_type     *arena       /* arena allocator              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u64                     rewind_sz;  /* amount being rewound         */

rewind_sz = sizeof( *arena->memory ) * arena->caret;
memset( arena->memory, 0, rewind_sz );

arena->caret = 0;

}   /* VKN_arena_rewind_by_token() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_arena_rewind_by_token
*
*   DESCRIPTION:
*       Rewind to position of token.
*
*********************************************************************/

void VKN_arena_rewind_by_token
    (
    const VKN_arena_token_type
                        token,      /* rewind token                 */
    VKN_arena_type     *arena       /* arena allocator              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_arena_token_type    new_token;  /* new position token           */
u64                     rewind_sz;  /* amount being rewound         */

debug_assert( token <= arena->caret );
new_token = min_token( arena->caret, token );
rewind_sz = sizeof( *arena->memory ), ( arena->caret - new_token );
memset( &arena->memory[ new_token ], 0, rewind_sz );

arena->caret = new_token;

}   /* VKN_arena_rewind_by_token() */
