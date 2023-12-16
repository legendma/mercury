#include <cstring>

#include "ComUtilities.hpp"
#include "RenderInitializers.hpp"
#include "RenderPassDefault.hpp"
#include "RenderPipelines.hpp"
#include "RenderShaders.hpp"
#include "Utilities.hpp"

#define SHADER_CACHE_SZ             ( 250 * 1024 * 1024 )
#define CONSTANT_BUFFER_VIEW_COUNT  ( 0 )
#define SHADER_RESOURCE_VIEW_COUNT  ( RenderPass::DEFAULT_PASS_SRV_COUNT )


namespace RenderPipelines
{

/*******************************************************************
*
*   AddEffectStage()
*
*   DESCRIPTION:
*       Add the given shader to the effect's stages.
*
*******************************************************************/

static inline void AddEffectStage( const char *shader_asset_name, const RenderShaders::ShaderStageName stage_name, RenderShaders::ShaderCache *shader_cache, RenderShaders::ShaderEffect *effect )
{
RenderShaders::ShaderStage   stage;
RenderShaders::ShaderModule *shader = NULL;

stage = {};
shader = RenderShaders::ShaderCache_GetShader( shader_asset_name, shader_cache );
hard_assert( shader );

stage.stage_name                = stage_name;
stage.byte_code.pShaderBytecode = shader->byte_code;
stage.byte_code.BytecodeLength  = shader->byte_code_sz;

hard_assert( RenderShaders::ShaderEffect_PushStage( stage, effect ) );

} /* AddEffectStage() */


/*******************************************************************
*
*   GetEffect()
*
*   DESCRIPTION:
*       Get an effect from the effects map.
*
*******************************************************************/

#define GetEffect( _name, _map ) \
    (RenderShaders::ShaderEffect*)HashMap_At( Utilities_HashString2( _name ), &(_map)->map )


/*******************************************************************
*
*   InsertBuilder()
*
*   DESCRIPTION:
*       Insert a builder in the builders map.
*
*******************************************************************/

#define InsertBuilder( _name, _map ) \
    (PipelineBuilder*)HashMap_Insert( Utilities_HashString2( _name ), NULL, &(_map)->map )


/*******************************************************************
*
*   InsertEffect()
*
*   DESCRIPTION:
*       Insert an effect in the effects map.
*
*******************************************************************/

#define InsertEffect( _name, _map ) \
    (RenderShaders::ShaderEffect*)HashMap_Insert( Utilities_HashString2( _name ), NULL, &(_map)->map )


static bool CreateBuilders( ID3D12Device *device, Pipelines *pipelines );
static void CreateShaders( Pipelines *pipelines );


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

for( uint32_t i = 0; i < builder->effect->stage_count; i++ )
    {
    switch( builder->effect->stages[ i ].stage_name )
        {
        case RenderShaders::SHADER_STAGE_NAME_VERTEX:
            desc.VS = builder->effect->stages[ i ].byte_code;
            break;

        case RenderShaders::SHADER_STAGE_NAME_PIXEL:
            desc.PS = builder->effect->stages[ i ].byte_code;
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


/*******************************************************************
*
*   Pipelines_Destroy()
*
*   DESCRIPTION:
*       Destroy the pipelines.
*
*******************************************************************/

void Pipelines_Destroy( Pipelines *pipelines )
{
RenderShaders::ShaderCache_Destroy( &pipelines->shader_cache );
*pipelines = {};

} /* Pipelines_Destroy() */


/*******************************************************************
*
*   Pipelines_Init()
*
*   DESCRIPTION:
*       Initialize the pipelines manager.
*
*******************************************************************/

bool Pipelines_Init( ID3D12Device *device, Pipelines *pipelines )
{
*pipelines = {};

HashMap_InitImplementation( &pipelines->builders );
HashMap_InitImplementation( &pipelines->effects );

RenderShaders::ShaderCache_Init( SHADER_CACHE_SZ, &pipelines->shader_cache );

CreateShaders( pipelines );
CreateBuilders( device, pipelines );

return( true );

} /* Pipelines_Init() */


/*******************************************************************
*
*   CreateBuilders()
*
*   DESCRIPTION:
*       Create the pipeline builders.
*
*******************************************************************/

static bool CreateBuilders( ID3D12Device *device, Pipelines *pipelines )
{
/* Default opaque */
    {
    PipelineBuilder *builder = InsertBuilder( PIPELINE_NAME_DEFAULT_OPAQUE, &pipelines->builders );
    hard_assert( builder );
    RenderShaders::ShaderEffect *effect = GetEffect( PIPELINE_NAME_DEFAULT_OPAQUE, &pipelines->effects );
    hard_assert( effect );

    RenderShaders::ShaderEffect_GetRootSignature( effect, device, builder );

    }

return( true );
//default_opaque_builder->

} /* CreateBuilders() */


/*******************************************************************
*
*   CreateShaders()
*
*   DESCRIPTION:
*       Build the shader effects used by the program.
*
*******************************************************************/

static void CreateShaders( Pipelines *pipelines )
{
D3D12_DESCRIPTOR_RANGE textures_template = RenderInitializers::GetDescriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, BOUND_TEXTURES_COUNT, 0, 0 );

/* default opaque */
    {
    RenderShaders::ShaderEffect *effect = InsertEffect( PIPELINE_NAME_DEFAULT_OPAQUE, &pipelines->effects );
    hard_assert( effect );
    AddEffectStage( RenderShaders::DEFAULT_VERTEX_SHADER, RenderShaders::SHADER_STAGE_NAME_VERTEX, &pipelines->shader_cache, effect );
    AddEffectStage( RenderShaders::DEFAULT_PIXEL_SHADER, RenderShaders::SHADER_STAGE_NAME_PIXEL, &pipelines->shader_cache, effect );
    D3D12_DESCRIPTOR_RANGE *textures = RenderShaders::ShaderEffect_PushDescriptorRange( effect );
    *textures = textures_template;
    
    D3D12_ROOT_PARAMETER params[ SLOT_NAME_COUNT ];
    params[ SLOT_NAME_PER_OBJECT ] = RenderInitializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 0, 0 );
    params[ SLOT_NAME_MATERIALS  ] = RenderInitializers::GetRootParameterDescriptorTable( D3D12_SHADER_VISIBILITY_PIXEL, textures, 1 );
    params[ SLOT_NAME_PER_PASS   ] = RenderInitializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 1, 0 );
    params[ SLOT_NAME_PER_FRAME  ] = RenderInitializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 2, 0 );
    RenderShaders::ShaderEffect_SetRootParameters( params, cnt_of_array( params ), effect );    
    }


} /* CreateShaders() */


} /* namespace RenderPipelines() */