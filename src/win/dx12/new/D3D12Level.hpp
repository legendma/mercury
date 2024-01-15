#pragma once
#include <pthread.h>

#include "AssetFile.hpp"
#include "D3D12Core.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Types.hpp"
#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "ResourceLoader.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Level
{
static const int MAX_MODEL_COUNT = 20;
static const int MAX_TEXTURE_COUNT = MAX_MODEL_COUNT * ASSET_FILE_MODEL_TEXTURES_COUNT;

static const int MAX_ASSETS_COUNT = MAX_MODEL_COUNT
                                  + MAX_TEXTURE_COUNT;

static const int MODEL_MESH_MAX_COUNT = 5;
static const int MODEL_MATERIAL_MAX_COUNT = 5;

static const uint32_t MAX_LEVEL_COUNT = 10;
static const uint64_t LEVEL_HEAP_SZ = align_size_round_down( ( Core::EXPECTED_VRAM_8GB - Core::NON_LEVEL_VRAM_2GB ) / MAX_LEVEL_COUNT, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT );

typedef union _MaterialOrdinal
    {
    uint32_t            value;
    struct
        {
        uint32_t        transparent  : 1;
        uint32_t        albedo       : 1;
        uint32_t        normal       : 1;
        uint32_t        emissive     : 1;
        uint32_t        metallic     : 1;
        uint32_t        roughness    : 1;
        uint32_t        displacement : 1;
        } t;
    } MaterialOrdinal;

typedef struct Material
    {
    MaterialOrdinal     ordinal;
    AssetFileAssetId    textures[ ASSET_FILE_MODEL_TEXTURES_COUNT ];
                                    /* material texture maps        */
    uint32_t            texture_count;
    } _Material;

typedef struct _Mesh
    {
    uint8_t             material_index;
                                    /* material in model materials  */
    bool                is_transparent;
                                    /* use transparent pass?        */
    uint32_t            index_first;/* first index of this mesh     */
    uint32_t            index_count;/* number of indices            */
    uint32_t            vertex_offset;
                                    /* mesh's first vertex in vb    */
    Float4x4            transform;  /* mesh in model's origin       */
    } Mesh;

typedef struct _Model
    {
    Material            materials[ MODEL_MATERIAL_MAX_COUNT ];
    uint32_t            material_count;
    Mesh                meshes[ MODEL_MESH_MAX_COUNT ];
                                    /* this model's meshes          */
    uint32_t            mesh_count; /* number of meshes in model    */
    uint32_t            vertex_count;
    uint32_t            index_count;
    } Model;

typedef struct _GPUAsset
    {
    AssetFileAssetId    id;
    AssetFileAssetKind  kind;
    union
        {
        Model           model;
        Texture::Texture
                        texture;    /* placed texture buffer        */
        } t;
    } GPUAsset;

HASH_MAP_IMPLEMENT( AssetMap, MAX_ASSETS_COUNT, GPUAsset );

typedef struct _Level
    {
    D3D12_VERTEX_BUFFER_VIEW
                        vbv;
    D3D12_INDEX_BUFFER_VIEW
                        ibv;
    Core::IMResource   *vb;
    Core::IMResource   *ib;
    AssetFileAssetId    models[ MAX_MODEL_COUNT ];
    uint32_t            model_count;
    AssetFileAssetId    textures[ MAX_TEXTURE_COUNT ];
    uint32_t            texture_count;
    Core::IMHeap       *heap;
    bool                is_loaded;
    bool                is_read_ready;
    pthread_t           load_thread;
    pthread_mutex_t     mutex;
    AssetMap            assets;
    AssetFileAssetId    id;
    AssetFileNameString name;
    } Level;


void Destroy( Level *level );
bool Init( const char *name, Level *level );
void LoadAsync( const char *thread_name, Level *level );


}} /* namespace D3D12::Level */