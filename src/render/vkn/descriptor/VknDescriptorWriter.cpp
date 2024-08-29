#include "Global.hpp"
#include "Utilities.hpp"

#include "VknArena.hpp"
#include "VknCommon.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknDescriptorWriterTypes.hpp"
#include "VknDescriptorWriter.hpp"
#include "VknShaderParamTypes.hpp"

/*------------------------------------------------------------------------------------------
                                   DESIGN ASSURANCE LEVEL
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                     LITERAL CONSTANTS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                           TYPES
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                      MEMORY CONSTANTS
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                         VARIABLES
------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
                                           MACROS
------------------------------------------------------------------------------------------*/

/*********************************************************************
*
*   PROCEDURE NAME:
*       bind_descriptor_set
*
*********************************************************************/

static __inline void bind_descriptor_set
    (
    const u8            index,      /* set index                    */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
vkCmdBindDescriptorSets( writer->state.commands, VK_PIPELINE_BIND_POINT_GRAPHICS, writer->state.pipeline_layout, index, 1, &writer->state.sets[ index ], 0, NULL );
writer->state.set_disturbed[ index ] = FALSE;

}   /* bind_descriptor_set() */


/*------------------------------------------------------------------------------------------
                                         PROCEDURES
------------------------------------------------------------------------------------------*/

static VKN_descriptor_writer_begin_proc_type begin;
static VKN_descriptor_writer_begin_frame_proc_type begin_frame;
static VKN_descriptor_writer_bind_buffer_proc_type bind_buffer;
static VKN_descriptor_writer_bind_image_proc_type bind_image;
static VKN_descriptor_writer_clean_image_proc_type clean_image;
static VKN_descriptor_writer_clean_vector_proc_type clean_vector;

