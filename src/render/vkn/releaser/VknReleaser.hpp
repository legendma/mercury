#pragma once

#include "Global.hpp"

#include "VknReleaserTypes.hpp"


void VKN_releaser_create
    (
    const u32           capacity,   /* number of variants in pool   */
    VKN_releaser_variant_type
                       *pool,       /* variant pool                 */
    VKN_releaser_type  *releaser    /* output new releaser          */
    );

void VKN_releaser_destroy
    (
    VKN_releaser_type  *releaser    /* releaser to destroy          */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_auto_mini_begin
*
*********************************************************************/

#define VKN_releaser_auto_mini_begin( _input, _usage )              \
    VKN_releaser_mini_type  __mini;                                 \
    VKN_releaser_type      *_usage;                                 \
    (_usage) = (_input);                                            \
    if( !(_usage) )                                                 \
        {                                                           \
        VKN_releaser_mini_create( &__mini );                        \
        (_usage) = &__mini.releaser;                                \
        }


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_auto_mini_end
*
*********************************************************************/

#define VKN_releaser_auto_mini_end( _usage )                        \
    if( (_usage) == &__mini.releaser )                              \
        {                                                           \
        VKN_releaser_mini_destroy( &__mini );                       \
        }


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_mini_create
*
*********************************************************************/

static __inline void VKN_releaser_mini_create
    (
    VKN_releaser_mini_type
                       *mini        /* miniture releaser            */
    )
{
VKN_releaser_create( cnt_of_array( mini->vars ), mini->vars, &mini->releaser );

}   /* VKN_releaser_mini_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_releaser_mini_destroy
*
*********************************************************************/

static __inline void VKN_releaser_mini_destroy
    (
    VKN_releaser_mini_type
                       *mini        /* miniture releaser            */
    )
{
VKN_releaser_destroy( &mini->releaser );

}   /* VKN_releaser_mini_destroy() */
