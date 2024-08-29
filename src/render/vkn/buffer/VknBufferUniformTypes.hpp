#pragma once

#include "Global.hpp"

#include "VknArenaTypes.hpp"
#include "VknCommon.hpp"
#include "VknMemoryTypes.hpp"


#define VKN_BUFFER_UNIFORM_MAX_SHARING_FAMILY_CNT \
                                    ( 3 )
#define VKN_BUFFER_UNIFORM_CONFIG_API \
                                    const struct _VKN_buffer_uniform_build_config_type *


typedef VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_build_add_sharing_family_proc_type
    (
    const u32           index,      /* family index to share with   */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );

typedef VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_build_reset_proc_type
    (
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );

typedef VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );

typedef VKN_BUFFER_UNIFORM_CONFIG_API VKN_buffer_uniform_build_set_page_size_proc_type
    (
    const u32           page_size,  /* individual page size         */
    struct _VKN_buffer_uniform_build_type
                       *builder     /* uniform buffer builder       */
    );

typedef struct _VKN_buffer_uniform_build_config_type
    {
    VKN_buffer_uniform_build_add_sharing_family_proc_type
                       *add_sharing_family;
    VKN_buffer_uniform_build_reset_proc_type
                       *reset;
    VKN_buffer_uniform_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
    VKN_buffer_uniform_build_set_page_size_proc_type
                       *set_page_size;
    } VKN_buffer_uniform_build_config_type;

typedef struct
    {
    u32                 indices[ VKN_BUFFER_UNIFORM_MAX_SHARING_FAMILY_CNT ];
                                    /* family indices               */
    u32                 count;      /* number of indices            */
    } VKN_buffer_uniform_build_family_indices_type;

typedef struct
    {
    u16                 uniform_alignment;
                                    /* required uniform alignment   */
    u32                 max_page_size;
                                    /* physical device max page size*/
    u32                 page_size;  /* individual page size         */
    VKN_memory_type    *memory;     /* memory allocator             */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VKN_buffer_uniform_build_family_indices_type
                        families;   /* queue families               */
    } VKN_buffer_uniform_build_state_type;

typedef struct _VKN_buffer_uniform_build_type
    {
    const VKN_buffer_uniform_build_config_type
                       *config;
    VKN_buffer_uniform_build_state_type
                        state;
    } VKN_buffer_uniform_build_type;

typedef struct
    {
    u32                 offset;
    VkBuffer            buffer;
    } VKN_buffer_uniform_map_result_type;
    
typedef void VKN_buffer_uniform_clear_proc_type
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to clear      */
    );
    
typedef void VKN_buffer_uniform_copy_proc_type
    (
    const float        *data,       /* data to copy                 */
    const u8            num_floats, /* number of floats in data     */
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to copy in    */
    );

typedef void VKN_buffer_uniform_flush_proc_type
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to flush      */
    );

typedef VKN_buffer_uniform_map_result_type VKN_buffer_uniform_map_proc_type
    (
    const u32           size,       /* size of memory to map        */
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to map        */
    );
    
typedef u32 VKN_buffer_uniform_unmap_proc_type
    (
    struct _VKN_buffer_uniform_type
                       *buffer      /* uniform buffer to unmap      */
    );

typedef struct
    {
    VKN_buffer_uniform_clear_proc_type
                       *clear;
    VKN_buffer_uniform_copy_proc_type
                       *copy;
    VKN_buffer_uniform_flush_proc_type
                       *flush;
    VKN_buffer_uniform_map_proc_type
                       *map;
    VKN_buffer_uniform_unmap_proc_type
                       *unmap;
    } VKN_buffer_uniform_api_type;

typedef struct _VKN_buffer_uniform_page_type
    {
    u32                 caret;
    u32                 dirty_size;
    VkBuffer            buffer;
    VKN_memory_allocation_type
                        allocation;
    struct _VKN_buffer_uniform_page_type
                       *next;
    } VKN_buffer_uniform_page_type;

typedef VKN_buffer_uniform_build_family_indices_type VKN_buffer_uniform_family_indices_type;

typedef struct
    {
    bool                is_mapped : 1;
    bool                is_dirty : 1;
    u16                 uniform_alignment;
    u32                 page_size;
    u32                 uniform_size;
    VKN_arena_type     *arena;
    VKN_buffer_uniform_page_type
                       *frees;
    VKN_buffer_uniform_page_type
                       *dirtys;
    VKN_buffer_uniform_page_type
                       *current;
    const VkAllocationCallbacks
                       *allocator;
    VKN_memory_type    *memory;
    VkDevice            logical;
    VKN_buffer_uniform_family_indices_type
                        families;
    } VKN_buffer_uniform_state_type;

typedef struct _VKN_buffer_uniform_type
    {
    const VKN_buffer_uniform_api_type
                       *i;
    VKN_buffer_uniform_state_type
                        state;
    } VKN_buffer_uniform_type;
