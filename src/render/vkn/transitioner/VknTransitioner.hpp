#pragma once

#include "Global.hpp"

#include "VknTransitionerTypes.hpp"

#define VKN_TRANSITIONER_IS_MASTER  ( true )
#define VKN_TRANSITIONER_IS_INSTANCE \
                                    ( false )


void VKN_transitioner_create
    (
    const bool          is_master,  /* is this the master ledger?   */
    const u32           barrier_capacity,
                                    /* max barriers                 */
    const u32           resource_capacity,
                                    /* max resources                */
    const u32           subresource_capacity,
                                    /* max subresources             */
    const u32           unregister_capacity,
                                    /* max deferred unregisters     */
    VKN_transitioner_barrier_type
                       *barriers,   /* barrier records              */
    VKN_transitioner_resource_type
                       *resources,  /* resource records             */
    VKN_transitioner_subresource_type
                       *subresources,
                                    /* subresource records          */
    VKN_transitioner_unregister_type
                       *unregisters,/* unregister records           */
    VKN_transitioner_type
                       *transitioner/* new resource transitioner    */
    );
