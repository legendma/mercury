#pragma once

#include <d3d12.h>
#include <cstdint>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "Math.hpp"
#include "ResourceManager.hpp"

namespace RenderModels
{

#define VERTEX_DESCRIPTOR_MAX_ELEMENT_COUNT \
                                    ( 2 )

#define MAX_MODEL_COUNT             ( 1000 )
#define MAX_MODEL_MESH_COUNT        ( 5 )          
#define ASSETS_HASH_MAP_LUFT        ( 1.3f )
#define asset_hash_map_capacity( _count ) \
    (uint32_t)( ASSETS_HASH_MAP_LUFT * (float)(_count) )

typedef AssetFileModelVertex Vertex;
typedef AssetFileModelIndex Index;

typedef struct _VertexDescriptor
    {
    uint8_t             element_count;
    D3D12_INPUT_ELEMENT_DESC
                        elements[ VERTEX_DESCRIPTOR_MAX_ELEMENT_COUNT ];
    } VertexDescriptor;

typedef struct _ModelMesh
    {
    uint32_t            index_first;/* first index of this mesh     */
    uint32_t            index_count;/* number of indices            */
    uint32_t            vertex_offset;
                                    /* mesh's first vertex in vb    */
    } ModelMesh;

typedef struct _Model
    {
    AssetFileAssetId    asset_id;   /* asset ID from disk           */
    Vertex             *vertices;   /* cpu vertex buffer            */
    Index              *indices;    /* cpu index buffer             */
    ModelMesh           meshes[ MAX_MODEL_MESH_COUNT ];
                                    /* this model's meshes          */
    uint32_t            mesh_count; /* number of meshes in model    */
    } Model;

typedef struct _ModelCacheMap
    {
    HashMapKey          keys[ asset_hash_map_capacity( MAX_MODEL_COUNT ) ];
    Model               values[ asset_hash_map_capacity( MAX_MODEL_COUNT ) ];
    HashMap             map;
    } ModelCacheMap;

typedef struct _ModelCache
    {
    ResourceManager     loader;
    ModelCacheMap       cache;
    LinearAllocator     pool;
    } ModelCache;

        
Model *          ModelCache_GetModel( const char *asset_name, ModelCache *cache );
bool             ModelCache_Init( const uint32_t cache_sz, ModelCache *cache );
VertexDescriptor Vertex_GetDescriptor();

} /* namespace RenderModels */