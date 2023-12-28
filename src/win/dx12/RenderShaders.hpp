#pragma once

#include <cassert>
#include <cstdint>
#include <d3d12.h>

#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "ResourceLoader.hpp"
#include "Utilities.hpp"


#define RENDER_SHADERS_SHADER_CACHE_MAX_COUNT \
                                    ( 100 )

#define RENDER_SHADERS_EFFECT_MAX_ROOT_PARAMETERS \
                                    ( 20 )
#define RENDER_SHADERS_DESCRIPTOR_RANGE_MAX_COUNT \
                                    ( RENDER_SHADERS_EFFECT_MAX_ROOT_PARAMETERS )

namespace Render { namespace Pipelines{ struct _PipelineBuilder; } }
namespace Render { namespace Shaders
{

static const char      *DEFAULT_VERTEX_SHADER = "shader_vs_mvp";
static const char      *DEFAULT_PIXEL_SHADER  = "shader_ps_flat";

typedef struct _ShaderModule
    {
    const char         *name;
    uint8_t            *byte_code;
    uint32_t            byte_code_sz;
    } ShaderModule;

typedef enum _ShaderStageName
    {
    SHADER_STAGE_NAME_VERTEX,
    SHADER_STAGE_NAME_PIXEL,
    /* count */
    SHADER_STAGE_NAME_COUNT
    } ShaderStageName;

typedef struct _ShaderStage
    {
    ShaderStageName     stage_name;
    D3D12_SHADER_BYTECODE
                        byte_code;
    } ShaderStage;

typedef struct _ShaderEffect
    {
    ShaderStage         stages[ SHADER_STAGE_NAME_COUNT ];
    uint8_t             stage_count;
    D3D12_DESCRIPTOR_RANGE
                        range[ RENDER_SHADERS_DESCRIPTOR_RANGE_MAX_COUNT ];
    uint8_t             range_count;
    uint16_t            parameter_count;
    D3D12_ROOT_PARAMETER
                        parameters[ RENDER_SHADERS_EFFECT_MAX_ROOT_PARAMETERS ];
    D3D12_ROOT_SIGNATURE_FLAGS
                        signature_flags;

    /* TODO <MPA> - Look into reflecting root signature via ID3D12ShaderReflection, see https://rtarun9.github.io/blogs/shader_reflection/ */
    } ShaderEffect;

HASH_MAP_IMPLEMENT( ShaderCacheMap, RENDER_SHADERS_SHADER_CACHE_MAX_COUNT, ShaderModule );

typedef struct _ShaderCache
    {
    ResourceLoader      loader;
    ShaderCacheMap      cache;
    LinearAllocator     pool;
    } ShaderCache;


bool           ShaderEffect_GetRootSignature( const ShaderEffect *shaders, ID3D12Device *device, Pipelines::_PipelineBuilder *builder );
void           ShaderCache_Destroy( ShaderCache *cache );
ShaderModule * ShaderCache_GetShader( const char *asset_name, ShaderCache *cache );
bool           ShaderCache_Init( const uint32_t cache_sz, ShaderCache *cache );
bool           ShaderEffect_PushStage( const ShaderStage new_stage, ShaderEffect *shaders );


/*******************************************************************
*
*   ShaderEffect_PushDescriptorRange()
*
*   DESCRIPTION:
*       Obtain a new descriptor range.
*
*******************************************************************/

static D3D12_DESCRIPTOR_RANGE * ShaderEffect_PushDescriptorRange( ShaderEffect *shaders )
{
if( shaders->range_count == cnt_of_array( shaders->range ) )
    {
    assert( false );
    return( NULL );
    }

return( &shaders->range[ shaders->range_count++ ] );

} /* ShaderEffect_PushDescriptorRange() */


/*******************************************************************
*
*   ShaderEffect_SetRootParameters()
*
*   DESCRIPTION:
*       Set the root parameters on the shader effect.
*
*******************************************************************/

static void inline ShaderEffect_SetRootParameters( const D3D12_ROOT_PARAMETER *params, const uint8_t param_count, ShaderEffect *shaders )
{
assert( param_count <= cnt_of_array( shaders->parameters ) );
shaders->parameter_count = 0;
for( uint8_t i = 0; i < Utilities_ClampToMaxU32( param_count, cnt_of_array( shaders->parameters ) ); i++ )
    {
    shaders->parameters[ shaders->parameter_count++ ] = params[ i ];
    }

} /* ShaderEffect_SetRootParameters() */


} }/* namespace RenderShaders */