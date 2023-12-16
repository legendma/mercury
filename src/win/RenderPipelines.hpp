#pragma once

#include <d3d12.h>

#include "HashMap.hpp"
#include "RenderModels.hpp"
#include "RenderShaders.hpp"

#define RENDER_PIPELINES_MAX_COUNT     ( 10 )

namespace RenderPipelines
{
static const char *PIPELINE_NAME_DEFAULT_OPAQUE      = "default_opaque";
static const char *PIPELINE_NAME_DEFAULT_TRANSPARENT = "default_transparent";

static const char *ALL_PIPELINES[] =
    {
    PIPELINE_NAME_DEFAULT_OPAQUE,
    PIPELINE_NAME_DEFAULT_TRANSPARENT
    };

typedef enum _BoundTextures
    {
    BOUND_TEXTURES_ALBEDO_MAP,            /* t0 */
    BOUND_TEXTURES_NORMAL_MAP,            /* t1 */
    BOUND_TEXTURES_METALLIC_MAP,          /* t2 */
    BOUND_TEXTURES_ROUGHNESS_MAP,         /* t3 */
    BOUND_TEXTURES_DISPLACEMENT_MAP,      /* t4 */
    BOUND_TEXTURES_AMBIENT_OCCLUSION_MAP, /* t5 */
    /* count */
    BOUND_TEXTURES_COUNT
    } BoundTextures;

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


ID3D12PipelineState *       PipelineBuilder_BuildPipeline( const PipelineBuilder *builder, ID3D12Device *device );

void                        Pipelines_Destroy( Pipelines *pipelines );
bool                        Pipelines_Init( ID3D12Device *device, Pipelines *pipelines );


} /* namespace RenderPipelines */