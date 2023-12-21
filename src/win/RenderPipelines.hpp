#pragma once

#include <d3d12.h>

#include "HashMap.hpp"
#include "RenderModels.hpp"
#include "RenderShaders.hpp"

#define RENDER_PIPELINES_MAX_COUNT     ( 10 )

namespace RenderPipelines
{
static const char *BUILDER_NAME_DEFAULT = "default_builder";

static const char *ALL_BUILDERS[] =
    {
    BUILDER_NAME_DEFAULT
    };

static const char *EFFECT_NAME_DEFAULT  = "default_effect";

typedef enum _SlotName
    {
    SLOT_NAME_PER_OBJECT,
    SLOT_NAME_MATERIALS,
    SLOT_NAME_PER_PASS,
    SLOT_NAME_PER_FRAME,
    /* count */
    SLOT_NAME_COUNT
    } SlotName;

typedef struct _PipelineBuilder
    {
    ID3D12RootSignature
                       *root_signature;
    D3D12_BLEND_DESC    blending;
    D3D12_RASTERIZER_DESC
                        rasterizer;
    D3D12_DEPTH_STENCIL_DESC
                        depth_stencil;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE
                        primitive_topology;
    RenderModels::VertexDescriptor
                        vertex_descriptor;
    RenderShaders::_ShaderEffect
                       *effect;
    DXGI_FORMAT         rt_formats[ cnt_of_array( ( (D3D12_GRAPHICS_PIPELINE_STATE_DESC*)NULL )->RTVFormats ) ];
    uint32_t            num_render_targets;
    } PipelineBuilder;

HASH_MAP_IMPLEMENT( BuilderMap, RENDER_PIPELINES_MAX_COUNT, PipelineBuilder );
HASH_MAP_IMPLEMENT( EffectMap, RENDER_PIPELINES_MAX_COUNT, RenderShaders::ShaderEffect );

typedef struct _Pipelines
    {
    BuilderMap          builders;
    EffectMap           effects;
    RenderShaders::ShaderCache
                        shader_cache;
    } Pipelines;


ID3D12PipelineState * PipelineBuilder_BuildPipeline( const PipelineBuilder *builder, ID3D12Device *device );

void                  Pipelines_Destroy( Pipelines *pipelines );
PipelineBuilder *     Pipelines_GetBuilder( const char *name, Pipelines *pipelines );
bool                  Pipelines_Init( ID3D12Device *device, Pipelines *pipelines );


} /* namespace RenderPipelines */