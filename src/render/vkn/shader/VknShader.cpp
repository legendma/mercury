#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknShader.hpp"
#include "VknShaderParamTypes.hpp"
#include "VknShaderReflect.hpp"
#include "VknShaderTypes.hpp"
#include "VknReleaser.hpp"


#define ENTRY_POINT_NAME            "main"
#define MAX_UNIFORM_STRING_LENGTH   ( 150 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       has_override
*
*********************************************************************/

static __inline bool has_override
    (
    const u32           check,      /* override name to check       */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* existing overrides           */
    const u32           override_cnt/* number of existing overrides */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < override_cnt; i++ )
    {
    if( check == overrides[ i ].name )
        {
        return( TRUE );
        }
    }

return( FALSE );

}   /* has_override() */


static VKN_shader_build_add_override_proc_type add_override;

static bool add_override_safe
    (
    const VKN_shader_reflect_spirv_override_type
                       *item,       /* override                     */
    VKN_shader_build_overrides_type
                       *overrides   /* override list                */
    );

static VKN_shader_build_map_uniform_image_proc_type map_uniform_image;
static VKN_shader_build_map_uniform_vector_proc_type map_uniform_vector;
static VKN_shader_build_reset_proc_type reset;

static bool resolve_layout
    (
    const VKN_shader_build_type
                       *builder,   /* shader builder               */
    VKN_shader_type    *shader     /* working shader               */
    );

static bool retrieve_layout
    (
    VKN_shader_build_type
                       *builder    /* shader builder               */
    );

static VKN_shader_build_set_allocation_callbacks_proc_type set_allocation_callbacks;
static VKN_shader_build_set_code_proc_type set_code;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_create
*
*   DESCRIPTION:
*       Create a shader via the given builder.
*
*********************************************************************/

bool VKN_shader_create
    (
    const VKN_shader_build_type
                       *builder,    /* shader builder               */
    VKN_shader_type    *shader      /* output new shader            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkShaderModuleCreateInfo
                        ci_shader;  /* shader create info           */

/*----------------------------------------------------------
Validate
----------------------------------------------------------*/
if( builder->state.logical == VK_NULL_HANDLE
 || !builder->state.code
 || !builder->state.code_sz
 || !builder->state.have_layout )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Create the shader
----------------------------------------------------------*/
clr_struct( &ci_shader );
ci_shader.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
ci_shader.pCode    = builder->state.code;
ci_shader.codeSize = (size_t)builder->state.code_sz;

VKN_return_fail( vkCreateShaderModule( builder->state.logical, &ci_shader, builder->state.allocator, &shader->shader ) );

shader->logical     = builder->state.logical;
shader->allocator   = builder->state.allocator;
shader->entry_point = ENTRY_POINT_NAME;

/*----------------------------------------------------------
Resolve the layout
----------------------------------------------------------*/
if( !resolve_layout( builder, shader ) )
    {
    VKN_shader_destroy( NULL, shader );
    return( FALSE );
    }

return( TRUE );

}   /* VKN_shader_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_destroy
*
*   DESCRIPTION:
*       Destroy the given shader.
*
*********************************************************************/

void VKN_shader_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_shader_type    *shader      /* shader to destroy            */
    )
{
VKN_releaser_auto_mini_begin( releaser, use );
if( shader->shader != VK_NULL_HANDLE )
    {
    use->i->release_shader_module( shader->logical, shader->allocator, shader->shader, use );
    }

VKN_releaser_auto_mini_end( use );
clr_struct( shader );

}   /* VKN_shader_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_init_builder
*
*   DESCRIPTION:
*       Initialize a shader builder.
*
*********************************************************************/

VKN_SHADER_CONFIG_API VKN_shader_init_builder
    (
    const VkDevice      logical,    /* associated logical device    */
    VKN_shader_build_type
                       *builder     /* shader builder               */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_shader_build_config_type CONFIG =
    {
    add_override,
    map_uniform_image,
    map_uniform_vector,
    reset,
    set_allocation_callbacks,
    set_code
    };

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.logical = logical;

return( builder->config );

}   /* VKN_shader_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_override
*
*********************************************************************/

