#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"
#include "VknStagingTypes.hpp"


#define VKN_BUFFER_VERTEX_MAX_SHARING_FAMILY_CNT \
                                    ( 3 )
#define VKN_BUFFER_VERTEX_CONFIG_API \
                                    const struct _VKN_buffer_vertex_build_config_type *


typedef VKN_BUFFER_VERTEX_CONFIG_API VKN_buffer_vertex_build_add_sharing_family_proc_type
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_buffer_vertex_build_type
                       *builder     /* vertex buffer builder        */
    );

typedef VKN_BUFFER_VERTEX_CONFIG_API VKN_buffer_vertex_build_reset_proc_type
    (
    struct _VKN_buffer_vertex_build_type
                       *builder     /* vertex buffer builder        */
    );

typedef VKN_BUFFER_VERTEX_CONFIG_API VKN_buffer_vertex_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_buffer_vertex_build_type
                       *builder     /* vertex buffer builder        */
    );

typedef struct _VKN_buffer_vertex_build_config_type
    {
    VKN_buffer_vertex_build_add_sharing_family_proc_type
                       *add_sharing_family;
    VKN_buffer_vertex_build_reset_proc_type
                       *reset;
    VKN_buffer_vertex_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;

    } VKN_buffer_vertex_build_config_type;

typedef struct
    {
    u32                 indices[ VKN_BUFFER_VERTEX_MAX_SHARING_FAMILY_CNT ];
                                    /* family indices               */
    u32                 count;      /* number of indices            */
    } VKN_buffer_vertex_build_family_indices_type;

typedef struct
    {
    u8                  upload_alignment;
                                    /* required alignment for upload*/
    VKN_memory_type    *memory;     /* memory allocator             */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VKN_buffer_vertex_build_family_indices_type
                        families;   /* queue families               */
    } VKN_buffer_vertex_build_state_type;

typedef struct _VKN_buffer_vertex_build_type
    {
    const VKN_buffer_vertex_build_config_type
                       *config;
    VKN_buffer_vertex_build_state_type
                        state;
    } VKN_buffer_vertex_build_type;

typedef VkBuffer VKN_buffer_vertex_get_proc_type
    (
    const struct _VKN_buffer_vertex_type
                       *buffer      /* vertex buffer to fetch       */
    );

typedef void * VKN_buffer_vertex_map_proc_type
    (
    VKN_staging_type   *staging,    /* staging buffer               */
    struct _VKN_buffer_vertex_type
                       *buffer      /* vertex buffer to map         */
    );
    
typedef void VKN_buffer_vertex_unmap_proc_type
    (
    struct _VKN_buffer_vertex_type
                       *buffer      /* vertex buffer to unmap       */
    );

typedef struct
    {
    VKN_buffer_vertex_get_proc_type
                       *get;
    VKN_buffer_vertex_map_proc_type
                       *map;
    VKN_buffer_vertex_unmap_proc_type
                       *unmap;
    } VKN_buffer_vertex_api_type;

typedef struct
    {
    bool                is_mapped : 1;
    u8                  upload_alignment;
    u32                 size;
    VkBuffer            object;
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VKN_memory_type    *memory;
    VKN_memory_allocation_type
                        allocation;
    VKN_staging_upload_instruct_type
                        upload;
    } VKN_buffer_vertex_state_type;

typedef struct _VKN_buffer_vertex_type
    {
    const VKN_buffer_vertex_api_type
                       *i;
    VKN_buffer_vertex_state_type
                        state;
    } VKN_buffer_vertex_type;
