#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknEffect.hpp"
#include "VknEffectTypes.hpp"
#include "VknReleaser.hpp"


static VKN_effect_build_add_stage_proc_type add_stage;

static u32 fingerprint_descriptor_set
    (
    const VkDescriptorSetLayoutCreateFlags
                        flags,      /* set layout flags             */
    const VkDescriptorSetLayoutBinding
                       *bindings,   /* descriptor bindings          */
    const u32           binding_count
                                    /* num of desccriptor bindings  */
    );

static bool merge_stage_bindings
    (
    const VkShaderStageFlags
                        stage,      /* stage name                   */
    const VKN_shader_layout_bindings_type
                       *bindings,   /* descriptor bindings          */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

static bool merge_stage_push_constants
    (
    const VkShaderStageFlags
                        stage,      /* stage name                   */
    const VKN_shader_layout_push_constants_type
                       *push_constants,
                                    /* push constant ranges         */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

static VKN_effect_build_finalize_stages_proc_type finalize_stages;
static VKN_effect_build_reset_proc_type reset;
static VKN_effect_build_set_allocation_callbacks_proc_type set_allocation_callbacks;

static void sort_descriptor_bindings
    (
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_effect_create
*
*   DESCRIPTION:
*       Create a shader effect via the given builder.
*
*********************************************************************/

bool VKN_effect_create
    (
    const char         *str_name,   /* optional string for debug    */
    const VKN_effect_build_type
                       *builder,    /* shader effect builder        */
    VKN_effect_type    *effect      /* output new shader effect     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPipelineLayoutCreateInfo          /* pipeline layout create info  */
                        ci_pipeline_layout;
VkDescriptorSetLayoutCreateInfo     /* set layout create info       */
                        ci_set_layout;
u32                     i;          /* loop counter                 */
u32                     param_count;/* used param count             */
VKN_effect_parameter_mapping_type
                      **parameters; /* shader parameter head        */
VKN_effect_pipeline_stage_info_type /* working pipeline stage info  */
                       *pipeline_stage;
u32                     set;        /* working set number           */
u32                     set_binding_count[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* number bindings in sets      */
u32                     set_layout_count;
                                    /* number of set layouts        */
VkDescriptorSetLayout   set_layouts[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* set layout array             */
const VKN_effect_build_set_binding_type
                       *set_starts[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* first binding each set       */
const VKN_shader_parameter_mapping_type
                       *that_param; /* builder's shader parameter   */
VKN_effect_parameter_mapping_type
                       *this_param; /* effect's shader parameter    */

/*----------------------------------------------------------
Create the effect
----------------------------------------------------------*/
clr_struct( effect );
effect->logical        = builder->state.logical;
effect->allocator      = builder->state.allocator;

param_count = 0;

/*----------------------------------------------------------
Set layouts
----------------------------------------------------------*/
clr_array( set_binding_count );
clr_array( set_starts );
for( i = 0; i < builder->state.set_bindings.count; i++ )
    {
    /*------------------------------------------------------
    Set binding start and count
    ------------------------------------------------------*/
    set = builder->state.set_bindings.set_bindings[ i ].set;
    if( !set_starts[ set ] )
        {
        set_starts[ set ] = &builder->state.set_bindings.set_bindings[ i ];
        }

    set_binding_count[ set ]++;

    /*-----------------------------------------------------
    Shader parameters
    -----------------------------------------------------*/
    that_param = builder->state.set_bindings.set_bindings[ i ].parameters;
    if( !that_param )
        {
        continue;
        }

    parameters = &effect->set_params[ set ];
    while( *parameters )
        {
        /*-------------------------------------------------
        We add to tail
        -------------------------------------------------*/
        parameters = &(*parameters)->next;
        }

    /*-----------------------------------------------------
    Image parameter
    -----------------------------------------------------*/
    if( that_param->cls == VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE )
        {
        if( param_count >= cnt_of_array( effect->param_pool ) )
            {
            debug_assert_always();
            VKN_effect_destroy( NULL, effect );
            return( FALSE );
            }

        this_param  = &effect->param_pool[ param_count++ ];
        *parameters = this_param;
        
        this_param->binding = builder->state.set_bindings.set_bindings[ i ].binding->binding;
        this_param->cls     = VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE;
        this_param->image   = that_param->image;
        continue;
        }

    /*-----------------------------------------------------
    Vector parameter(s)
    -----------------------------------------------------*/
    for( ; that_param; that_param = that_param->vector.next )
        {
        if( param_count >= cnt_of_array( effect->param_pool ) )
            {
            debug_assert_always();
            VKN_effect_destroy( NULL, effect );
            return( FALSE );
            }

        this_param  = &effect->param_pool[ param_count++ ];
        *parameters = this_param;
        parameters  = &this_param->next;

        this_param->binding       = builder->state.set_bindings.set_bindings[ i ].binding->binding;
        this_param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
        this_param->vector.vector = that_param->vector.vector;
        this_param->vector.width  = that_param->vector.width;
        }
    }

/*----------------------------------------------------------
Descriptor set layouts.  Must have a layout for each set,
even when that set is empty, since Vulkan indexes the array
by the set index in the pipeline layout... (>_<)aagh!
----------------------------------------------------------*/
set_layout_count = 0;
for( i = 0; i < cnt_of_array( set_starts ); i++ )
    {
    clr_struct( &ci_set_layout );
    ci_set_layout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci_set_layout.bindingCount = set_binding_count[ i ];
    if( set_starts[ i ] )
        {
        ci_set_layout.pBindings = set_starts[ i ]->binding;
        }

    if( VKN_failed( vkCreateDescriptorSetLayout( effect->logical, &ci_set_layout, effect->allocator, &effect->sets[ i ] ) ) )
        {
        VKN_effect_destroy( NULL, effect );
        return( FALSE );
        }

    effect->set_fingerprints[ i ] = fingerprint_descriptor_set( ci_set_layout.flags, ci_set_layout.pBindings, ci_set_layout.bindingCount );
    set_layouts[ set_layout_count++ ] = effect->sets[ i ];
    }

/*----------------------------------------------------------
Push constants
----------------------------------------------------------*/
for( i = 0; i < builder->state.push_constants.count; i++, effect->push_constant_cnt++ )
    {
    effect->push_constants[ effect->push_constant_cnt ] = builder->state.push_constants.ranges[ i ];

    parameters = &effect->push_constant_params[ effect->push_constant_cnt ];
    for( that_param = builder->state.push_constants.parameters[ i ]; that_param; that_param = that_param->vector.next )
        {
        if( param_count >= cnt_of_array( effect->param_pool ) )
            {
            debug_assert_always();
            VKN_effect_destroy( NULL, effect );
            return( FALSE );
            }

        this_param  = &effect->param_pool[ param_count++ ];
        *parameters = this_param;
        parameters  = &this_param->next;

        this_param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
        this_param->vector.vector = that_param->vector.vector;
        this_param->vector.width  = that_param->vector.width;
        }
    }

/*----------------------------------------------------------
Pipeline layout
----------------------------------------------------------*/
clr_struct( &ci_pipeline_layout );
ci_pipeline_layout.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
ci_pipeline_layout.flags                  = 0;
ci_pipeline_layout.pPushConstantRanges    = effect->push_constants;
ci_pipeline_layout.pushConstantRangeCount = effect->push_constant_cnt;
ci_pipeline_layout.pSetLayouts            = set_layouts;
ci_pipeline_layout.setLayoutCount         = set_layout_count;

if( VKN_failed( vkCreatePipelineLayout( effect->logical, &ci_pipeline_layout, effect->allocator, &effect->layout ) ) )
    {
    VKN_effect_destroy( NULL, effect );
    return( FALSE );
    }

/*----------------------------------------------------------
Shader pipeline stages
----------------------------------------------------------*/
for( i = 0; i < cnt_of_array( builder->state.stages ); i++ )
    {
    if( !builder->state.stages[ i ] )
        {
        continue;
        }

    pipeline_stage = &effect->stages[ effect->stage_cnt++ ];
    pipeline_stage->flags       = 0;
    pipeline_stage->entry_point = builder->state.stages[ i ]->entry_point;
    pipeline_stage->stage       = VKN_SHADER_GFX_STAGE_TO_STAGE_FLAGS[ i ];
    pipeline_stage->shader      = builder->state.stages[ i ]->shader;
    }

/*----------------------------------------------------------
Debug Name
----------------------------------------------------------*/
for( i = 0; i < cnt_of_array( effect->debug_name ); i++ )
    {
    effect->debug_name[ i ] = str_name[ i ];
    if( !effect->debug_name[ i ] )
        {
        break;
        }
    }

effect->debug_name[ cnt_of_array( effect->debug_name ) - 1 ] = 0;

return( TRUE );

}   /* VKN_effect_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_effect_destroy
*
*   DESCRIPTION:
*       Destroy the given shader effect.
*
*********************************************************************/

void VKN_effect_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_effect_type    *effect      /* shader effect to destroy     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

VKN_releaser_auto_mini_begin( releaser, use );
for( i = 0; i < cnt_of_array( effect->sets ); i++ )
    {
    use->i->release_descriptor_set_layout( effect->logical, effect->allocator, effect->sets[ i ], use );
    }

use->i->release_pipeline_layout( effect->logical, effect->allocator, effect->layout, use );

VKN_releaser_auto_mini_end( use );
clr_struct( effect );

}   /* VKN_effect_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_effect_init_builder
*
*   DESCRIPTION:
*       Initialize a shader effect builder.
*
*********************************************************************/

VKN_EFFECT_CONFIG_API VKN_effect_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_effect_build_config_type CONFIG =
    {
    add_stage,
    finalize_stages,
    reset,
    set_allocation_callbacks
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical = logical;

return( builder->config );

}   /* VKN_effect_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_stage
*
*********************************************************************/

static VKN_EFFECT_CONFIG_API add_stage
    (
    const VKN_shader_gfx_stage_type
                        stage,      /* graphics stage name          */
    const VKN_shader_type
                       *shader,     /* shader module                */
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
builder->state.stages[ stage ] = shader;

return( builder->config );

}   /* add_stage() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       finalize_stages
*
*********************************************************************/

static VKN_EFFECT_CONFIG_API finalize_stages
    (
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
VkShaderStageFlags      stage;      /* stage name                   */

clr_array( builder->state.bindings );
clr_struct( &builder->state.set_bindings );
clr_struct( &builder->state.push_constants );

/*----------------------------------------------------------
Copy/merge the bindings and push constants from the shader
stages into our temporary buffer
----------------------------------------------------------*/
for( i = 0; i < cnt_of_array( builder->state.stages ); i++ )
    {
    if( !builder->state.stages[ i ] )
        {
        continue;
        }

    stage = VKN_SHADER_GFX_STAGE_TO_STAGE_FLAGS[ i ];
    merge_stage_bindings( stage, &builder->state.stages[ i ]->descriptor_bindings, builder );
    merge_stage_push_constants( stage, &builder->state.stages[ i ]->push_constants, builder );
    }

sort_descriptor_bindings( builder );

return( builder->config );

}   /* finalize_stages() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       fingerprint_descriptor_set
*
*********************************************************************/

static u32 fingerprint_descriptor_set
    (
    const VkDescriptorSetLayoutCreateFlags
                        flags,      /* set layout flags             */
    const VkDescriptorSetLayoutBinding
                       *bindings,   /* descriptor bindings          */
    const u32           binding_count
                                    /* num of desccriptor bindings  */
    )
{
/*----------------------------------------------------------
Local types
----------------------------------------------------------*/
typedef union
    {
    struct
        {
        VkDescriptorSetLayoutCreateFlags
                        flags;
        u32             binding_count;
        } set;

    struct
        {
        u32             binding;
        VkDescriptorType
                        descriptor_kind;
        u32             descriptor_count;
        VkShaderStageFlags
                        stages;
        } binding;
    } fingerprint_type;

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
fingerprint_type        fingerprint;/* fingerprint data             */
u32                     i;          /* loop counter                 */
u32                     ret;        /* return fingerprint           */

ret = VKN_HASH_SEED;

/*----------------------------------------------------------
Set
----------------------------------------------------------*/
clr_struct( &fingerprint );
fingerprint.set.flags         = flags;
fingerprint.set.binding_count = binding_count;

ret = VKN_hash_blob( ret, &fingerprint, sizeof( fingerprint ) );

/*----------------------------------------------------------
Bindings
----------------------------------------------------------*/
for( i = 0; i < binding_count; i++ )
    {
    clr_struct( &fingerprint );
    fingerprint.binding.binding          = bindings[ i ].binding;
    fingerprint.binding.descriptor_count = bindings[ i ].descriptorCount;
    fingerprint.binding.descriptor_kind  = bindings[ i ].descriptorType;
    fingerprint.binding.stages           = bindings[ i ].stageFlags;
    
    ret = VKN_hash_blob( ret, &fingerprint, sizeof( fingerprint ) );
    }

return( ret );

}   /* fingerprint_descriptor_set() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       merge_stage_bindings
*
*********************************************************************/

static bool merge_stage_bindings
    (
    const VkShaderStageFlags
                        stage,      /* stage name                   */
    const VKN_shader_layout_bindings_type
                       *bindings,   /* descriptor bindings          */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_effect_build_set_binding_type
                       *binding;    /* working binding              */
VKN_shader_parameter_mapping_type
                      **head;       /* parameter head               */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VKN_shader_parameter_mapping_type
                       *this_param; /* builder's shader parameter   */
VKN_shader_parameter_mapping_type
                       *that_param; /* stage's shader parameter     */

for( i = 0; i < bindings->count; i++ )
    {
    binding = NULL;

    if( bindings->bindings[ i ].set >= cnt_of_array ( ( (VKN_effect_type*)NULL )->sets ) )
        {
        debug_assert_always();
        continue;
        }

    /*------------------------------------------------------
    Search for the binding in our catalog
    ------------------------------------------------------*/
    for( j = 0; j < builder->state.set_bindings.count; j++ )
        {
        if( bindings->bindings[ i ].set     == builder->state.set_bindings.set_bindings[ j ].set
         && bindings->bindings[ i ].binding == builder->state.set_bindings.set_bindings[ j ].binding->binding )
            {
            binding = &builder->state.set_bindings.set_bindings[ j ];
            break;
            }
        }

    if( !binding )
        {
        /*--------------------------------------------------
        First time seeing this binding
        --------------------------------------------------*/
        if( builder->state.set_bindings.count >= cnt_of_array( builder->state.set_bindings.set_bindings ) )
            {
            debug_assert_always();
            return( FALSE );
            }

        binding          = &builder->state.set_bindings.set_bindings[ builder->state.set_bindings.count ];
        binding->binding = &builder->state.bindings[ builder->state.set_bindings.count ];
        builder->state.set_bindings.count++;


        binding->set                      = bindings->bindings[ i ].set;
        binding->binding->binding         = bindings->bindings[ i ].binding;
        binding->binding->descriptorType  = bindings->bindings[ i ].kind;
        binding->binding->descriptorCount = bindings->bindings[ i ].count;
        }

    /*------------------------------------------------------
    Shader parameters
    ------------------------------------------------------*/
    if( !binding->parameters )
        {
        /*--------------------------------------------------
        Inherit parameters from this stage
        --------------------------------------------------*/
        head = &binding->parameters;
        for( that_param = bindings->bindings[ i ].parameters; that_param; )
            {
            if( builder->state.param_cnt >= cnt_of_array( builder->state.param_pool ) )
                {
                debug_assert_always();
                return( FALSE );
                }

            this_param = &builder->state.param_pool[ builder->state.param_cnt++ ];
            *head      = this_param;
            head       = &this_param->vector.next;

            switch( that_param->cls )
                {
                case VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE:
                    this_param->cls   = VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE;
                    this_param->image = that_param->image;

                    that_param = NULL;
                    break;
                    
                case VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR:
                    this_param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
                    this_param->vector.vector = that_param->vector.vector;
                    this_param->vector.width  = that_param->vector.width;

                    that_param = that_param->vector.next;
                    break;

                default:
                    debug_assert_always();
                    return( FALSE );
                }
            }
        }
    else if( bindings->bindings[ i ].parameters )
        {
        /*--------------------------------------------------
        Check that parameters match
        --------------------------------------------------*/
        if( bindings->bindings[ i ].parameters->cls == VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE
         && binding->parameters->cls                == VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE )
            {
            debug_assert_always();
            return( FALSE );
            }
        else
            {
            for( this_param = binding->parameters, that_param = bindings->bindings[ i ].parameters;
                 this_param && that_param;
                 this_param = this_param->vector.next, that_param->vector.next )
                {
                if( this_param->cls           != that_param->cls
                 || this_param->cls           != VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR
                 || this_param->vector.vector != that_param->vector.vector
                 || this_param->vector.width  != that_param->vector.width )
                    {
                    debug_assert_always();
                    return( FALSE );
                    }
                }

            if( this_param
             || that_param )
                {
                debug_assert_always();
                return( FALSE );
                }
            }
        }

    binding->binding->stageFlags |= stage;
    }

return( TRUE );

}   /* merge_stage_bindings() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       merge_stage_push_constants
*
*********************************************************************/

static bool merge_stage_push_constants
    (
    const VkShaderStageFlags
                        stage,      /* stage name                   */
    const VKN_shader_layout_push_constants_type
                       *push_constants,
                                    /* push constant ranges         */
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VKN_shader_parameter_mapping_type
                      **parameters; /* working push constant params */
VkPushConstantRange    *range;      /* push constant range          */
VKN_shader_parameter_mapping_type
                       *this_param; /* builder's shader parameter   */
VKN_shader_parameter_mapping_type
                       *that_param; /* stage's shader parameter     */

for( i = 0; i < push_constants->count; i++ )
    {
    range      = NULL;
    parameters = NULL;

    /*------------------------------------------------------
    Search for the push constant in our catalog
    ------------------------------------------------------*/
    for( j = 0; j < builder->state.push_constants.count; j++ )
        {
        if( push_constants->constants[ i ].offset == builder->state.push_constants.ranges[ j ].offset
         && push_constants->constants[ i ].size   == builder->state.push_constants.ranges[ j ].size )
            {
            range      = &builder->state.push_constants.ranges[ j ];
            parameters = &builder->state.push_constants.parameters[ j ];
            break;
            }
        }

    if( !range )
        {
        /*--------------------------------------------------
        First time seeing this push constant
        --------------------------------------------------*/
        if( builder->state.push_constants.count >= cnt_of_array( builder->state.push_constants.ranges ) )
            {
            debug_assert_always();
            return( FALSE );
            }

        range      = &builder->state.push_constants.ranges[ builder->state.push_constants.count ];
        parameters = &builder->state.push_constants.parameters[ builder->state.push_constants.count++ ];

        range->offset       = push_constants->constants[ i ].offset;
        range->size         = push_constants->constants[ i ].size;
        }

    /*------------------------------------------------------
    Shader parameters
    ------------------------------------------------------*/
    if( !*parameters )
        {
        /*--------------------------------------------------
        Inherit parameters from this stage
        --------------------------------------------------*/
        for( that_param = push_constants->constants[ i ].parameters; that_param; that_param = that_param->vector.next )
            {
            if( builder->state.param_cnt >= cnt_of_array( builder->state.param_pool ) )
                {
                debug_assert_always();
                return( FALSE );
                }

            this_param = &builder->state.param_pool[ builder->state.param_cnt++ ];
            *parameters = this_param;
            parameters  = &this_param->vector.next;

            if( that_param->cls != VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR )
                {
                debug_assert_always();
                return( FALSE );
                }

            this_param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
            this_param->vector.vector = that_param->vector.vector;
            this_param->vector.width  = that_param->vector.width;
            }
        }
    else if( push_constants->constants[ i ].parameters )
        {
        /*--------------------------------------------------
        Check that parameters match
        --------------------------------------------------*/
        for( this_param = *parameters, that_param = push_constants->constants[ i ].parameters;
             this_param && that_param;
             this_param = this_param->vector.next, that_param->vector.next )
            {
            if( this_param->cls           != that_param->cls
             || this_param->cls           != VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR
             || this_param->vector.vector != that_param->vector.vector
             || this_param->vector.width  != that_param->vector.width )
                {
                debug_assert_always();
                return( FALSE );
                }
            }

        if( this_param
         || that_param )
            {
            debug_assert_always();
            return( FALSE );
            }
        }

    range->stageFlags |= stage;
    }

return( TRUE );

}   /* merge_stage_push_constants() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_EFFECT_CONFIG_API reset
    (
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
clr_array( builder->state.stages );
clr_array( builder->state.bindings );
clr_struct( &builder->state.set_bindings );
clr_struct( &builder->state.push_constants );
clr_array( builder->state.param_pool );
builder->state.param_cnt = 0;

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_EFFECT_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       sort_descriptor_bindings
*
*********************************************************************/

static void sort_descriptor_bindings
    (
    VKN_effect_build_type
                       *builder     /* shader effect builder        */
    )
{
/*----------------------------------------------------------
Local types
----------------------------------------------------------*/
typedef struct
    {
    VKN_effect_build_set_binding_type
                       *set_binding;/* set/binding                  */
    VkDescriptorSetLayoutBinding
                       *binding;    /* binding                      */
    } swap_type;

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
swap_type               a;          /* first swap value             */
swap_type               b;          /* second swap value            */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VkDescriptorSetLayoutBinding        /* temporary swap value         */
                        temp_binding;
VKN_effect_build_set_binding_type   /* temporary swap value         */
                        temp_set_binding;

/*----------------------------------------------------------
Sort the bindings in row (major), binding index (minor)
ascending order - such that the bindings form a contiguous
array within the buffer.

We know since both the set binding and underlying binding
were allocated in tandem, that both arrays are currently in
the same order.  We take advantage of this by swapping both
arrays in tandem.
----------------------------------------------------------*/
for( i = 1; i < builder->state.set_bindings.count; i++ )
    {
    for( j = 0; j < builder->state.set_bindings.count - i; j++ )
        {
        a.set_binding = &builder->state.set_bindings.set_bindings[ j + 0 ];
        a.binding     = a.set_binding->binding;

        b.set_binding = &builder->state.set_bindings.set_bindings[ j + 1 ];
        b.binding     = b.set_binding->binding;

        if( a.set_binding->set < b.set_binding->set
         || ( a.set_binding->set == b.set_binding->set
           && a.binding->binding < b.binding->binding ) )
            {
            continue;
            }

        temp_set_binding = *a.set_binding;
        temp_binding     = *a.binding;

        *a.set_binding = *b.set_binding;
        *a.binding     = *b.binding;

        *b.set_binding = temp_set_binding;
        *b.binding     = temp_binding;

        a.set_binding->binding = a.binding;
        b.set_binding->binding = b.binding;
        }
    }

}   /* sort_descriptor_bindings() */
