#pragma once

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknPipelineGraphics.hpp"
#include "VknPipelineGraphicsTypes.hpp"
#include "VknReleaser.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       has_dynamic
*
*********************************************************************/

static __inline bool has_dynamic
    (
    const VkDynamicState
                        state,      /* state to make dynamic        */
    const VKN_pipeline_graphics_build_dynamic_states_type
                       *dynamics    /* dynamic states               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < dynamics->count; i++ )
    {
    if( dynamics->states[ i ] == state )
        {
        return( TRUE );
        }
    }

return( FALSE );

}   /* has_dynamic() */


static VKN_pipeline_graphics_build_add_dynamic_state_proc_type add_dynamic_state;
static VKN_pipeline_graphics_build_add_stage_proc_type add_stage;
static VKN_pipeline_graphics_build_add_vertex_attribute_proc_type add_vertex_attribute;
static VKN_pipeline_graphics_build_add_vertex_binding_proc_type add_vertex_binding;
static VKN_pipeline_graphics_build_enable_dynamic_rendering_proc_type enable_dynamic_rendering;

static VKN_render_flags_type get_render_flags_mask
    (
    const VKN_pipeline_graphics_build_dynamic_states_type
                       *dynamics    /* dynamic states               */
    );

static VKN_pipeline_graphics_build_reset_proc_type reset;
static VKN_pipeline_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_pipeline_build_set_pipeline_cache_proc_type set_pipeline_cache;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_pipeline_graphics_create
*
*   DESCRIPTION:
*       Create a graphics pipeline via the given builder.
*
*********************************************************************/

bool VKN_pipeline_graphics_create
    (
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    const VkPipelineLayout
                        layout,     /* pipeline layout              */
    const VKN_pipeline_graphics_build_type
                       *builder,    /* graphics pipeline builder    */
    VKN_pipeline_graphics_type
                       *pipeline    /* output new graphics pipeline */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPipelineColorBlendAttachmentState /* first color attach blending  */
                        blend_attach0;
VkPipelineColorBlendStateCreateInfo
                        ci_blend;   /* blending create info         */
VkPipelineDepthStencilStateCreateInfo
                        ci_depth_stencil;
                                    /* depth/stencil create info    */
VkPipelineDynamicStateCreateInfo
                        ci_dynamic; /* dynamic state create info    */
VkPipelineInputAssemblyStateCreateInfo 
                        ci_input;   /* input assembly create info   */
VkPipelineMultisampleStateCreateInfo
                        ci_msaa;    /* multi-sampling create info   */
VkGraphicsPipelineCreateInfo
                        ci_pipeline;/* pipeline create info         */
VkPipelineRasterizationStateCreateInfo
                        ci_raster;  /* rasterization create info    */
VkPipelineRenderingCreateInfo
                        ci_render;  /* render target info           */
VkPipelineVertexInputStateCreateInfo
                        ci_vertex;  /* vertex input create info     */
VkPipelineViewportStateCreateInfo 
                        ci_viewport;/* viewport state create info   */

/*----------------------------------------------------------
Create the pipeline
----------------------------------------------------------*/
clr_struct( pipeline );
pipeline->logical   = builder->state.logical;
pipeline->allocator = builder->state.allocator;

/*----------------------------------------------------------
Vertex input
----------------------------------------------------------*/
clr_struct( &ci_vertex );
ci_vertex.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
ci_vertex.pVertexAttributeDescriptions    = builder->state.vertex_attributes.attributes;
ci_vertex.vertexAttributeDescriptionCount = builder->state.vertex_attributes.count;
ci_vertex.pVertexBindingDescriptions      = builder->state.vertex_bindings.bindings;
ci_vertex.vertexBindingDescriptionCount   = builder->state.vertex_bindings.count;

/*----------------------------------------------------------
Input assembly
----------------------------------------------------------*/
clr_struct( &ci_input );
ci_input.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
ci_input.topology = VKN_render_flags_get_topology( flags );

/*----------------------------------------------------------
Viewport/scissor
----------------------------------------------------------*/
clr_struct( &ci_viewport );
ci_viewport.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
ci_viewport.scissorCount  = 1;
ci_viewport.viewportCount = 1;

/*----------------------------------------------------------
Rasterizer
----------------------------------------------------------*/
clr_struct( &ci_raster );
ci_raster.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
ci_raster.polygonMode = VKN_render_flags_get_fill_mode( flags );
ci_raster.cullMode    = VKN_render_flags_get_cull_mode( flags );
ci_raster.frontFace   = VKN_render_flags_get_front_face_winding( flags );

/* line width - for values not 1.0, use VK_DYNAMIC_STATE_LINE_WIDTH */ // TODO <MPA> - Look into packing line width into render state flags
ci_raster.lineWidth = 1.0f;

/*----------------------------------------------------------
Multi-sampling
----------------------------------------------------------*/
clr_struct( &ci_msaa ); // TODO <MPA> - Look into packing VKN_msaa_type into the render state flags to support this
ci_msaa.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
ci_msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
ci_msaa.minSampleShading     = 1.0f;

/*----------------------------------------------------------
Depth/stencil ops
----------------------------------------------------------*/
clr_struct( &ci_depth_stencil );
ci_depth_stencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
ci_depth_stencil.flags                 = 0;
ci_depth_stencil.depthWriteEnable      = VKN_render_flags_get_depth_write_enabled( flags );
ci_depth_stencil.depthCompareOp        = VKN_render_flags_get_depth_test_compare_op( flags );
ci_depth_stencil.depthTestEnable       = ( ci_depth_stencil.depthCompareOp != VK_COMPARE_OP_ALWAYS );
ci_depth_stencil.depthBoundsTestEnable = ci_depth_stencil.depthTestEnable;
ci_depth_stencil.minDepthBounds        = 0.0f;
ci_depth_stencil.maxDepthBounds        = 1.0f;
ci_depth_stencil.front.failOp          = VKN_render_flags_get_stencil_write_fail_op( flags );
ci_depth_stencil.front.passOp          = VKN_render_flags_get_stencil_write_pass_op( flags );
ci_depth_stencil.front.depthFailOp     = VKN_render_flags_get_stencil_write_depth_fail_op( flags );
ci_depth_stencil.front.compareOp       = VKN_render_flags_get_stencil_test_compare_op( flags );
ci_depth_stencil.front.compareMask     = VKN_render_flags_get_stencil_test_compare_mask( flags );
ci_depth_stencil.front.reference       = VKN_render_flags_get_stencil_test_reference( flags );
ci_depth_stencil.front.writeMask       = max_uint_value( u32 );
ci_depth_stencil.stencilTestEnable     = ci_depth_stencil.front.compareOp   != VK_COMPARE_OP_ALWAYS
                                      || ci_depth_stencil.front.failOp      != VK_STENCIL_OP_KEEP
                                      || ci_depth_stencil.front.depthFailOp != VK_STENCIL_OP_KEEP
                                      || ci_depth_stencil.front.passOp      != VK_STENCIL_OP_KEEP;

/*----------------------------------------------------------
Blending
----------------------------------------------------------*/
clr_struct( &blend_attach0 );
blend_attach0.colorWriteMask      = VKN_render_flags_get_color_mask( flags );
blend_attach0.colorBlendOp        = VKN_render_flags_get_blend_op( flags );
blend_attach0.alphaBlendOp        = blend_attach0.colorBlendOp;
blend_attach0.srcColorBlendFactor = VKN_render_flags_get_blend_factor_source_color( flags );
blend_attach0.srcAlphaBlendFactor = VKN_render_flags_get_blend_factor_source_alpha( flags );
blend_attach0.dstColorBlendFactor = VKN_render_flags_get_blend_factor_destination_color( flags );
blend_attach0.dstAlphaBlendFactor = VKN_render_flags_get_blend_factor_destination_alpha( flags );
blend_attach0.blendEnable         = blend_attach0.srcColorBlendFactor != VK_BLEND_FACTOR_ONE
                                 || blend_attach0.srcAlphaBlendFactor != VK_BLEND_FACTOR_ONE
                                 || blend_attach0.dstColorBlendFactor != VK_BLEND_FACTOR_ZERO
                                 || blend_attach0.dstAlphaBlendFactor != VK_BLEND_FACTOR_ZERO;

clr_struct( &ci_blend );
ci_blend.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
ci_blend.pAttachments    = &blend_attach0;
ci_blend.attachmentCount = 1;

/*----------------------------------------------------------
Dynamic state
----------------------------------------------------------*/
clr_struct( &ci_dynamic );
ci_dynamic.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
ci_dynamic.pDynamicStates    = builder->state.dynamic_states.states;
ci_dynamic.dynamicStateCount = builder->state.dynamic_states.count;

/*----------------------------------------------------------
Create the pipeline
----------------------------------------------------------*/
clr_struct( &ci_pipeline );
ci_pipeline.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
ci_pipeline.pStages             = builder->state.stages.stages;
ci_pipeline.stageCount          = builder->state.stages.count;
ci_pipeline.pVertexInputState   = &ci_vertex;
ci_pipeline.pInputAssemblyState = &ci_input;
ci_pipeline.pViewportState      = &ci_viewport;
ci_pipeline.pRasterizationState = &ci_raster;
ci_pipeline.pMultisampleState   = &ci_msaa;
ci_pipeline.pDepthStencilState  = &ci_depth_stencil;
ci_pipeline.pColorBlendState    = &ci_blend;
ci_pipeline.pDynamicState       = &ci_dynamic;
ci_pipeline.layout              = layout;

/*----------------------------------------------------------
Dynamic rendering
----------------------------------------------------------*/
if( builder->state.use_dynamic_rendering )
    {
    clr_struct( &ci_render );
    ci_render.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    ci_render.colorAttachmentCount    = 1;
    ci_render.pColorAttachmentFormats = &builder->state.color_format;
    ci_render.depthAttachmentFormat   = builder->state.depth_format;
    ci_render.stencilAttachmentFormat = builder->state.stencil_format;

    ci_pipeline.pNext = &ci_render;
    }

if( VKN_failed( vkCreateGraphicsPipelines( builder->state.logical, builder->state.cache, 1, &ci_pipeline, builder->state.allocator, &pipeline->pipeline ) ) )
    {
    debug_assert_always();
    VKN_pipeline_graphics_destroy( NULL, pipeline );
    return( FALSE );
    }

/*----------------------------------------------------------
Save the render state used to create this
----------------------------------------------------------*/
pipeline->render_flags_mask = get_render_flags_mask( &builder->state.dynamic_states );
pipeline->render_flags      = ( flags & pipeline->render_flags_mask );

return( TRUE );

}   /* VKN_pipeline_graphics_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_pipeline_graphics_destroy
*
*   DESCRIPTION:
*       Destroy the given graphics pipeline.
*
*********************************************************************/

void VKN_pipeline_graphics_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_pipeline_graphics_type
                       *pipeline    /* graphics pipeline to destroy */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
use->i->release_pipeline( pipeline->logical, pipeline->allocator, pipeline->pipeline, use );

VKN_releaser_auto_mini_end( use );
clr_struct( pipeline );

}   /* VKN_pipeline_graphics_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_pipeline_graphics_init_builder
*
*   DESCRIPTION:
*       Initialize a graphics pipeline builder.
*
*********************************************************************/

