#include <cstring>
#include <cstdio>

#include "ComUtilities.hpp"
#include "RenderShaders.hpp"
#include "RenderInitializers.hpp"
#include "Utilities.hpp"


namespace RenderShaders
{


static ShaderModule * LoadShaderFromFile( const char *asset_name, const uint32_t asset_id, ShaderCache *cache );


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
if( !ResourceManager_Init( &cache->loader ) )
    {
    debug_assert_always();
    return( false );
    }

if( !LinearAllocator_Init( cache_sz, &cache->pool ) )
    {
    return( false );
    }

compiler_assert( cnt_of_array( cache->cache.keys ) == cnt_of_array( cache->cache.values ), RenderShaders_cpp );
HashMap_Init( cnt_of_array( cache->cache.values ), sizeof( *cache->cache.values ), &cache->cache.map, cache->cache.keys, cache->cache.values );

return( true );

} /* ShaderCache_Init() */


/*******************************************************************
*
*   ShaderEffect_SetRootSignature()
*
*   DESCRIPTION:
*       Create the root signature and set it on the pipeline
*       builder.
*
*******************************************************************/

bool ShaderEffect_SetRootSignature( const ShaderEffect *shaders, ID3D12Device *device, RenderPipelines::PipelineBuilder *builder )
{
ID3DBlob *serialized;
ID3DBlob *errors;
D3D12_ROOT_SIGNATURE_DESC desc = RenderInitializers::GetRootSignatureDescriptor( shaders->signature_flags, shaders->parameters, shaders->parameter_count, NULL, 0 ); // TODO <MPA> - Pass samplers

if( FAILED( D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors ) ) )
    {
    printf_s( "%s", (const char*)errors->GetBufferPointer() );
    ComSafeRelease( &serialized );
    ComSafeRelease( &errors );

    return( false );
    }

if( FAILED( device->CreateRootSignature( NODE_MASK_SINGLE_GPU, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS( &builder->root_signature ) ) ) )
    {
    ComSafeRelease( &serialized );
    ComSafeRelease( &errors );

    return( false );
    }

return( true );

} /* ShaderEffect_SetRootSignature() */


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
if( !ResourceManager_GetShader( asset_id, &byte_code_sz, NULL, &cache->loader ) )
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

hard_assert( ResourceManager_GetShader( asset_id, &byte_code_sz, ret->byte_code, &cache->loader ) );

return( ret );

} /* LoadShaderFromFile() */


} /* namespace RenderShaders */