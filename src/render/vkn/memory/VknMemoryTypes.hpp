#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_MEMORY_MAX_BLOCK_CNT    ( 2000 )
#define VKN_MEMORY_MAX_POOL_CNT     ( 20 )

#define VKN_MEMORY_CONFIG_API       const struct _VKN_memory_build_config_type *

typedef enum
    {
    VKN_MEMORY_HEAP_USAGE_DEFAULT,  /* not visible by CPU           */
    VKN_MEMORY_HEAP_USAGE_UPLOAD,   /* CPU write, GPU read          */
    VKN_MEMORY_HEAP_USAGE_READBACK, /* GPU write, CPU read          */
    /* count */
    VKN_MEMORY_HEAP_USAGE_CNT
    } VKN_memory_heap_usage_type;


typedef VKN_MEMORY_CONFIG_API VKN_memory_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_memory_build_type
                       *builder     /* memory allocator builder     */
    );

typedef struct _VKN_memory_build_config_type
    {
    VKN_memory_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    } VKN_memory_build_config_type;

typedef struct
    {
    VkInstance          instance;   /* associated Vulkan instance   */
    VkPhysicalDevice    physical;   /* associated physical device   */
    VkDevice            logical;    /* associated logical device    */
    VkPhysicalDeviceProperties
                        props;      /* device properties            */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    } VKN_memory_build_state_type;

typedef struct _VKN_memory_build_type
    {
    VKN_memory_build_state_type
                        state;      /* builder state                */
    const VKN_memory_build_config_type
                       *config;     /* configuration interface      */
    } VKN_memory_build_type;

typedef struct _VKN_memory_allocation_type
    {
    u32                 pool_id;    /* pool to which this belongs   */
    u32                 block_id;   /* block to which this belongs  */
    u32                 size;       /* allocation size              */
    VkDeviceSize        offset;     /* memory offset                */
    char               *mapping;    /* host mapping                 */
    struct _VKN_memory_allocation_type
                       *next;       /* deferred destruction         */
    VkDeviceMemory      memory;     /* memory allocation            */
    } VKN_memory_allocation_type;

typedef bool VKN_memory_allocate_proc_type
    (
    const u32           size,       /* required allocation size     */
    const u32           alignment,  /* required allocation alignment*/
    const u32           memory_bits,/* memory type indices as bits  */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new allocation        */
    );

typedef void VKN_memory_begin_frame_proc_type
    (
    struct _VKN_memory_type
                       *allocator   /* memory allocator             */
    );

typedef bool VKN_memory_create_buffer_memory_proc_type
    (
    const VkBuffer      buffer,     /* for which to create memory   */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new memory allocation */
    );

typedef bool VKN_memory_create_image_memory_proc_type
    (
    const VkImage       image,      /* for which to create memory   */
    const VKN_memory_heap_usage_type
                        usage,      /* how memory is to be used     */
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* output new memory allocation */
    );

typedef void VKN_memory_deallocate_proc_type
    (
    struct _VKN_memory_type
                       *allocator,  /* memory allocator             */
    VKN_memory_allocation_type
                       *allocation  /* allocation to deallocate     */
    );

typedef struct
    {
    VKN_memory_allocate_proc_type
                       *allocate;   /* allocate some memory         */
    VKN_memory_begin_frame_proc_type
                       *begin_frame;/* begin a new frame            */
    VKN_memory_create_buffer_memory_proc_type
                       *create_buffer_memory;
                                    /* back buffer w/ device memory */
    VKN_memory_create_image_memory_proc_type
                       *create_image_memory;
                                    /* back image w/ device memory  */
    VKN_memory_deallocate_proc_type
                       *deallocate; /* free an allocation           */
    } VKN_memory_api_type;

typedef struct _VKN_memory_block_type
    {
    bool                is_utilized : 1;
                                    /* is this block being used?    */
    bool                is_being_freed : 1;
                                    /* is block being freed?        */
    u32                 block_id;   /* unique block identifier      */
    u32                 pool_id;    /* owning pool identifier       */
    struct _VKN_memory_block_type
                       *prev;       /* previous block               */
    struct _VKN_memory_block_type
                       *next;       /* next block                   */
    struct _VKN_memory_block_type
                       *next_destroy;
                                    /* destruction ordered by offset*/
    VkDeviceSize        offset;     /* memory offset                */
    VkDeviceSize        size;       /* memory size                  */
    } VKN_memory_block_type;

typedef struct _VKN_memory_pool_type
    {
    u32                 pool_id;    /* unique pool identifier       */
    u32                 memory_index;
                                    /* memory type index            */
    struct _VKN_memory_pool_type
                       *next;       /* next pool                    */
    char               *mapping;    /* host mapping                 */
    VKN_memory_block_type
                       *head_blocks;/* used blocks                  */
    VKN_memory_heap_usage_type
                        usage;      /* how this pool is to be used  */
    VkDeviceMemory      memory;     /* memory allocation            */
    VkDeviceSize        size;       /* memory size                  */
    VkDeviceSize        min_alignment;
                                    /* minimum alignment for pool   */
    VkDeviceSize        min_block_size;
                                    /* minimum block size allowed   */
    } VKN_memory_pool_type;

typedef struct
    {
    bool                is_discrete : 1;
                                    /* GPU is discrete?             */
    u8                  frame_index;/* current frame index          */
    u32                 next_pool_id;
                                    /* id of the next pool          */
    u32                 next_block_id;
                                    /* id of the next block         */
    VkDevice            logical;    /* logical device               */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VKN_memory_pool_type
                       *free_pools; /* unused pools                 */
    VKN_memory_pool_type
                       *head_pools; /* used pools                   */
    VKN_memory_block_type
                       *free_blocks;/* unused blocks                */
    VKN_memory_block_type
                       *to_destroy[ VKN_FRAME_CNT ];
                                    /* deferred destruction         */
    VkDeviceSize        heap_remaining[ VK_MAX_MEMORY_HEAPS ];
                                    /* remaining heap sizes         */
    VkDeviceSize        heap_pool_size[ VK_MAX_MEMORY_HEAPS ];
                                    /* pool size for each heap      */
    VkDeviceSize        noncoherent_atom_size;
                                    /* smallest block size          */
    VKN_memory_block_type
                        blocks[ VKN_MEMORY_MAX_BLOCK_CNT ];
                                    /* block storage                */
    VKN_memory_pool_type
                        pools[ VKN_MEMORY_MAX_POOL_CNT ];
                                    /* pool storage                 */
    VkPhysicalDeviceMemoryProperties/* device memory properties     */
                        memory_props;
    } VKN_memory_state_type;

typedef struct _VKN_memory_type
    {
    const VKN_memory_api_type
                       *i;          /* allocator interface          */
    VKN_memory_state_type
                        state;      /* private state                */
    } VKN_memory_type;