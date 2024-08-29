#include "Global.hpp"
#include "Utilities.hpp"

#include "VknArena.hpp"
#include "VknPipelineGraphics.hpp"
#include "VknProgramTypes.hpp"


static VKN_pipeline_graphics_type * create_pipeline
    (
    const u32           vertex_index,
                                    /* index of vertex in defines   */
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    VKN_pipeline_graphics_build_type
                       *builder,    /* pipeline builder             */
    VKN_program_type   *program     /* program                      */
    );

static VKN_program_get_pipeline_proc_type get_pipeline;
static VKN_program_unload_proc_type unload;
static VKN_program_write_descriptors_proc_type write_descriptors;

static void write_image
    (
    const u8            set,        /* set index                    */
    const VKN_effect_parameter_mapping_type
                       *param,      /* image parameter              */
    VKN_arena_type     *scratch,    /* scratch arena                */
    VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

static VKN_program_write_push_constants_proc_type write_push_constants;

static const VKN_effect_parameter_mapping_type * write_uniform_vectors
    (
    const u8            set,        /* set index                    */
    const VKN_effect_parameter_mapping_type
                       *start,      /* start parameter              */
    VKN_arena_type     *scratch,    /* scratch arena                */
    VKN_buffer_uniform_type
                       *buffer,     /* uniform buffer               */
    VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_program_create
*
*   DESCRIPTION:
*       Create a program object.
*
*********************************************************************/

void VKN_program_create
    (
    const VKN_vertex_type
                       *vertices,   /* vertex type definitions      */
    const VKN_effect_type
                       *effect,     /* underlying effect            */
    VKN_program_type   *program     /* output new program           */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_program_api_type API =
    {
    get_pipeline,
    unload,
    write_descriptors,
    write_push_constants
    };

/*----------------------------------------------------------
Create the program
----------------------------------------------------------*/
clr_struct( program );
program->i = &API;

program->state.effect   = effect;
program->state.vertices = vertices;

}   /* VKN_program_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_program_destroy
*
*   DESCRIPTION:
*       Destroy the given program.
*
*********************************************************************/

void VKN_program_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_program_type   *program     /* program to destroy           */
    )
{
unload( releaser, program );
clr_struct( &program );

}   /* VKN_program_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_pipeline
*
*********************************************************************/

static VKN_pipeline_graphics_type * create_pipeline
    (
    const u32           vertex_index,
                                    /* index of vertex in defines   */
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    VKN_pipeline_graphics_build_type
                       *builder,    /* pipeline builder             */
    VKN_program_type   *program     /* program                      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_attribute_type
                       *attribute;  /* vertex attribute             */
VKN_vertex_binding_type
                       *binding;    /* vertex binding               */
u32                     i;          /* loop counter                 */
VKN_program_pipeline_type          /* newly created pipeline       */
                       *new_pipeline;
const VKN_vertex_type  *vertex;     /* vertex desription            */

vertex = &program->state.vertices[ vertex_index ];

/*----------------------------------------------------------
Tentatively obtain a pipeline object
----------------------------------------------------------*/
if( program->state.pipelines.count >= cnt_of_array( program->state.pipelines.pipelines ) )
    {
    debug_assert_always();
    return( NULL );
    }

new_pipeline = &program->state.pipelines.pipelines[ program->state.pipelines.count ];

/*----------------------------------------------------------
Configure the builder
----------------------------------------------------------*/
builder->config->reset( builder );
for( binding = vertex->bindings; binding; binding = binding->next )
    {
    builder->config->add_vertex_binding( binding->binding.binding,
                                         binding->binding.stride,
                                         binding->binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE,
                                         builder );
    }
    
for( attribute = vertex->attributes; attribute; attribute = attribute->next )
    {
    builder->config->add_vertex_attribute( attribute->attribute.location,
                                           attribute->attribute.binding,
                                           attribute->attribute.format,
                                           attribute->attribute.offset,
                                           builder );
    }

for( i = 0; i < program->state.effect->stage_cnt; i++ )
    {
    builder->config->add_stage( program->state.effect->stages[ i ].stage,
                                program->state.effect->stages[ i ].shader,
                                program->state.effect->stages[ i ].flags,
                                program->state.effect->stages[ i ].entry_point,
                                builder );
    }

/*----------------------------------------------------------
Create the pipeline
----------------------------------------------------------*/
if( !VKN_pipeline_graphics_create( flags, program->state.effect->layout, builder, &new_pipeline->pipeline) )
    {
    debug_assert_always();
    return( NULL );
    }

VKN_name_object( program->state.effect->logical, new_pipeline->pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, program->state.effect->debug_name );

program->state.pipelines.count++;
new_pipeline->vertex_index = vertex_index;

return( &new_pipeline->pipeline );

}   /* create_pipeline() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_pipeline
*
*********************************************************************/

static const VKN_pipeline_graphics_type * get_pipeline
    (
    const u32           vertex_index,
                                    /* index of vertex in defines   */
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    VKN_pipeline_graphics_build_type
                       *builder,    /* pipeline builder             */
    struct _VKN_program_type
                       *program     /* program                      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_render_flags_type   compare_flags;
                                    /* flags to compare             */
u32                     i;          /* loop counter                 */
VKN_pipeline_graphics_type
                       *pipeline;   /* working pipeline             */
VKN_program_pipeline_type
                       *search;     /* searching pipeline           */

/*----------------------------------------------------------
Check if the pipeline exists
----------------------------------------------------------*/
pipeline = NULL;
for( i = 0; i < program->state.pipelines.count; i++ )
    {
    search = &program->state.pipelines.pipelines[ i ];
    compare_flags = flags & search->pipeline.render_flags_mask;
    if( compare_flags == search->pipeline.render_flags
     && vertex_index == search->vertex_index ) // TODO <MPA> - Dynamic rendering complicates this matching.  Probably need to hash the attachment formats to properly match
        {
        pipeline = &search->pipeline;
        break;
        }
    }

/*----------------------------------------------------------
Otherwise create it
----------------------------------------------------------*/
if( !pipeline )
    {
    pipeline = create_pipeline( vertex_index, flags, builder, program );
    }

if( !pipeline )
    {
    return( VK_NULL_HANDLE );
    }

return( pipeline );

}   /* get_pipeline() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       unload
*
*********************************************************************/

static void unload
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    struct _VKN_program_type
                       *program     /* program                      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

for( i = 0; i < program->state.pipelines.count; i++ )
    {
    VKN_pipeline_graphics_destroy( releaser, &program->state.pipelines.pipelines[ i ].pipeline );
    }

program->state.pipelines.count = 0;

}   /* unload() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       write_descriptors
*
*********************************************************************/

static void write_descriptors
    (
    VkCommandBuffer     commands,   /* command buffer               */
    VkPipeline          pipeline,   /* pipeline                     */
    VKN_arena_type     *scratch,    /* scratch arena                */
    VKN_buffer_uniform_type
                       *uniforms,   /* uniform buffer               */
    VKN_descriptor_writer_type
                       *writer,     /* descriptor writer            */
    struct _VKN_program_type
                       *program     /* program                      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u8                      i;          /* loop counter                 */
const VKN_effect_parameter_mapping_type
                       *param;      /* working parameter            */
VKN_arena_token_type    token;      /* reset token                  */

token = VKN_arena_get_token( scratch );

writer->i->begin( commands, pipeline, program->state.effect->layout, writer );
for( i = 0; i < cnt_of_array( program->state.effect->set_params ); i++ )
    {
    writer->i->set_descriptor_set( i, program->state.effect->sets[ i ], program->state.effect->set_fingerprints[ i ], writer );
    for( param = program->state.effect->set_params[ i ]; param; param = param->next )
        {
        switch( param->cls )
            {
            case VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE:
                write_image( i, param, scratch, writer );
                break;

            case VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR:
                param = write_uniform_vectors( i, param, scratch, uniforms, writer );
                break;

            default:
                debug_assert_always();
                break;
            }
        }
    }

writer->i->end( writer );

VKN_arena_rewind_by_token( token, scratch );

}   /* write_descriptors() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       write_image
*
*********************************************************************/

static void write_image
    (
    const u8            set,        /* set index                    */
    const VKN_effect_parameter_mapping_type
                       *param,      /* image parameter              */
    VKN_arena_type     *scratch,    /* scratch arena                */
    VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
if( !writer->i->is_image_dirty( param->image, writer ) )
    {
    writer->i->copy_descriptor( set, param->binding, writer );
    return;
    }

writer->i->bind_image( set, param->binding, param->image, scratch, writer );
writer->i->clean_image( param->image, writer );

}   /* write_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       write_push_constants
*
*********************************************************************/

static void write_push_constants
    (
    VkCommandBuffer     commands,   /* command buffer               */
    VKN_arena_type     *scratch,    /* scratch arena                */
    const VKN_shader_param_type
                       *parameters, /* shader parameters            */
    struct _VKN_program_type
                       *program     /* program                      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
float                  *data;       /* push constant data           */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
u32                     offset;     /* copy offset                  */
const VKN_effect_parameter_mapping_type
                       *param;      /* working parameter            */
const VkPushConstantRange
                       *range;      /* working push constant        */
u32                     size;       /* allocation size              */
VKN_arena_token_type    token;      /* reset token                  */

for( i = 0; i < program->state.effect->push_constant_cnt; i++ )
    {
    token = VKN_arena_get_token( scratch );
    VKN_arena_align( sizeof( *parameters->vectors ), scratch );

    /*------------------------------------------------------
    Size
    ------------------------------------------------------*/
    size = 0;
    for( param = program->state.effect->push_constant_params[ i ]; param; param = param->next )
        {
        size = VKN_uniform_get_size( param->vector.width, size );
        }

    /*------------------------------------------------------
    Copy aligned dense
    ------------------------------------------------------*/
    data   = (float*)VKN_arena_allocate( size, scratch );
    offset = 0;
    for( param = program->state.effect->push_constant_params[ i ]; param; param = param->next )
        {
        offset += VKN_uniform_get_align_adjust( param->vector.width, offset );
        for( j = 0; j < param->vector.width; j++ )
            {
            data[ j + offset / sizeof( *data ) ] = parameters->vectors[ param->vector.vector ].v[ j ];
            }
        }

    /*------------------------------------------------------
    Write
    ------------------------------------------------------*/
    range = &program->state.effect->push_constants[ i ];
    vkCmdPushConstants( commands, program->state.effect->layout, range->stageFlags, range->offset, range->size, data );

    VKN_arena_rewind_by_token( token, scratch );
    }

}   /* write_push_constants() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       write_uniform_vectors
*
*********************************************************************/

static const VKN_effect_parameter_mapping_type * write_uniform_vectors
    (
    const u8            set,        /* set index                    */
    const VKN_effect_parameter_mapping_type
                       *start,      /* start parameter              */
    VKN_arena_type     *scratch,    /* scratch arena                */
    VKN_buffer_uniform_type
                       *buffer,     /* uniform buffer               */
    VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    needs_update;
                                    /* buffer needs update?         */
const VKN_effect_parameter_mapping_type
                       *param;      /* working parameter            */
const VKN_effect_parameter_mapping_type
                       *ret;        /* working parameter            */
u32                     size;       /* tally size                   */
VKN_buffer_uniform_map_result_type
                        uniform;    /* uniform in buffer            */

/*----------------------------------------------------------
Check that this uniform is dirty, and compute the size
while we are at it
----------------------------------------------------------*/
ret          = start;
size         = 0;
needs_update = FALSE;
for( param = start; param && param->binding == start->binding; param = param->next )
    {
    debug_assert( param->cls == VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR );
    needs_update |= writer->i->is_vector_dirty( param->vector.vector, writer );
    size          = VKN_uniform_get_size( param->vector.width, size );
    ret           = param;
    }

if( !needs_update
 || !start )
    {
    writer->i->copy_descriptor( set, start->binding, writer );
    return( ret );
    }

/*----------------------------------------------------------
Upload to uniform buffer
----------------------------------------------------------*/
uniform = buffer->i->map( size, buffer );
for( param = start; param && param->binding == start->binding; param = param->next )
    {
    buffer->i->copy( writer->i->get_parameters( writer )->vectors[ param->vector.vector ].v, param->vector.width, buffer );
    writer->i->clean_vector( param->vector.vector, writer );
    }

size = buffer->i->unmap( buffer );

/*----------------------------------------------------------
Write the descriptor
----------------------------------------------------------*/
writer->i->bind_buffer( set, start->binding, uniform.buffer, uniform.offset, size, scratch, writer );

return( ret );

}   /* write_uniform_vectors() */