static VKN_SHADER_CONFIG_API add_override
    (
    const char         *name,       /* binding string name          */
    const VkDescriptorType
                        descriptor, /* desired descriptor type      */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_shader_reflect_spirv_override_type
                        item;       /* override item                */

VKN_shader_reflect_make_override( name, descriptor, &item );
add_override_safe( &item, &builder->state.overrides );

return( builder->config );

}   /* add_override() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_override_safe
*
*********************************************************************/

static bool add_override_safe
    (
    const VKN_shader_reflect_spirv_override_type
                       *item,       /* override                     */
    VKN_shader_build_overrides_type
                       *overrides   /* override list                */
    )
{
if( has_override( item->name, overrides->overrides, overrides->count ) )
    {
    return( TRUE );
    }
else if( overrides->count >= cnt_of_array( overrides->overrides ) )
    {
    debug_assert_always();
    return( FALSE );
    }

overrides->overrides[ overrides->count++ ] = *item;
return( TRUE );

}   /* add_override_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       map_uniform_image
*
*********************************************************************/

static VKN_SHADER_CONFIG_API map_uniform_image
    (
    const char         *uniform,    /* uniform string name          */
    const VKN_shader_param_image_name_type
                        image,      /* image parameter name         */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
u32                     name;       /* uniform name                 */
VKN_shader_build_uniform_param_type
                       *param;      /* new shader parameter mapping */

/*----------------------------------------------------------
Hash the string name
----------------------------------------------------------*/
name = VKN_hash_blob( VKN_HASH_SEED, uniform, 1 + (u32)strnlen( uniform, MAX_UNIFORM_STRING_LENGTH ) );

/*----------------------------------------------------------
Check for duplicates
----------------------------------------------------------*/
param = NULL;
for( i = 0; i < cnt_of_array( builder->state.uniforms.params ); i++ )
    {
    if( builder->state.uniforms.params[ i ].uniform_name == name
     && builder->state.uniforms.params[ i ].cls == VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE )
        {
        param = &builder->state.uniforms.params[ i ];
        break;
        }
    }

if( !param
 && builder->state.uniforms.count < cnt_of_array( builder->state.uniforms.params ) )
    {
    param = &builder->state.uniforms.params[ builder->state.uniforms.count++ ];
    }

/*----------------------------------------------------------
Add the mapping
----------------------------------------------------------*/
debug_assert( param );
if( param )
    {
    param->uniform_name = name;
    param->cls          = VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE;
    param->image        = image;
    }

return( builder->config );

}   /* map_uniform_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       map_uniform_vector
*
*********************************************************************/

static VKN_SHADER_CONFIG_API map_uniform_vector
    (
    const char         *uniform,    /* uniform string name          */
    const VKN_shader_param_vector_name_type
                        vector,     /* vector parameter name        */
    const u8            width,      /* number of floats in vector   */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     name;       /* uniform name                 */
VKN_shader_build_uniform_param_type
                       *param;      /* new shader parameter mapping */

/*----------------------------------------------------------
Hash the string name
----------------------------------------------------------*/
name = VKN_hash_blob( VKN_HASH_SEED, uniform, 1 + (u32)strnlen( uniform, MAX_UNIFORM_STRING_LENGTH ) );

/*----------------------------------------------------------
Add the mapping
----------------------------------------------------------*/
param = NULL;
if( builder->state.uniforms.count < cnt_of_array( builder->state.uniforms.params ) )
    {
    param = &builder->state.uniforms.params[ builder->state.uniforms.count++ ];
    }

debug_assert( param );
if( param )
    {
    param->uniform_name  = name;
    param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
    param->vector.vector = vector;
    param->vector.width  = width;
    }

return( builder->config );

}   /* map_uniform_vector() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       reset
*
*********************************************************************/

