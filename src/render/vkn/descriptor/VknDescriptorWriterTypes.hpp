#pragma once

#include "Global.hpp"

#include "VknArenaTypes.hpp"
#include "VknCommon.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknShaderParamTypes.hpp"


#define VKN_DESCRIPTOR_WRITER_MAX_DESCRIPTOR_SET_WRITE_CNT \
                                    ( 50 )

#define VKN_DESCRIPTOR_WRITER_MAX_DESCRIPTOR_SET_COPY_CNT \
                                    VKN_DESCRIPTOR_WRITER_MAX_DESCRIPTOR_SET_WRITE_CNT


typedef void VKN_descriptor_writer_begin_proc_type
    (
    VkCommandBuffer     commands,   /* buffer to use for writing    */
    VkPipeline          pipeline,   /* handle to pipeline           */
    VkPipelineLayout    layout,     /* shape of all descriptor sets */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_begin_frame_proc_type
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_bind_buffer_proc_type
    (
    const u8            set,        /* set index                    */
    const u32           binding,    /* binding index                */
    VkBuffer            buffer,     /* buffer containing update     */
    VkDeviceSize        offset,     /* start of update              */
    VkDeviceSize        size,       /* size of update               */
    VKN_arena_type     *arena,      /* arena for storage            */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_bind_image_proc_type
    (
    const u8            set,        /* set index                    */
    const u32           binding,    /* binding index                */
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    VKN_arena_type     *arena,      /* arena for storage            */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_clean_image_proc_type
    (
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_clean_vector_proc_type
    (
    const VKN_shader_param_vector_name_type
                        name,       /* vector name                  */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_copy_descriptor_proc_type
    (
    const u8            index,      /* set index                    */
    const u32           binding,    /* binding index                */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_end_proc_type
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef const VKN_shader_param_type * VKN_descriptor_writer_get_parameters_proc_type
    (
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef bool VKN_descriptor_writer_is_image_dirty_proc_type
    (
    const VKN_shader_param_image_name_type
                        name,       /* image name                   */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef bool VKN_descriptor_writer_is_vector_dirty_proc_type
    (
    const VKN_shader_param_vector_name_type
                        name,       /* vector name                  */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef void VKN_descriptor_writer_set_descriptor_set_proc_type
    (
    const u8            index,      /* set index                    */
    const VkDescriptorSetLayout
                        layout,     /* set layout                   */
    const u32           fingerprint,/* set layout fingerprint       */
    struct _VKN_descriptor_writer_type
                       *writer      /* descriptor writer            */
    );

typedef struct _VKN_descriptor_writer_api_type
    {
    VKN_descriptor_writer_begin_proc_type
                       *begin;
    VKN_descriptor_writer_begin_frame_proc_type
                       *begin_frame;
    VKN_descriptor_writer_bind_buffer_proc_type
                       *bind_buffer;
    VKN_descriptor_writer_bind_image_proc_type
                       *bind_image;
    VKN_descriptor_writer_clean_image_proc_type
                       *clean_image;
    VKN_descriptor_writer_clean_vector_proc_type
                       *clean_vector;
    VKN_descriptor_writer_copy_descriptor_proc_type
                       *copy_descriptor;
    VKN_descriptor_writer_end_proc_type
                       *end;
    VKN_descriptor_writer_get_parameters_proc_type
                       *get_parameters;
    VKN_descriptor_writer_is_image_dirty_proc_type
                       *is_image_dirty;
    VKN_descriptor_writer_is_vector_dirty_proc_type
                       *is_vector_dirty;
    VKN_descriptor_writer_set_descriptor_set_proc_type
                       *set_descriptor_set;
    } VKN_descriptor_writer_api_type;

typedef struct
    {
    u16                 write_cnt;
    u16                 copy_cnt;
    u16                 dirty_copies_start;
    VkDevice            logical;
    VkPipeline          pipeline;
    VkPipelineLayout    pipeline_layout;
    u32                 set_fingerprints[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* current set fingerprints     */
    bool                set_disturbed[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* flag for disturbed status    */
    VkDescriptorSetLayout
                        set_layouts[ VKN_DESCRIPTOR_SET_CNT ];
    VkDescriptorSet     sets[ VKN_DESCRIPTOR_SET_CNT ];
    VKN_shader_param_type
                       *parameters; /* shader parameters            */
    VkCopyDescriptorSet copies[ VKN_DESCRIPTOR_WRITER_MAX_DESCRIPTOR_SET_COPY_CNT ];
                                    /* pending descriptor copies    */
    VkWriteDescriptorSet            /* pending descriptor writes    */
                        writes[ VKN_DESCRIPTOR_WRITER_MAX_DESCRIPTOR_SET_WRITE_CNT ];
    VKN_descriptor_pool_type
                       *pool;       /* descriptor pool              */
    VkCommandBuffer     commands;   /* command buffer               */
    } VKN_descriptor_writer_state_type;

typedef struct _VKN_descriptor_writer_type
    {
    const VKN_descriptor_writer_api_type
                       *i;          /* writer interface             */
    VKN_descriptor_writer_state_type
                        state;      /* writer state                 */
    } VKN_descriptor_writer_type;
