#include <cstring>

#include "RenderInitializers.hpp"
#include "RenderPipelines.hpp"
#include "RenderShaders.hpp"
#include "Utilities.hpp"


namespace RenderPipelines
{


/*******************************************************************
*
*   PipelineBuilder_BuildPipeline()
*
*   DESCRIPTION:
*       Build the pipeline state object from the given builder.
*
*******************************************************************/

ID3D12PipelineState * PipelineBuilder_BuildPipeline( const PipelineBuilder *builder, ID3D12Device *device )
{
D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
desc.pRootSignature        = builder->root_signature;
desc.RasterizerState       = builder->rasterizer;
desc.BlendState            = builder->blending;
desc.DepthStencilState     = builder->depth_stencil;
desc.PrimitiveTopologyType = builder->primitive_topology;
desc.DSVFormat             = DEPTH_STENCIL_FORMAT;

desc.RTVFormats[ 0 ]       = RENDER_TARGET_FORMAT;

desc.InputLayout.NumElements        = builder->vertex_descriptor.element_count;
desc.InputLayout.pInputElementDescs = builder->vertex_descriptor.elements;

for( uint32_t i = 0; i < builder->shaders->stage_count; i++ )
    {
    switch( builder->shaders->stages[ i ].stage_name )
        {
        case RenderShaders::SHADER_STAGE_NAME_VERTEX:
            desc.VS = builder->shaders->stages[ i ].byte_code;
            break;

        case RenderShaders::SHADER_STAGE_NAME_PIXEL:
            desc.PS = builder->shaders->stages[ i ].byte_code;
            break;

        default:
            debug_assert_always();
            break;
        }
    }


ID3D12PipelineState *ret;
if( FAILED( device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( &ret ) ) ) )
    {
    debug_assert_always();
    return( NULL );
    }

return( ret );

} /* PipelineBuilder_BuildPipeline() */


} /* namespace RenderPipelines() */