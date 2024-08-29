#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"


#define VKN_STAGING_MAX_SHARING_FAMILY_CNT \
                                    ( 3 )

#define VKN_STAGING_CONFIG_API      const struct _VKN_staging_build_config_type *


typedef VKN_STAGING_CONFIG_API VKN_staging_build_add_sharing_family_proc_type
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_staging_build_type
                       *builder     /* staging builder              */
    );

typedef VKN_STAGING_CONFIG_API VKN_staging_build_set_frame_size_proc_type
    (
    const u32           per_frame_sz,
                                    /* buffer size of each frame    */
    struct _VKN_staging_build_type
                       *builder     /* staging builder              */
    );

typedef struct _VKN_staging_build_config_type
    {
    VKN_staging_build_add_sharing_family_proc_type
                       *add_sharing_family;
                                    /* add family to share with     */
    VKN_staging_build_set_frame_size_proc_type
                       *set_frame_size;
                                    /* set frames' buffer size      */
    } VKN_staging_build_config_type;

typedef struct
    {
    u32                 indices[ VKN_STAGING_MAX_SHARING_FAMILY_CNT ];
                                    /* family indices               */
    u32                 count;      /* number of indices            */
    } VKN_staging_build_family_indices_type;

typedef struct
    {
    u32                 queue_index;/* family index of submit queue */
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    VKN_memory_type    *memory;     /* device memory allocator      */
    VkDevice            logical;    /* logical device               */
    VkQueue             queue;      /* queue on which to submit     */
    VkDeviceSize        buffer_frame_sz;
                                    /* size of each buffer frame    */
    VKN_staging_build_family_indices_type
                        families;   /* queue families               */
    } VKN_staging_build_state_type;

typedef struct _VKN_staging_build_type
    {
    VKN_staging_build_state_type
                        state;      /* builder state                */
    const VKN_staging_build_config_type
                       *config;     /* configuration interface      */
    } VKN_staging_build_type;

typedef struct
    {
    u32                 offset;     /* offset into the buffer       */
    char               *mapping;    /* where data should go         */
    VkBuffer            buffer;     /* buffer which to use          */
    VkCommandBuffer     commands;   /* command buffer to use        */
    } VKN_staging_upload_instruct_type;

typedef void VKN_staging_flush_proc_type
    (
    struct _VKN_staging_type
                       *staging     /* resource staging to flush   */
    );

typedef VKN_staging_upload_instruct_type VKN_staging_upload_proc_type
    (
    const u32           size,       /* size of the upload           */
    const u32           alignment,  /* alignment of the upload      */
    struct _VKN_staging_type
                       *staging     /* resource staging             */
    );

typedef struct
    {
    VKN_staging_flush_proc_type
                       *flush;      /* flush the upload queue       */
    VKN_staging_upload_proc_type
                       *upload;     /* upload a resource            */
    } VKN_staging_api_type;

typedef struct
    {
    bool                in_flight;  /* is the buffer in-flight?     */
    u32                 caret;      /* current buffer write start   */
    VkBuffer            buffer;     /* buffer object                */
    VkCommandBuffer     commands;   /* command buffer object        */
    VkFence             fence;      /* fence object                 */
    VKN_memory_allocation_type
                        allocation; /* device memory                */
    } VKN_staging_frame_type;

typedef struct
    {
    u32                 frame_num;  /* current frame number         */
    u32                 queue_index;/* family index of submit queue */
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    VKN_memory_type    *memory;     /* device memory allocator      */
    VkDeviceSize        buffer_frame_sz;
                                    /* size of each buffer frame    */
    VkDevice            logical;    /* logical device               */
    VkQueue             queue;      /* queue on which to submit     */
    VkCommandPool       command_pool;
                                    /* command buffer pool          */
    VKN_staging_frame_type          /* frame state                  */
                        frames[ VKN_FRAME_CNT ];
    } VKN_staging_state_type;

typedef struct _VKN_staging_type
    {
    const VKN_staging_api_type
                       *i;          /* resource staging interface   */
    VKN_staging_state_type
                        state;      /* resource staging state       */
    } VKN_staging_type;