static VKN_SHADER_CONFIG_API reset
    (
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
builder->state.code        = NULL;
builder->state.code_sz     = 0;
builder->state.have_layout = FALSE;
clr_struct( &builder->state.overrides );
clr_struct( &builder->state.descriptor_bindings );
clr_struct( &builder->state.push_constants );
clr_struct( &builder->state.uniforms );

return( builder->config );

}   /* reset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       resolve_layout
*
*********************************************************************/

static bool resolve_layout
    (
    const VKN_shader_build_type
                       *builder,   /* shader builder               */
    VKN_shader_type    *shader     /* working shader               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const VKN_shader_reflect_descriptor_binding_type
                       *binding_that;
                                    /* builder binding             */
VKN_shader_descriptor_binding_type  /* shader binding              */
                       *binding_this;
const VKN_shader_reflect_push_constant_type
                       *constant_that;
                                    /* builder push constant       */
VKN_shader_layout_push_constant_type/* shader push constant        */
                       *constant_this;
VKN_shader_parameter_mapping_type
                      **head;       /* parameter list head         */
u32                     i;          /* loop counter                */
u32                     j;          /* loop counter                */
VKN_shader_parameter_mapping_type
                       *new_param;  /* new parameter mapping       */
u32                     vectors_sz; /* size of vector parameters   */
u32                     used_cnt;   /* number of params used       */

used_cnt = 0;

/*----------------------------------------------------------
Descriptor bindings
----------------------------------------------------------*/
for( i = 0; i < builder->state.descriptor_bindings.count; i++ )
    {
    binding_that = &builder->state.descriptor_bindings.bindings[ i ];
    if( shader->descriptor_bindings.count >= cnt_of_array( shader->descriptor_bindings.bindings ) )
        {
        debug_assert_always();
        return( FALSE );
        }

    binding_this = &shader->descriptor_bindings.bindings[ shader->descriptor_bindings.count++ ];
    binding_this->set     = binding_that->set;
    binding_this->binding = binding_that->binding;
    binding_this->kind    = binding_that->kind;
    binding_this->count   = binding_that->count;
    
    /*------------------------------------------------------
    Image binding
    ------------------------------------------------------*/
    for( j = 0; j < builder->state.uniforms.count; j++ )
        {
        if( builder->state.uniforms.params[ j ].uniform_name == binding_that->name
         && builder->state.uniforms.params[ j ].cls          == VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE )
            {
            if( used_cnt >= cnt_of_array( shader->param_pool ) )
                {
                debug_assert_always();
                return( FALSE );
                }

            binding_this->parameters = &shader->param_pool[ used_cnt++ ];

            binding_this->parameters->cls   = VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE;
            binding_this->parameters->image = builder->state.uniforms.params[ j ].image;
            break;
            }
        }

    if( binding_this->parameters )
        {
        continue;
        }

    /*------------------------------------------------------
    Vectors binding
    ------------------------------------------------------*/
    head = &binding_this->parameters;
    for( j = 0; j < builder->state.uniforms.count; j++ )
        {
        if( builder->state.uniforms.params[ j ].uniform_name == binding_that->name
         && builder->state.uniforms.params[ j ].cls          == VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR )
            {
            if( used_cnt >= cnt_of_array( shader->param_pool )
             || builder->state.uniforms.params[ j ].vector.vector < VKN_SHADER_PARAM_SET_LIMITS[ binding_this->set ].first
             || builder->state.uniforms.params[ j ].vector.vector > VKN_SHADER_PARAM_SET_LIMITS[ binding_this->set ].last )
                {
                debug_assert_always();
                return( FALSE );
                }

            new_param = &shader->param_pool[ used_cnt++ ];
            *head     = new_param;
            head      = &new_param->vector.next;

            new_param->cls            = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
            new_param->vector.vector = builder->state.uniforms.params[ j ].vector.vector;
            new_param->vector.width  = builder->state.uniforms.params[ j ].vector.width;
            }
        }
    }

/*----------------------------------------------------------
Push constants
----------------------------------------------------------*/
for( i = 0; i < builder->state.push_constants.count; i++ )
    {
    constant_that = &builder->state.push_constants.constants[ i ];
    if( shader->push_constants.count >= cnt_of_array( shader->push_constants.constants ) )
        {
        debug_assert_always();
        return( FALSE );
        }

    constant_this = &shader->push_constants.constants[ shader->push_constants.count++ ];
    constant_this->offset = constant_that->constant.offset;
    constant_this->size   = constant_that->constant.size;

    /*------------------------------------------------------
    Vectors binding
    ------------------------------------------------------*/
    head       = &constant_this->parameters;
    vectors_sz = 0;
    for( j = 0; j < builder->state.uniforms.count; j++ )
        {
        debug_assert( builder->state.uniforms.params[ j ].uniform_name != constant_that->name
                   || builder->state.uniforms.params[ j ].cls          != VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE );

        if( builder->state.uniforms.params[ j ].uniform_name == constant_that->name
         && builder->state.uniforms.params[ j ].cls          == VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR )
            {
            if( used_cnt >= cnt_of_array( shader->param_pool ) )
                {
                debug_assert_always();
                return( FALSE );
                }

            new_param = &shader->param_pool[ used_cnt++ ];
            *head     = new_param;
            head      = &new_param->vector.next;

            new_param->cls           = VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR;
            new_param->vector.vector = builder->state.uniforms.params[ j ].vector.vector;
            new_param->vector.width  = builder->state.uniforms.params[ j ].vector.width;

            vectors_sz += sizeof( float ) * new_param->vector.width;
            break;
            }
        }

    debug_assert( !constant_this->parameters
               || vectors_sz == constant_this->size );

    }

return( TRUE );

}   /* resolve_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       retrieve_layout
*
*********************************************************************/

