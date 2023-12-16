#include <cstring>
#include <cstdio>

#include "ComUtilities.hpp"
#include "RenderPipelines.hpp"
#include "RenderShaders.hpp"
#include "RenderInitializers.hpp"
#include "Utilities.hpp"


namespace RenderShaders
{

static const D3D12_STATIC_SAMPLER_DESC SAMPLERS[] =
	{
	/* PointWrap   = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 0, D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP  ),
	/* PointClamp  = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 1, D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP ),
	/* LinearWrap  = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP  ),
	/* LinearClamp = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP ),
	/* AnisoWrap   = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 4, D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,  D3D12_TEXTURE_ADDRESS_MODE_WRAP  ),
	/* PointClamp  = */ RENDER_INITIALIZER_DECLARE_STATIC_SAMPLER( 5, D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP )
	};


static ShaderModule * LoadShaderFromFile( const char *asset_name, const uint32_t asset_id, ShaderCache *cache );


/*******************************************************************
*
*   ShaderEffect_GetRootSignature()
*
*   DESCRIPTION:
*       Create the root signature and set it on the pipeline
*       builder.
*
*******************************************************************/

bool ShaderEffect_GetRootSignature( const ShaderEffect *shaders, ID3D12Device *device, RenderPipelines::_PipelineBuilder *builder )
{
ID3DBlob *serialized = NULL;
ID3DBlob *errors = NULL;
D3D12_ROOT_SIGNATURE_DESC desc = RenderInitializers::GetRootSignatureDescriptor( shaders->signature_flags, shaders->parameters, shaders->parameter_count, SAMPLERS, cnt_of_array( SAMPLERS ) );
if( FAILED( D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &serialized, &errors ) ) )
    {
    if( errors )
        {
        ::OutputDebugStringA( (char *)errors->GetBufferPointer() );
        }
    ComSafeRelease( &serialized );
    ComSafeRelease( &errors );

    return( false );
    }

ComSafeRelease( &errors );

if( FAILED( device->CreateRootSignature( NODE_MASK_SINGLE_GPU, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS( &builder->root_signature ) ) ) )
    {
    ComSafeRelease( &serialized );
    return( false );
    }

ComSafeRelease( &serialized );

return( true );

} /* ShaderEffect_GetRootSignature() */


/*******************************************************************
*
*   ShaderCache_Destroy()
*
*   DESCRIPTION:
*       Release the shader cache's resources.
*
*******************************************************************/

void RenderShaders::ShaderCache_Destroy( ShaderCache *cache )
{
ResourceLoader_Destroy( &cache->loader );

} /* ShaderCache_Destroy() */


/*******************************************************************
*
*   ShaderCache_GetShader()
*
*   DESCRIPTION:
*       Get the cached shader, and if it doesn't exist, load it
*       from file.
*
*******************************************************************/

ShaderModule * RenderShaders::ShaderCache_GetShader( const char *asset_name, ShaderCache *cache )
{
uint32_t key = AssetFile_MakeAssetIdFromName( asset_name, (uint32_t)strlen( asset_name ) );
ShaderModule *ret = (ShaderModule*)HashMap_At( key, &cache->cache.map );
if( ret )
    {
    return( ret );
    }

ret = LoadShaderFromFile( asset_name, key, cache );
debug_assert( ret );

return( ret );

} /* ShaderCache_GetShader() */


/*******************************************************************
*
*   ShaderCache_Init()
*
*   DESCRIPTION:
*       Initialize the shader cache by allocating its memory pool
*       and opening the file reader.
*
*******************************************************************/

bool RenderShaders::ShaderCache_Init( const uint32_t cache_sz, ShaderCache *cache )
{
*cache = {};
if( !ResourceLoader_Init( &cache->loader ) )
    {
    debug_assert_always();
    return( false );
    }

if( !LinearAllocator_Init( cache_sz, &cache->pool ) )
    {
    return( false );
    }

compiler_assert( cnt_of_array( cache->cache.keys ) == cnt_of_array( cache->cache.values ), RenderShaders_cpp );
HashMap_InitImplementation( &cache->cache );

return( true );

} /* ShaderCache_Init() */


/*******************************************************************
*
*   ShaderEffect_PushStage()
*
*   DESCRIPTION:
*       Add a pipeline stage to the shader effect.
*
*******************************************************************/

bool ShaderEffect_PushStage( const ShaderStage new_stage, ShaderEffect *shaders )
{
if( shaders->stage_count >= cnt_of_array( shaders->stages ) )
    {
    return( false );
    }

shaders->stages[ shaders->stage_count++ ] = new_stage;

return( true );

} /* ShaderEffect_PushStage() */


/*******************************************************************
*
*   LoadShaderFromFile()
*
*   DESCRIPTION:
*       Load the shader from file, cache it, then return the cache.
*
*******************************************************************/

static ShaderModule * LoadShaderFromFile( const char *asset_name, const uint32_t asset_id, ShaderCache *cache )
{
uint32_t byte_code_sz = 0;
if( !ResourceLoader_GetShader( asset_id, &byte_code_sz, NULL, &cache->loader ) )
    {
    debug_assert_always();
    return( NULL );
    }

ShaderModule *ret = (ShaderModule*)HashMap_Insert( asset_id, NULL, &cache->cache.map );
if( !ret )
    {
    debug_assert_always();
    return( NULL );
    }

uint32_t name_sz = (uint32_t)strlen( asset_name ) + 1/* for null */;
uint8_t *allocation = (uint8_t*)LinearAllocator_Allocate( name_sz + byte_code_sz, &cache->pool );
if( !allocation )
    {
    HashMap_Delete( asset_id, &cache->cache.map );
    debug_assert_always();
    return( NULL );
    }

ret->name         = (char*)allocation;
ret->byte_code    = &allocation[ name_sz ];
ret->byte_code_sz = byte_code_sz;

strncpy_s( (char*)allocation, name_sz, asset_name, name_sz );

hard_assert( ResourceLoader_GetShader( asset_id, &byte_code_sz, ret->byte_code, &cache->loader ) );

return( ret );

} /* LoadShaderFromFile() */


} /* namespace RenderShaders */