VKN_PIPELINE_GRAPHICS_CONFIG_API VKN_pipeline_graphics_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_pipeline_graphics_build_config_type CONFIG =
    {
    add_dynamic_state,
    add_stage,
    add_vertex_attribute,
    add_vertex_binding,
    enable_dynamic_rendering,
    reset,
    set_allocation_callbacks,
    set_pipeline_cache
    };
    
/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical = logical;
reset( builder );

return( builder->config );

}   /* VKN_pipeline_graphics_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_dynamic_state
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API add_dynamic_state
    (
    const VkDynamicState
                        state,      /* state to make dynamic        */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VkDynamicState         *value;      /* dynamic state value          */

value = NULL;
for( i = 0; i < builder->state.dynamic_states.count; i++ )
    {
    if( builder->state.dynamic_states.states[ i ] == state )
        {
        value = &builder->state.dynamic_states.states[ i ];
        break;
        }
    }

if( !value )
    {
    if( builder->state.dynamic_states.count <= cnt_of_array( builder->state.dynamic_states.states ) )
        {
        value = &builder->state.dynamic_states.states[ builder->state.dynamic_states.count++ ];
        }

    debug_assert( value );
    }

if( value )
    {
    *value = state;
    }

return( builder->config );

}   /* add_dynamic_state() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_stage
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API add_stage
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
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPipelineShaderStageCreateInfo
                       *ci_stage;   /* stage create info            */
