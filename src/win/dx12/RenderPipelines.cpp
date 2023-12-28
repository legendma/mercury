#include <cstring>

#include "ComUtilities.hpp"
#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderPipelines.hpp"
#include "RenderShaders.hpp"
#include "Utilities.hpp"

#define SHADER_CACHE_SZ             ( 250 * 1024 * 1024 )
#define CONSTANT_BUFFER_VIEW_COUNT  ( 0 )
#define SHADER_RESOURCE_VIEW_COUNT  ( RenderPass::DEFAULT_PASS_SRV_COUNT )


namespace Render { namespace Pipelines
{

/*******************************************************************
*
*   AddEffectStage()
*
*   DESCRIPTION:
*       Add the given shader to the effect's stages.
*
*******************************************************************/

static inline void AddEffectStage( const char *shader_asset_name, const Shaders::ShaderStageName stage_name, Shaders::ShaderCache *shader_cache, Shaders::ShaderEffect *effect )
{
Shaders::ShaderStage   stage;
Shaders::ShaderModule *shader = NULL;

stage = {};
shader = Shaders::ShaderCache_GetShader( shader_asset_name, shader_cache );
hard_assert( shader );

stage.stage_name                = stage_name;
stage.byte_code.pShaderBytecode = shader->byte_code;
stage.byte_code.BytecodeLength  = shader->byte_code_sz;

hard_assert( Shaders::ShaderEffect_PushStage( stage, effect ) );

} /* AddEffectStage() */


/*******************************************************************
*
*   GetBuilder()
*
*   DESCRIPTION:
*       Get a builder from the builders map.
*
*******************************************************************/

#define GetBuilder( _name, _map ) \
    (PipelineBuilder*)HashMap_At( Utilities_HashString2( _name ), &(_map)->map )


/*******************************************************************
*
*   GetEffect()
*
*   DESCRIPTION:
*       Get an effect from the effects map.
*
*******************************************************************/

#define GetEffect( _name, _map ) \
    (Shaders::ShaderEffect*)HashMap_At( Utilities_HashString2( _name ), &(_map)->map )


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
    (Shaders::ShaderEffect*)HashMap_Insert( Utilities_HashString2( _name ), NULL, &(_map)->map )


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

for( uint32_t i = 0; i < builder->effect->stage_count; i++ )
    {
    switch( builder->effect->stages[ i ].stage_name )
        {
        case Shaders::SHADER_STAGE_NAME_VERTEX:
            desc.VS = builder->effect->stages[ i ].byte_code;
            break;

        case Shaders::SHADER_STAGE_NAME_PIXEL:
            desc.PS = builder->effect->stages[ i ].byte_code;
            break;

        default:
            debug_assert_always();
            break;
        }
    }

desc.StreamOutput                   = {};
desc.BlendState                     = builder->blending;
desc.SampleMask                     = 0xffffffff;
desc.RasterizerState                = builder->rasterizer;
desc.DepthStencilState              = builder->depth_stencil;
desc.InputLayout.NumElements        = builder->vertex_descriptor.element_count;
desc.InputLayout.pInputElementDescs = builder->vertex_descriptor.elements;
desc.IBStripCutValue                = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
desc.PrimitiveTopologyType          = builder->primitive_topology;

desc.NumRenderTargets = builder->num_render_targets;
for( uint32_t i = 0; i < cnt_of_array( desc.RTVFormats ); i++ )
    {
    desc.RTVFormats[ i ] = builder->rt_formats[ i ];
    }

desc.DSVFormat                        = Initializers::DEPTH_STENCIL_FORMAT;
desc.SampleDesc                       = Initializers::GetDepthStencilResourceDescriptor( 0, 0 ).SampleDesc;
desc.NodeMask                         = Initializers::NODE_MASK_SINGLE_GPU;
desc.CachedPSO.CachedBlobSizeInBytes  = 0;
desc.CachedPSO.pCachedBlob            = NULL;
desc.Flags                            = D3D12_PIPELINE_STATE_FLAG_NONE;


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
Shaders::ShaderCache_Destroy( &pipelines->shader_cache );
for( uint32_t i = 0; i < cnt_of_array( ALL_BUILDERS ); i++ )
    {
    PipelineBuilder *builder = GetBuilder( ALL_BUILDERS[ i ], &pipelines->builders );
    if( builder )
        {
        ComSafeRelease( &builder->root_signature );
        }
    }

*pipelines = {};

} /* Pipelines_Destroy() */


/*******************************************************************
*
*   Pipelines_GetBuilder()
*
*   DESCRIPTION:
*       Get a pipeline builder by its name.
*
*******************************************************************/

PipelineBuilder * Pipelines_GetBuilder( const char *name, Pipelines *pipelines )
{
return( GetBuilder( name, &pipelines->builders ) );

} /* Pipelines_GetBuilder() */


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

Shaders::ShaderCache_Init( SHADER_CACHE_SZ, &pipelines->shader_cache );

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
/* Default */
    {
    PipelineBuilder *builder = InsertBuilder( BUILDER_NAME_DEFAULT, &pipelines->builders );
    hard_assert( builder );
    builder->effect = GetEffect( EFFECT_NAME_DEFAULT, &pipelines->effects );
    hard_assert( builder->effect );

    Shaders::ShaderEffect_GetRootSignature( builder->effect, device, builder );

    NameD3D( builder->root_signature, L"Pipelines::DefaultPipeline::RootSignature" );

    for( uint32_t i = 0; i < cnt_of_array( builder->rt_formats ); i++ )
        {
        builder->rt_formats[ i ] = DXGI_FORMAT_UNKNOWN;
        }

    builder->rt_formats[ 0 ]    = Initializers::RENDER_TARGET_FORMAT;
    builder->num_render_targets = 1;
    
    builder->blending.AlphaToCoverageEnable = FALSE;
    builder->blending.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC *blend = builder->blending.RenderTarget;
    blend->BlendEnable           = FALSE;
    blend->LogicOpEnable         = FALSE;
    blend->SrcBlend              = D3D12_BLEND_SRC_COLOR;
    blend->DestBlend             = D3D12_BLEND_DEST_COLOR;
    blend->BlendOp               = D3D12_BLEND_OP_ADD;
    blend->SrcBlendAlpha         = D3D12_BLEND_ONE;
    blend->DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
    blend->BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blend->LogicOp               = D3D12_LOGIC_OP_NOOP;
    blend->RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    builder->rasterizer         = Initializers::GetRasterizerDescriptorDefault( D3D12_FILL_MODE_SOLID );
    builder->depth_stencil      = Initializers::GetDepthStencilDescriptor( Initializers::ENABLE_DEPTH_TEST, Initializers::ENABLE_DEPTH_WRITE );
    builder->primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    builder->vertex_descriptor  = RenderModels::Vertex_GetDescriptor();
    }

return( true );

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
D3D12_DESCRIPTOR_RANGE textures_template = Initializers::GetDescriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, RenderModels::TextureNames::ASSET_FILE_MODEL_TEXTURES_COUNT, 0, 0 );

