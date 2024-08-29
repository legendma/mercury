#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_RELEASER_MINI_VAR_CNT   ( 20 )
#define VKN_RELEASER_API            const struct _VKN_releaser_api_type *


typedef VKN_RELEASER_API VKN_releaser_flush_proc_type
    (
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_buffer_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkBuffer      buffer,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_command_buffer_proc_type
    (
    const VkDevice      logical,
    const VkCommandPool pool,
    const VkCommandBuffer
                        buffer,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_command_pool_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkCommandPool pool,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_debug_messenger_proc_type
    (
    const VkInstance    instance,
    const VkAllocationCallbacks
                       *allocator,
    const VkDebugUtilsMessengerEXT
                        messenger,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_descriptor_pool_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDescriptorPool
                        pool,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_descriptor_set_layout_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDescriptorSetLayout
                        layout,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_device_proc_type
    (
    const VkAllocationCallbacks
                       *allocator,
    const VkDevice      device,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_fence_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkFence       fence,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_image_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkImage       image,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_image_view_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkImageView   view,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_instance_proc_type
    (
    const VkAllocationCallbacks
                       *allocator,
    const VkInstance    instance,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_memory_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkDeviceMemory
                        memory,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_pipeline_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkPipeline    pipeline,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_pipeline_layout_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkPipelineLayout
                        layout,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_sampler_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSampler     sampler,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_semaphore_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSemaphore   semaphore,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_shader_module_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkShaderModule
                        shader,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_surface_proc_type
    (
    const VkInstance    instance,
    const VkAllocationCallbacks
                       *allocator,
    const VkSurfaceKHR  surface,
    struct _VKN_releaser_type
                       *releaser
    );

typedef VKN_RELEASER_API VKN_releaser_release_swap_chain_proc_type
    (
    const VkDevice      logical,
    const VkAllocationCallbacks
                       *allocator,
    const VkSwapchainKHR
                        swap_chain,
    struct _VKN_releaser_type
                       *releaser
    );

typedef struct _VKN_releaser_api_type
    {
    VKN_releaser_flush_proc_type
                       *flush;
    VKN_releaser_release_buffer_proc_type
                       *release_buffer;
    VKN_releaser_release_command_buffer_proc_type
                       *release_command_buffer;
    VKN_releaser_release_command_pool_proc_type
                       *release_command_pool;
    VKN_releaser_release_debug_messenger_proc_type
                       *release_debug_messenger;
    VKN_releaser_release_descriptor_pool_proc_type
                       *release_descriptor_pool;
    VKN_releaser_release_descriptor_set_layout_proc_type
                       *release_descriptor_set_layout;
    VKN_releaser_release_device_proc_type
                       *release_device;
    VKN_releaser_release_fence_proc_type
                       *release_fence;
    VKN_releaser_release_image_proc_type
                       *release_image;
    VKN_releaser_release_image_view_proc_type
                       *release_image_view;
    VKN_releaser_release_instance_proc_type
                       *release_instance;
    VKN_releaser_release_memory_proc_type
                       *release_memory;
    VKN_releaser_release_pipeline_proc_type
                       *release_pipeline;
    VKN_releaser_release_pipeline_layout_proc_type
                       *release_pipeline_layout;
    VKN_releaser_release_sampler_proc_type
                       *release_sampler;
    VKN_releaser_release_semaphore_proc_type
                       *release_semaphore;
    VKN_releaser_release_shader_module_proc_type
                       *release_shader_module;
    VKN_releaser_release_surface_proc_type
                       *release_surface;
    VKN_releaser_release_swap_chain_proc_type
                       *release_swap_chain;
    } VKN_releaser_api_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkBuffer            buffer;
    } VKN_releaser_args_buffer_type;    

typedef struct
    {
    VkDevice            logical;
    VkCommandPool       pool;
    VkCommandBuffer     buffer;
    } VKN_releaser_args_command_buffer_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkCommandPool       pool;
    } VKN_releaser_args_command_pool_type;

typedef struct
    {
    VkInstance          instance;
    const VkAllocationCallbacks
                       *allocator;
    VkDebugUtilsMessengerEXT
                        messenger;
    } VKN_releaser_args_debug_messenger_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkDescriptorPool    pool;
    } VKN_releaser_args_descriptor_pool_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkDescriptorSetLayout
                        layout;
    } VKN_releaser_args_descriptor_set_layout_type;

typedef struct
    {
    const VkAllocationCallbacks
                       *allocator;
    VkDevice            device;
    } VKN_releaser_args_device_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkFence             fence;
    } VKN_releaser_args_fence_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkImage             image;
    } VKN_releaser_args_image_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkImageView         view;
    } VKN_releaser_args_image_view_type;

typedef struct
    {
    const VkAllocationCallbacks
                       *allocator;
    VkInstance          instance;
    } VKN_releaser_args_instance_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkDeviceMemory      memory;
    } VKN_releaser_args_memory_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkPipeline          pipeline;
    } VKN_releaser_args_pipeline_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkPipelineLayout    layout;
    } VKN_releaser_args_pipeline_layout_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkSampler           sampler;
    } VKN_releaser_args_sampler_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkSemaphore         semaphore;
    } VKN_releaser_args_semaphore_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkShaderModule      shader;
    } VKN_releaser_args_shader_module_type;

typedef struct
    {
    VkInstance          instance;
    const VkAllocationCallbacks
                       *allocator;
    VkSurfaceKHR        surface;
    } VKN_releaser_args_surface_type;

typedef struct
    {
    VkDevice            logical;
    const VkAllocationCallbacks
                       *allocator;
    VkSwapchainKHR      swap_chain;
    } VKN_releaser_args_swap_chain_type;

typedef union
    {
    VKN_releaser_args_buffer_type
                        buffer;
    VKN_releaser_args_command_buffer_type
                        command_buffer;
    VKN_releaser_args_command_pool_type
                        command_pool;
    VKN_releaser_args_debug_messenger_type
                        debug_messenger;
    VKN_releaser_args_descriptor_pool_type
                        descriptor_pool;
    VKN_releaser_args_descriptor_set_layout_type
                        descriptor_set_layout;
    VKN_releaser_args_device_type
                        device;
    VKN_releaser_args_fence_type
                        fence;
    VKN_releaser_args_image_type
                        image;
    VKN_releaser_args_image_view_type
                        image_view;
    VKN_releaser_args_instance_type
                        instance;
    VKN_releaser_args_memory_type
                        memory;
    VKN_releaser_args_pipeline_type
                        pipeline;
    VKN_releaser_args_pipeline_layout_type
                        pipeline_layout;
    VKN_releaser_args_sampler_type
                        sampler;
    VKN_releaser_args_semaphore_type
                        semaphore;
    VKN_releaser_args_shader_module_type
                        shader_module;
    VKN_releaser_args_surface_type
                        surface;
    VKN_releaser_args_swap_chain_type
                        swap_chain;
    } VKN_releaser_args_all_type;

typedef struct
    {
    VkObjectType        tag;
    VKN_releaser_args_all_type
                        args;
    } VKN_releaser_variant_type;

typedef struct _VKN_releaser_type
    {
    VKN_releaser_variant_type
                       *vars;
    u32                 capacity;
    u32                 count;
    const VKN_releaser_api_type
                       *i;
    } VKN_releaser_type;

typedef struct
    {
    VKN_releaser_variant_type       /* variants                     */
                        vars[ VKN_RELEASER_MINI_VAR_CNT ];
    VKN_releaser_type   releaser;   /* releaser                     */
    } VKN_releaser_mini_type;