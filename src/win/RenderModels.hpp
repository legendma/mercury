#pragma once

#include <d3d12.h>
#include <cstdint>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "Math.hpp"
#include "ResourceLoader.hpp"

namespace RenderModels
{

#define VERTEX_DESCRIPTOR_MAX_ELEMENT_COUNT \
                                    ( 2 )

#define MODEL_MAX_COUNT             ( 1000 )
#define MODEL_MESH_MAX_COUNT        ( 5 )
#define MODEL_NODES_MAX_COUNT       ( 20 )

typedef AssetFileModelVertex Vertex;
typedef AssetFileModelIndex Index;

static const uint8_t VERTEX_UV_COUNT = ASSET_FILE_MODEL_VERTEX_UV_COUNT;

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
    Float4x4            transform;  /* mesh in model's origin       */
    } ModelMesh;

typedef struct _Model
    {
    AssetFileAssetId    asset_id;   /* asset ID from disk           */
    Vertex             *vertices;   /* cpu vertex buffer            */
    Index              *indices;    /* cpu index buffer             */
    ModelMesh           meshes[ MODEL_MESH_MAX_COUNT ];
                                    /* this model's meshes          */
    uint32_t            mesh_count; /* number of meshes in model    */
    } Model;

HASH_MAP_IMPLEMENT( ModelCacheMap, MODEL_MAX_COUNT, Model );
HASH_MAP_IMPLEMENT( NodeSeenMap, MODEL_NODES_MAX_COUNT, uint8_t );

typedef struct _NodeScratch
    {
    AssetFileModelNode  nodes[ MODEL_NODES_MAX_COUNT ];
    NodeSeenMap         seen;
    AssetFileModelIndex remaining[ MODEL_NODES_MAX_COUNT ];
    } NodeScratch;

typedef struct _ModelCache
    {
    ResourceLoader      loader;
    ModelCacheMap       cache;
    LinearAllocator     pool;
    NodeScratch         node_scratch;
    } ModelCache;

        
Model *          ModelCache_GetModel( const char *asset_name, ModelCache *cache );
bool             ModelCache_Init( const uint32_t cache_sz, ModelCache *cache );
VertexDescriptor Vertex_GetDescriptor();

} /* namespace RenderModels */