static void commit_copies
    (
    const u8            set,        /* set index                    */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

static VKN_descriptor_writer_copy_descriptor_proc_type copy_descriptor;

static void create_descriptor_set
    (
    const u8            index,      /* set index                    */
    const VkDescriptorSetLayout
                        layout,     /* set layout                   */
    const u32           fingerprint,/* set layout fingerprint       */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

static VKN_descriptor_writer_end_proc_type end;
static VKN_descriptor_writer_get_parameters_proc_type get_parameters;
static VKN_descriptor_writer_is_image_dirty_proc_type is_image_dirty;
static VKN_descriptor_writer_is_vector_dirty_proc_type is_vector_dirty;
static VKN_descriptor_writer_set_descriptor_set_proc_type set_descriptor_set;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_descriptor_writer_create
*
*   DESCRIPTION:
*       Create a descriptor writer.
*
*********************************************************************/

void VKN_descriptor_writer_create
    (
    const VkDevice     logical,     /* logical device               */
    VKN_shader_param_type
                       *parameters, /* shader parameters            */
    VKN_descriptor_pool_type
                       *pool,       /* descriptor pool              */
    VKN_descriptor_writer_type
                       *writer      /* output new descriptor writer */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_descriptor_writer_api_type API =
    {
    begin,
    begin_frame,
    bind_buffer,
    bind_image,
    clean_image,
    clean_vector,
    copy_descriptor,
    end,
    get_parameters,
    is_image_dirty,
    is_vector_dirty,
    set_descriptor_set
    };

/*----------------------------------------------------------
Create the descriptor pool
----------------------------------------------------------*/
clr_struct( writer );
writer->i = &API;

writer->state.pool       = pool;
writer->state.parameters = parameters;
writer->state.logical    = logical;

}   /* VKN_descriptor_writer_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       begin
*
*********************************************************************/

static void begin
    (
    VkCommandBuffer     commands,   /* buffer to use for writing    */
    VkPipeline          pipeline,   /* handle to pipeline           */
    VkPipelineLayout    layout,     /* shape of all descriptor sets */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
bool                    is_new_pipeline;
                                    /* did the pipeline change?     */

is_new_pipeline = writer->state.pipeline != pipeline;

writer->state.commands        = commands;
writer->state.pipeline        = pipeline;
writer->state.pipeline_layout = layout;

if( is_new_pipeline )
    {
    vkCmdBindPipeline( commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
    for( i = 0; i < cnt_of_array( writer->state.set_disturbed ); i++ )
        {
        writer->state.set_disturbed[ i ] = TRUE;
        }
    }

}   /* begin() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       begin_frame
*
*********************************************************************/

static void begin_frame
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
writer->state.commands        = VK_NULL_HANDLE;
writer->state.pipeline        = VK_NULL_HANDLE;
writer->state.pipeline_layout = VK_NULL_HANDLE;
clr_array( writer->state.set_fingerprints );

}   /* begin_frame() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       bind_buffer
*
*********************************************************************/

static void bind_buffer
    (
    const u8            set,        /* set index                    */
    const u32           binding,    /* binding index                */
    VkBuffer            buffer,     /* buffer containing update     */
    VkDeviceSize        offset,     /* start of update              */
    VkDeviceSize        size,       /* size of update               */
    VKN_arena_type     *arena,      /* arena for storage            */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDescriptorBufferInfo *info;       /* buffer info                  */
VkWriteDescriptorSet   *write;      /* new write                    */

commit_copies( set, writer );
if( writer->state.write_cnt >= cnt_of_array( writer->state.writes ) )
    {
    debug_assert_always();
    return;
    }

info = VKN_arena_allocate_struct( VkDescriptorBufferInfo, arena );
if( !info )
    {
    debug_assert_always();
    return;
    }

clr_struct( info );
info->buffer = buffer;
info->offset = offset;
info->range  = size;

write = &writer->state.writes[ writer->state.write_cnt++ ];

clr_struct( write );
write->sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
write->dstSet          = writer->state.sets[ set ];
write->dstBinding      = binding;
write->dstArrayElement = 0;
write->descriptorCount = 1;
write->descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
write->pBufferInfo     = info;

}   /* bind_buffer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       bind_image
*
*********************************************************************/

static void bind_image
    (
    const u8            set,        /* set index                    */
    const u32           binding,    /* binding index                */
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    VKN_arena_type     *arena,      /* arena for storage            */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDescriptorImageInfo  *info;       /* image info                   */
VkWriteDescriptorSet   *write;      /* new write                    */

commit_copies( set, writer );
if( writer->state.write_cnt >= cnt_of_array( writer->state.writes ) )
    {
    debug_assert( writer->state.write_cnt < cnt_of_array( writer->state.writes ) );
    return;
    }

info = VKN_arena_allocate_struct( VkDescriptorImageInfo, arena );
if( !info )
    {
    debug_assert_always();
    return;
    }

clr_struct( info );
info->sampler     = writer->state.parameters->images[ name ].sampler;
info->imageView   = writer->state.parameters->images[ name ].view;
info->imageLayout = writer->state.parameters->images[ name ].layout;

write = &writer->state.writes[ writer->state.write_cnt++ ];

clr_struct( write );
write->sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
write->dstSet          = writer->state.sets[ set ];
write->dstBinding      = binding;
write->dstArrayElement = 0;
write->descriptorCount = 1;
write->descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
write->pImageInfo      = info;

}   /* bind_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       commit_copies
*
*********************************************************************/

static void commit_copies
    (
    const u8            set,        /* set index                    */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

if( writer->state.dirty_copies_start == writer->state.copy_cnt )
    {
    return;
    }

create_descriptor_set( set, writer->state.set_layouts[ set ], writer->state.set_fingerprints[ set ], writer );
for( i = writer->state.dirty_copies_start; i < writer->state.copy_cnt; i++ )
    {
    writer->state.copies[ i ].dstSet = writer->state.sets[ set ];
    }

writer->state.dirty_copies_start = writer->state.copy_cnt;

}   /* commit_copies() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       clean_image
*
*********************************************************************/

static void clean_image
    (
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
VKN_shader_param_set_image_is_dirty( FALSE, name, writer->state.parameters );

}   /* clean_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       clean_vector
*
*********************************************************************/

static void clean_vector
    (
    const VKN_shader_param_vector_name_type
                        name,       /* vector name                  */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
VKN_shader_param_set_vector_is_dirty( FALSE, name, writer->state.parameters );

}   /* clean_vector() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       copy_descriptor
*
*********************************************************************/

static void copy_descriptor
    (
    const u8            index,      /* set index                    */
    const u32           binding,    /* binding index                */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCopyDescriptorSet    *copy;       /* new copy                     */

if( writer->state.copy_cnt >= cnt_of_array( writer->state.copies ) )
    {
    debug_assert_always();
    return;
    }

copy = &writer->state.copies[ writer->state.copy_cnt++ ];

clr_struct( copy );
copy->sType           = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
copy->srcSet          = writer->state.sets[ index ];
copy->srcBinding      = binding;
copy->srcArrayElement = 0;
copy->dstSet          = VK_NULL_HANDLE;
copy->dstBinding      = binding;
copy->dstArrayElement = 0;
copy->descriptorCount = 1;

}   /* copy_descriptor() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       create_descriptor_set
*
*********************************************************************/

static void create_descriptor_set
    (
    const u8            index,      /* set index                    */
    const VkDescriptorSetLayout
                        layout,     /* set layout                   */
    const u32           fingerprint,/* set layout fingerprint       */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Create a new descriptor set and bind it
----------------------------------------------------------*/
writer->state.sets[ index ]             = writer->state.pool->i->allocate_set( layout, writer->state.pool );
writer->state.set_fingerprints[ index ] = fingerprint;
writer->state.set_layouts[ index ]      = layout;

bind_descriptor_set( index, writer );

/*----------------------------------------------------------
Invalidate all parameters that could have been bound to this
set
----------------------------------------------------------*/
for( i = (u32)VKN_SHADER_PARAM_SET_LIMITS[ index ].first; i <= (u32)VKN_SHADER_PARAM_SET_LIMITS[ index ].last; i++ )
    {
    VKN_shader_param_set_vector_is_dirty( TRUE, (VKN_shader_param_vector_name_type)i, writer->state.parameters );
    }

for( i = 0; i < cnt_of_array( writer->state.parameters->images ); i++ )
    {
    VKN_shader_param_set_image_is_dirty( TRUE, (VKN_shader_param_image_name_type)i, writer->state.parameters );
    }

}   /* create_descriptor_set() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       end
*
*********************************************************************/

static void end
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Rollback uncommited copies
----------------------------------------------------------*/
writer->state.copy_cnt = writer->state.dirty_copies_start;

/*----------------------------------------------------------
Update the descriptor sets
----------------------------------------------------------*/
if( writer->state.write_cnt )
    {
    vkUpdateDescriptorSets( writer->state.logical, writer->state.write_cnt, writer->state.writes, writer->state.copy_cnt, writer->state.copies );
    }

writer->state.write_cnt          = 0;
writer->state.copy_cnt           = 0;
writer->state.dirty_copies_start = 0;

}   /* end() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_parameters
*
*********************************************************************/

static const VKN_shader_param_type * get_parameters
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
return( writer->state.parameters );

}   /* get_parameters() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_image_dirty
*
*********************************************************************/