u32                     i;          /* loop counter                 */

ci_stage = NULL;
for( i = 0; i < builder->state.stages.count; i++ )
    {
    if( builder->state.stages.stages[ i ].stage == stage )
        {
        ci_stage = &builder->state.stages.stages[ i ];
        break;
        }
    }

if( !ci_stage )
    {
    if( builder->state.stages.count <= cnt_of_array( builder->state.stages.stages ) )
        {
        ci_stage = &builder->state.stages.stages[ builder->state.stages.count++ ];
        }

    debug_assert( ci_stage );
    }

if( ci_stage )
    {
    ci_stage->sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ci_stage->flags  = flags;
    ci_stage->module = shader;
    ci_stage->pName  = entry_point;
    ci_stage->stage  = (VkShaderStageFlagBits)stage;
    }

return( builder->config );

}   /* add_stage() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_vertex_attribute
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API add_vertex_attribute
    (
    const u32           location,   /* attribute location           */
    const u32           binding,    /* binding to which this applies*/
    const VkFormat      format,     /* data type                    */
    const u32           offset,     /* byte offset to start         */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkVertexInputBindingDescription     /* binding description          */
                       *binding_record;                               
u32                     i;          /* loop counter                 */
VkVertexInputAttributeDescription
                       *value;      /* binding description          */