/* default */
    {
    Shaders::ShaderEffect *effect = InsertEffect( EFFECT_NAME_DEFAULT, &pipelines->effects );
    hard_assert( effect );
    effect->signature_flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    AddEffectStage( Shaders::DEFAULT_VERTEX_SHADER, Shaders::SHADER_STAGE_NAME_VERTEX, &pipelines->shader_cache, effect );
    AddEffectStage( Shaders::DEFAULT_PIXEL_SHADER, Shaders::SHADER_STAGE_NAME_PIXEL, &pipelines->shader_cache, effect );
    D3D12_DESCRIPTOR_RANGE *textures = Shaders::ShaderEffect_PushDescriptorRange( effect );
    *textures = textures_template;
    
    D3D12_ROOT_PARAMETER params[ SLOT_NAME_COUNT ];
    params[ SLOT_NAME_PER_OBJECT ] = Initializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 0, 0 );
    params[ SLOT_NAME_MATERIALS  ] = Initializers::GetRootParameterDescriptorTable( D3D12_SHADER_VISIBILITY_PIXEL, textures, 1 );
    params[ SLOT_NAME_PER_PASS   ] = Initializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 1, 0 );
    params[ SLOT_NAME_PER_FRAME  ] = Initializers::GetRootParameterDescriptorConstantBufferView( D3D12_SHADER_VISIBILITY_VERTEX, 2, 0 );
    Shaders::ShaderEffect_SetRootParameters( params, cnt_of_array( params ), effect );    
    }

} /* CreateShaders() */


} }/* namespace RenderPipelines() */