#pragma once
#include "D3D12Types.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Pipelines
{

static const char *ENGINE_SHADER_NAME_VERTEX_MVP = "shader_vs_mvp";
static const char* ENGINE_SHADER_NAME_PIXEL_FLAT = "shader_ps_flat";
static const char *ENGINE_SHADER_NAMES[] =
    {
    ENGINE_SHADER_NAME_VERTEX_MVP,
    ENGINE_SHADER_NAME_PIXEL_FLAT
    };

static const D3D12_ROOT_SIGNATURE_FLAGS DEFAULT_ROOT_SIGNATURE_FLAGS = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS        
                                                                     | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
                                                                     | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
                                                                     | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
                                                                     | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

#define DECLARE_PIPELINE_STATE_SUBOBJECT( _name, _member, _member_type ) \
    typedef union _concat( _StateSubobject, _name )                      \
        {                                                                \
        void           *dummy;                                           \
        struct                                                           \
            {                                                            \
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE                          \
                       type;                                             \
            _member_type   _member;                                      \
            } s;                                                         \
        } _concat( StateSubobject, _name )

DECLARE_PIPELINE_STATE_SUBOBJECT( RootSignature,       root_signature,  ID3D12RootSignature* );
DECLARE_PIPELINE_STATE_SUBOBJECT( VertexShader,        vs,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( PixelShader,         ps,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( DomainShader,        ds,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( HullShader,          hs,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( GeometryShader,      gs,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( ComputeShader,       cs,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( StreamOutput,        stream_output,   D3D12_STREAM_OUTPUT_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( BlendState,          blend_state,     D3D12_BLEND_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( SampleMask,          sample_mask,     unsigned int );
DECLARE_PIPELINE_STATE_SUBOBJECT( RasterizerState,     rasterizer,      D3D12_RASTERIZER_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( DepthStencilState,   depth_stencil,   D3D12_DEPTH_STENCIL_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( InputLayout,         input_layout,    D3D12_INPUT_LAYOUT_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( IBStripCut,          ib_strip_cut,    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE );
DECLARE_PIPELINE_STATE_SUBOBJECT( PrimitiveTopology,   topology,        D3D12_PRIMITIVE_TOPOLOGY_TYPE );
DECLARE_PIPELINE_STATE_SUBOBJECT( RTVFormats,          rtv_formats,     D3D12_RT_FORMAT_ARRAY );
DECLARE_PIPELINE_STATE_SUBOBJECT( DSVFormat,           dsv_format,      DXGI_FORMAT );
DECLARE_PIPELINE_STATE_SUBOBJECT( SampleDesc,          sample_desc,     DXGI_SAMPLE_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( NodeMask,            node_mask,       unsigned int );
DECLARE_PIPELINE_STATE_SUBOBJECT( CachedPSO,           cached_pso,      D3D12_CACHED_PIPELINE_STATE );
DECLARE_PIPELINE_STATE_SUBOBJECT( Flags,               flags,           D3D12_PIPELINE_STATE_FLAGS );
DECLARE_PIPELINE_STATE_SUBOBJECT( DepthStencilState1,  depth_stencil,   D3D12_DEPTH_STENCIL_DESC1 );
DECLARE_PIPELINE_STATE_SUBOBJECT( ViewInstancing,      view_instancing, D3D12_VIEW_INSTANCING_DESC );
DECLARE_PIPELINE_STATE_SUBOBJECT( AmplificationShader, as,              D3D12_SHADER_BYTECODE );
DECLARE_PIPELINE_STATE_SUBOBJECT( MeshShader,          ms,              D3D12_SHADER_BYTECODE );

#undef DECLARE_PIPELINE_STATE_SUBOBJECT


Core::IMPipelineState * CreatePipelineState( const D3D12_PIPELINE_STATE_STREAM_DESC *desc );

}} /* namespace D3D12::Pipelines */