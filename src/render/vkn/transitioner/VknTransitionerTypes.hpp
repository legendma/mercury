#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


typedef void VKN_transitioner_barrier_proc_type
    (
    const u32           uid,        /* image unique id              */
    const u32           mip_levels, /* image number of mip levels   */
    const VkPipelineStageFlags
                        src_stages, /* source pipeline stages       */
    const VkPipelineStageFlags
                        dst_stages, /* destination pipeline stages  */
    const VkImageMemoryBarrier
                       *barrier,    /* resource barrier             */
    VkCommandBuffer     commands,   /* command buffer for barrier   */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

typedef void VKN_transitioner_begin_frame_proc_type
    (
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

typedef void VKN_transitioner_commit_proc_type
    (
    const VkCommandBuffer
                        commands,   /* on which to commit           */
    struct _VKN_transitioner_type
                       *master,     /* master resource transitioner */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

typedef void VKN_transitioner_register_new_proc_type
    (
    const u32           uid,        /* image uid                    */
    const VkImage       image,      /* image resource               */
    const VkImageLayout initial,    /* initial layout               */
    const u32           mip_levels, /* image number of mip levels   */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

typedef void VKN_transitioner_reset_proc_type
    (
    struct _VKN_transitioner_type
                       *transitioner/* transitioner to reset        */
    );

typedef void VKN_transitioner_unregister_proc_type
    (
    const u32           uid,        /* resource uid                 */
    struct _VKN_transitioner_type
                       *transitioner/* resource transitioner        */
    );

typedef struct _VKN_transitioner_api_type
    {
    VKN_transitioner_barrier_proc_type
                       *barrier;    /* create a resource barrier    */
    VKN_transitioner_begin_frame_proc_type
                       *begin_frame;/* begin a new frame            */
    VKN_transitioner_commit_proc_type
                       *commit;     /* commit pending state         */
    VKN_transitioner_register_new_proc_type
                       *register_new;
                                    /* register a new resource      */
    VKN_transitioner_unregister_proc_type
                       *unregister; /* unregister existing resource */
    } VKN_transitioner_api_type;
    
typedef struct _VKN_transitioner_subresource_type
    {
    VkImageLayout       state;      /* current state                */
    u32                 start_mip;  /* starting mip level           */
    u32                 mip_count;  /* mips in range                */
    struct _VKN_transitioner_subresource_type
                       *next;       /* next subresource             */
    } VKN_transitioner_subresource_type;

typedef struct _VKN_transitioner_resource_type
    {
    VKN_transitioner_subresource_type
                       *subresources;
                                    /* subresource layout tracking  */
    VkImageLayout       all_subresources;
                                    /* current all subresources     */
    u32                 mip_count;  /* number of mip levels         */
    u32                 uid;        /* resource unique id           */
    struct _VKN_transitioner_resource_type
                       *next;       /* next resource                */
    } VKN_transitioner_resource_type;

typedef struct _VKN_transitioner_barrier_type
    {
    u32                 uid;        /* resource unique id           */
    VkPipelineStageFlags
                        src_stages; /* source pipeline stages       */
    VkPipelineStageFlags
                        dst_stages; /* destination pipeline stages  */
    VkImageMemoryBarrier
                        barrier;    /* memory barrier               */
    struct _VKN_transitioner_barrier_type
                       *next;       /* next barrier                 */
    } VKN_transitioner_barrier_type;

typedef struct
    {
    u32                 uid;        /* resource unique id           */
    } VKN_transitioner_unregister_type;

typedef struct
    {
    VKN_transitioner_unregister_type
                       *arr;        /* array head                   */
    u32                 capacity;   /* array capacity               */
    u32                 count;      /* current count                */
    } VKN_transitioner_unregister_frame_type;

typedef struct
    {
    bool                is_master : 1;
                                    /* is this the master ledger?   */
    u32                 frame_num;  /* current frame index          */
    VKN_transitioner_subresource_type
                       *subresource_free;
                                    /* free subresource records     */
    VKN_transitioner_resource_type  /* free resource records        */
                       *resource_free;
    VKN_transitioner_barrier_type   /* free barrier records         */
                       *barrier_free;
    VKN_transitioner_resource_type
                       *resources;  /* used resource records        */
    VKN_transitioner_barrier_type
                       *prepends;   /* barriers to pre-pend         */
    VKN_transitioner_barrier_type   /* last prepended barrier       */
                       *prepends_tail;
    VKN_transitioner_unregister_frame_type
                        unregister[ VKN_FRAME_CNT ];
    } VKN_transitioner_state_type;

typedef struct _VKN_transitioner_type
    {
    const VKN_transitioner_api_type
                       *i;          /* public interface             */
    VKN_transitioner_state_type
                        state;      /* private state                */
    } VKN_transitioner_type;