static bool retrieve_layout
    (
    VKN_shader_build_type
                       *builder    /* shader builder               */
    )
{
clr_struct( &builder->state.descriptor_bindings );
clr_struct( &builder->state.push_constants );

/*----------------------------------------------------------
Query the layout counts
----------------------------------------------------------*/
if( !VKN_shader_reflect_spirv_w_overrides( builder->state.code,
                                           builder->state.code_sz,
                                           NULL,
                                           0,
                                           NULL,
                                           &builder->state.descriptor_bindings.count,
                                           NULL,
                                           &builder->state.push_constants.count ) )
    {
    return( FALSE );
    }

if( builder->state.descriptor_bindings.count > cnt_of_array( builder->state.descriptor_bindings.bindings ) )
    {
    debug_assert_always();
    builder->state.descriptor_bindings.count = cnt_of_array( builder->state.descriptor_bindings.bindings );
    }

if( builder->state.push_constants.count > cnt_of_array( builder->state.push_constants.constants ) )
    {
    debug_assert_always();
    builder->state.push_constants.count = cnt_of_array( builder->state.push_constants.constants );
    }

/*----------------------------------------------------------
Retrieve the layout
----------------------------------------------------------*/
if( !VKN_shader_reflect_spirv_w_overrides( builder->state.code,
                                           builder->state.code_sz,
                                           builder->state.overrides.overrides,
                                           builder->state.overrides.count,
                                           builder->state.descriptor_bindings.bindings,
                                           &builder->state.descriptor_bindings.count,
                                           builder->state.push_constants.constants,
                                           &builder->state.push_constants.count ) )
    {
    return( FALSE );
    }

return( TRUE );

}   /* retrieve_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_allocation_callbacks
*
*********************************************************************/

static VKN_SHADER_CONFIG_API set_allocation_callbacks
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
builder->state.allocator = allocator;

return( builder->config );

}   /* set_allocation_callbacks() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_code
*
*********************************************************************/

static VKN_SHADER_CONFIG_API set_code
    (
    const u32          *code,       /* shader bytecode              */
    const u32           code_sz,    /* shader code byte size        */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    )
{
builder->state.code        = code;
builder->state.code_sz     = code_sz;
builder->state.have_layout = retrieve_layout( builder );

return( builder->config );

}   /* set_code() */