/*----------------------------------------------------------
Find the binding record
----------------------------------------------------------*/
binding_record = NULL;
for( i = 0; i < builder->state.vertex_bindings.count; i++ )
    {
    if( builder->state.vertex_bindings.bindings[ i ].binding == binding )
        {
        binding_record = &builder->state.vertex_bindings.bindings[ i ];
        }
    }

debug_assert( binding_record );
if( binding_record )
    {
    value = NULL;
    for( i = 0; i < builder->state.vertex_attributes.count; i++ )
        {
        if( builder->state.vertex_attributes.attributes[ i ].binding  == binding
         && builder->state.vertex_attributes.attributes[ i ].location == location )
            {
            value = &builder->state.vertex_attributes.attributes[ i ];
            }
        }

    if( !value )
        {
        if( builder->state.vertex_attributes.count <= cnt_of_array( builder->state.vertex_attributes.attributes ) )
            {
            value = &builder->state.vertex_attributes.attributes[ builder->state.vertex_attributes.count++ ];
            }

        debug_assert( value );
        }

    if( value )
        {
        value->binding  = binding;
        value->format   = format;
        value->location = location;
        value->offset   = offset;
        }
    }

return( builder->config );

}   /* add_vertex_attribute() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_vertex_binding
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API add_vertex_binding
    (
    const u32           binding,    /* binding index                */
    const u32           stride,     /* between consecutive elements */
    const bool          as_instance,/* rate is per instance?        */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VkVertexInputBindingDescription
                       *value;      /* binding description          */

