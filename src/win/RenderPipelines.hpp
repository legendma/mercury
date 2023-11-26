#pragma once

#include <d3d12.h>

#include "RenderModels.hpp"
//#include "RenderShaders.hpp"

namespace RenderShaders { struct _ShaderEffect; }
namespace RenderPipelines
{
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
    RenderShaders::_ShaderEffect       *shaders;
    } PipelineBuilder;

ID3D12PipelineState * PipelineBuilder_BuildPipeline( const PipelineBuilder *builder, ID3D12Device *device );


} /* namespace RenderPipelines */