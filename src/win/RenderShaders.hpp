#pragma once

#include <cstdint>
#include <d3d12.h>

#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "RenderPipelines.hpp"
#include "ResourceManager.hpp"


#define RENDER_SHADERS_SHADER_CACHE_MAX_COUNT \
                                    ( 100 )     
#define RENDER_SHADERS_SHADER_CACHE_HASH_MAP_LUFT \
                                    ( 1.3f )
#define render_shaders_shader_cache_hash_map_capacity( _count ) \
    (uint32_t)( RENDER_SHADERS_SHADER_CACHE_HASH_MAP_LUFT * (float)(_count) )

#define RENDER_SHADERS_EFFECT_MAX_ROOT_PARAMETERS \
                                    ( 20 )


namespace RenderShaders
{

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
    uint16_t            parameter_count;
    D3D12_ROOT_PARAMETER
                        parameters[ RENDER_SHADERS_EFFECT_MAX_ROOT_PARAMETERS ];
    D3D12_ROOT_SIGNATURE_FLAGS
                        signature_flags;

    /* TODO <MPA> - Look into reflecting root signature via ID3D12ShaderReflection, see https://rtarun9.github.io/blogs/shader_reflection/ */
    } ShaderEffect;

typedef struct _ShaderCacheMap
    {
    HashMapKey          keys[ render_shaders_shader_cache_hash_map_capacity( RENDER_SHADERS_SHADER_CACHE_MAX_COUNT ) ];
    ShaderModule        values[ render_shaders_shader_cache_hash_map_capacity( RENDER_SHADERS_SHADER_CACHE_MAX_COUNT ) ];
    HashMap             map;
    } ShaderCacheMap;

typedef struct _ShaderCache
    {
    ResourceManager     loader;
    ShaderCacheMap      cache;
    LinearAllocator     pool;
    } ShaderCache;


ShaderModule * ShaderCache_GetShader( const char *asset_name, ShaderCache *cache );
bool           ShaderCache_Init( const uint32_t cache_sz, ShaderCache *cache );
bool           ShaderEffect_SetRootSignature( const ShaderEffect *shaders, ID3D12Device *device, RenderPipelines::PipelineBuilder *builder );

} /* namespace RenderShaders */