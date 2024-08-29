#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_PIPELINE_GRAPHICS_CONFIG_API \
                                    const struct _VKN_pipeline_graphics_build_config_type *

#define VKN_PIPELINE_GRAPHICS_MAX_DYNAMIC_STATE_CNT \
                                    ( 25 )
#define VKN_PIPELINE_GRAPHICS_MAX_VERTEX_BINDINGS_CNT \
                                    ( 2 )
#define VKN_PIPELINE_GRAPHICS_MAX_VERTEX_ATTRIBUTES_CNT \
                                    ( 14 )


typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_add_dynamic_state_proc_type
    (
    const VkDynamicState
                        state,      /* state to make dynamic        */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_add_stage_proc_type
    (
    const VkShaderStageFlags
                        stage,      /* pipeline stage               */
    const VkShaderModule
                        shader,     /* shader module                */
    const VkPipelineShaderStageCreateFlags
                        flags,      /* stage flags                  */
    const char         *entry_point,/* shader entry point           */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_add_vertex_attribute_proc_type
    (
    const u32           location,   /* attribute location           */
    const u32           binding,    /* binding to which this applies*/
    const VkFormat      format,     /* data type                    */
    const u32           offset,     /* byte offset to start         */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_add_vertex_binding_proc_type
    (
    const u32           binding,    /* binding index                */
    const u32           stride,     /* between consecutive elements */
    const bool          as_instance,/* rate is per instance?        */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_enable_dynamic_rendering_proc_type
    (
    const bool          enable,     /* enable dynamic rendering?    */
    const VkFormat      color_format,
                                    /* color attachment format      */
    const VkFormat      depth_format,
                                    /* depth attachment format      */
    const VkFormat      stencil_format,
                                    /* stencil attachment format    */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_build_reset_proc_type
    (
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_build_set_pipeline_cache_proc_type
    (
    const VkPipelineCache
                        cache,      /* pipeline cache               */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    );

typedef struct _VKN_pipeline_graphics_build_config_type
    {
    VKN_pipeline_graphics_build_add_dynamic_state_proc_type
                       *add_dynamic_state;
                                    /* register dynamic state       */
    VKN_pipeline_graphics_build_add_stage_proc_type
                       *add_stage;  /* add pipeline stage           */
    VKN_pipeline_graphics_build_add_vertex_attribute_proc_type
                       *add_vertex_attribute;
                                    /* add attribute to vert binding*/
    VKN_pipeline_graphics_build_add_vertex_binding_proc_type
                       *add_vertex_binding;
                                    /* add a vertex binding         */
    VKN_pipeline_graphics_build_enable_dynamic_rendering_proc_type
                       *enable_dynamic_rendering;
                                    /* configure dynamic rendering  */
    VKN_pipeline_graphics_build_reset_proc_type
                       *reset;      /* reset the builder state      */
    VKN_pipeline_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_pipeline_build_set_pipeline_cache_proc_type
                       *set_pipeline_cache;
                                    /* set a cache to store pipeline*/
    } VKN_pipeline_graphics_build_config_type;

typedef struct
    {
    u32                 count;      /* number of dynamic states     */
    VkDynamicState      states[ VKN_PIPELINE_GRAPHICS_MAX_DYNAMIC_STATE_CNT ];
                                    /* dynamic states               */
    } VKN_pipeline_graphics_build_dynamic_states_type;

typedef struct
    {
    u32                 count;      /* number of vertex attributes  */
    VkVertexInputAttributeDescription
                        attributes[ VKN_PIPELINE_GRAPHICS_MAX_VERTEX_ATTRIBUTES_CNT ];
                                    /* vertex attributes            */
    } VKN_pipeline_graphics_build_vertex_attributes_type;

typedef struct
    {
    u32                 count;      /* number of vertex bindings    */
    VkVertexInputBindingDescription /* vertex bindings              */
                        bindings[ VKN_PIPELINE_GRAPHICS_MAX_VERTEX_BINDINGS_CNT ];
    } VKN_pipeline_graphics_build_vertex_bindings_type;

typedef struct
    {
    u32                 count;      /* number of graphics stages    */
    VkPipelineShaderStageCreateInfo /* graphics stages              */
                        stages[ VKN_SHADER_GFX_STAGE_CNT ];
    } VKN_pipeline_graphics_build_stages_type;

typedef struct
    {
    bool                use_dynamic_rendering;
                                    /* attach required info?        */
    VkFormat            color_format;
    VkFormat            depth_format;
    VkFormat            stencil_format;
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VkPipelineCache     cache;      /* cache to store pipeline      */
    VKN_pipeline_graphics_build_stages_type
                        stages;     /* stages for this pipeline     */
    VKN_pipeline_graphics_build_dynamic_states_type
                        dynamic_states;
                                    /* dynamic states for pipeline  */
    VKN_pipeline_graphics_build_vertex_bindings_type
                        vertex_bindings;
                                    /* vertex binding descriptions  */
    VKN_pipeline_graphics_build_vertex_attributes_type
                        vertex_attributes;
                                    /* vertex attribute descriptions*/
    } VKN_pipeline_graphics_build_state_type;

typedef struct _VKN_pipeline_graphics_build_type
    {
    VKN_pipeline_graphics_build_state_type
                        state;      /* builder state                */
    const VKN_pipeline_graphics_build_config_type
                       *config;     /* configuration interface      */
    } VKN_pipeline_graphics_build_type;

typedef struct
    {
    VKN_render_flags_type           /* render flags given at create */
                        render_flags;
    VKN_render_flags_type           /* dynamic state mask at create */
                        render_flags_mask;
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VkPipeline          pipeline;   /* pipeline handle              */
    } VKN_pipeline_graphics_type;