static bool is_image_dirty
    (
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
return( VKN_shader_param_get_image_is_dirty( name, writer->state.parameters ) );

}   /* is_image_dirty() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_vector_dirty
*
*********************************************************************/

static bool is_vector_dirty
    (
    const VKN_shader_param_vector_name_type
                        name,       /* vector name                  */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
return( VKN_shader_param_get_vector_is_dirty( name, writer->state.parameters ) );

}   /* is_vector_dirty() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       set_descriptor_set
*
*********************************************************************/

static void set_descriptor_set
    (
    const u8            index,      /* set index                    */
    const VkDescriptorSetLayout
                        layout,     /* set layout                   */
    const u32           fingerprint,/* set layout fingerprint       */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    )
{
/*----------------------------------------------------------
Rollback uncommited copies
----------------------------------------------------------*/
writer->state.copy_cnt = writer->state.dirty_copies_start;

/*----------------------------------------------------------
Check for compatibility
----------------------------------------------------------*/
if( writer->state.set_fingerprints[ index ] == fingerprint )
    {
    if( writer->state.set_disturbed[ index ] )
        {
        bind_descriptor_set( index, writer );
        }

    return;
    }

/*----------------------------------------------------------
Need a new descriptor set
----------------------------------------------------------*/
create_descriptor_set( index, layout, fingerprint, writer );

}   /* set_descriptor_set() */

