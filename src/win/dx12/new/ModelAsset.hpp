#pragma once
#include <stdint.h>

#include "AssetFile.hpp"
#include "LinearAllocator.hpp"
#include "Math.hpp"
#include "ResourceLoader.hpp"

namespace Render { namespace Asset
{
static const int MODEL_MESH_MAX_COUNT = 5;

typedef struct _Mesh
    {
    uint8_t             material_index;
                                    /* material in model materials  */
    uint32_t            index_first;/* first index of this mesh     */
    uint32_t            index_count;/* number of indices            */
    uint32_t            vertex_offset;
                                    /* mesh's first vertex in vb    */
    uint32_t            vertex_count;
                                    /* number of vertices           */
    Float4x4            transform;  /* mesh in model's origin       */
    } Mesh;

typedef struct _Model
    {
    AssetFileAssetId    asset_id;   /* asset ID from disk           */
    AssetFileModelVertex
                       *vertices;   /* cpu vertex buffer            */
    uint32_t            vertex_count;
                                    /* total number of vertices     */
    AssetFileModelIndex
                       *indices;    /* cpu index buffer             */
    uint32_t            index_count;/* total number of indices      */
    Mesh                meshes[ MODEL_MESH_MAX_COUNT ];
                                    /* this model's meshes          */
    AssetFileModelMaterial
                       *materials;  /* material array               */
    uint32_t            mesh_count; /* number of meshes in model    */
    } Model;


bool ReadModel( const AssetFileAssetId model_id, ResourceLoader *loader, LinearAllocator *out_buffer, Model *out_definition );


}} /* namespace Render::Asset */