value = NULL;
for( i = 0; i < builder->state.vertex_bindings.count; i++ )
    {
    if( builder->state.vertex_bindings.bindings[ i ].binding == binding )
        {
        value = &builder->state.vertex_bindings.bindings[ i ];
        break;
        }
    }

if( !value )
    {
    if( builder->state.vertex_bindings.count <= cnt_of_array( builder->state.vertex_bindings.bindings ) )
        {
        value = &builder->state.vertex_bindings.bindings[ builder->state.vertex_bindings.count++ ];
        }

    debug_assert( value );
    }

if( value )
    {
    value->binding   = binding;
    value->stride    = stride;
    value->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    if( as_instance )
        {
        value->inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        }
    }

return( builder->config );

}   /* add_vertex_binding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       enable_dynamic_rendering
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API enable_dynamic_rendering
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
    )
{
builder->state.use_dynamic_rendering = enable;
builder->state.color_format          = color_format;
builder->state.depth_format          = depth_format;
builder->state.stencil_format        = stencil_format;

return( builder->config );

}   /* enable_dynamic_rendering() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_render_flags_mask
*
*********************************************************************/

static VKN_render_flags_type get_render_flags_mask
    (
    const VKN_pipeline_graphics_build_dynamic_states_type
                       *dynamics    /* dynamic states               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VKN_render_flags_type   ret;        /* return state mask            */

ret = 0;
for( i = 0; i < dynamics->count; i++ )
    {
    switch( dynamics->states[ i ] )
        {
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
            ret |= VKN_RENDER_FLAG_BITS_BLEND_OP_MASK;
            if( has_dynamic( VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, dynamics ) )
                {
                ret |= VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_MASK;
                ret |= VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_MASK;
                ret |= VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_MASK;
                ret |= VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_MASK;
                }
            break;

        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
            ret |= VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_MASK;
            ret |= VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_MASK;
            ret |= VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_MASK;
            ret |= VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_MASK;
            break;

        case VK_DYNAMIC_STATE_CULL_MODE:
            ret |= VKN_RENDER_FLAG_BITS_CULL_MODE_MASK;
            break;

        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            if( has_dynamic( VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, dynamics ) )
                {
                ret |= VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK;
                }
            break;

        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            ret |= VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_MASK;
            break;

        case VK_DYNAMIC_STATE_FRONT_FACE:
            ret |= VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_MASK;
            break;

        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
            ret |= VKN_RENDER_FLAG_BITS_FILL_MODE_MASK;
            break;

        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
            ret |= VKN_RENDER_FLAG_BITS_TOPOLOGY_MASK;
            break;

        case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            ret |= VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_MASK;
            break;

        case VK_DYNAMIC_STATE_STENCIL_OP:
            if( has_dynamic( VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE, dynamics ) )
                {
                ret |= VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK;
                }
            break;

        case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            ret |= VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_MASK;
            break;

        default:
            /* ignore */
            break;
        }
    }

return( ~ret );

}   /* get_render_flags_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API reset
    (
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
clr_struct( &builder->state.dynamic_states );
clr_struct( &builder->state.vertex_attributes );
clr_struct( &builder->state.vertex_bindings );
clr_struct( &builder->state.stages );

/*----------------------------------------------------------
Always use dynamic viewport and scissor
----------------------------------------------------------*/
builder->config->add_dynamic_state( VK_DYNAMIC_STATE_VIEWPORT, builder )->
                 add_dynamic_state( VK_DYNAMIC_STATE_SCISSOR, builder );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_pipeline_cache
*
*********************************************************************/

static VKN_PIPELINE_GRAPHICS_CONFIG_API set_pipeline_cache
    (
    const VkPipelineCache
                        cache,      /* pipeline cache               */
    struct _VKN_pipeline_graphics_build_type
                       *builder     /* graphics pipeline builder    */
    )
{
builder->state.cache = cache;

return( builder->config );

}   /* set_pipeline_cache() */
