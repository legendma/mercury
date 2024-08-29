#pragma once

#include "VknArenaTypes.hpp"
#include "VknBufferUniformTypes.hpp"
#include "VknDescriptorPoolTypes.hpp"
#include "VknDescriptorWriterTypes.hpp"
#include "VknEffectTypes.hpp"
#include "VknPipelineGraphicsTypes.hpp"
#include "VknReleaserTypes.hpp"
#include "VknShaderParamTypes.hpp"
#include "VknVertexTypes.hpp"


#define VKN_PROGRAM_MAX_PIPELINES_CNT \
                                    ( 30 )


typedef const VKN_pipeline_graphics_type * VKN_program_get_pipeline_proc_type
    (
    const u32           vertex_index,
                                    /* index of vertex in defines   */
    const VKN_render_flags_type
                        flags,      /* render state flags           */
    VKN_pipeline_graphics_build_type
                       *builder,    /* pipeline builder             */
    struct _VKN_program_type
                       *program     /* program                      */
    );

typedef void VKN_program_unload_proc_type
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    struct _VKN_program_type
                       *program     /* program                      */
    );

typedef void VKN_program_write_descriptors_proc_type
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
    );

typedef void VKN_program_write_push_constants_proc_type
    (
    VkCommandBuffer     commands,   /* command buffer               */
    VKN_arena_type     *scratch,    /* scratch arena                */
    const VKN_shader_param_type
                       *parameters, /* shader parameters            */
    struct _VKN_program_type
                       *program     /* program                      */
    );

typedef struct
    {
    VKN_program_get_pipeline_proc_type
                       *get_pipeline;
                                    /* get program pipeline         */
    VKN_program_unload_proc_type
                       *unload;     /* unload all cached pipelines  */
    VKN_program_write_descriptors_proc_type
                       *write_descriptors;
                                    /* write descriptor sets        */
    VKN_program_write_push_constants_proc_type
                       *write_push_constants;
                                    /* write push constants         */
    } VKN_program_api_type;

typedef struct
    {
    u32                 vertex_index;
                                    /* vertex description index     */
    VKN_pipeline_graphics_type    
                        pipeline;   /* pipeline object              */
    } VKN_program_pipeline_type;

typedef struct
    {
    VKN_program_pipeline_type       /* current pipelines            */
                        pipelines[ VKN_PROGRAM_MAX_PIPELINES_CNT ];
    u32                 count;      /* number of pipelines          */
    } VKN_program_pipelines_type;

typedef struct
    {
    const VKN_vertex_type
                       *vertices;   /* vertex type definitions      */
    const VKN_effect_type
                       *effect;     /* underlying effect            */
    VKN_program_pipelines_type
                        pipelines;  /* pipeline store               */
    } VKN_program_state_type;

typedef struct _VKN_program_type
    {
    const VKN_program_api_type
                       *i;          /* program interface            */
    VKN_program_state_type
                        state;      /* private state                */
    } VKN_program_type;
