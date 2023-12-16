#include <cstring>
#include <d3d12.h>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "RenderModels.hpp"
#include "ResourceLoader.hpp"
#include "Utilities.hpp"

namespace RenderModels
{
static ModelMesh * AllocateModelMesh( Model *model );
static Model *     LoadModelFromFile( const char *asset_name, const uint32_t asset_id, ModelCache *cache );
static void        SolveMeshTransforms( const uint32_t node_count, NodeScratch *nodes, Model *model );


/*******************************************************************
*
*   ModelCache_GetModel()
*
*   DESCRIPTION:
*       Initialize the model cache.
*
*******************************************************************/

Model * ModelCache_GetModel( const char *asset_name, ModelCache *cache )
{
uint32_t key = AssetFile_MakeAssetIdFromName( asset_name, (uint32_t)strlen( asset_name ) );
Model *ret = (Model*)HashMap_At( key, &cache->cache.map );
if( ret )
    {
    return( ret );
    }

ret = LoadModelFromFile( asset_name, key, cache );
debug_assert( ret );

return( ret );

} /* ModelCache_GetModel() */


/*******************************************************************
*
*   ModelCache_Init()
*
*   DESCRIPTION:
*       Initialize the model cache.
*
*******************************************************************/

bool ModelCache_Init( const uint32_t cache_sz, ModelCache *cache )
{
clr_struct( cache );
if( !ResourceLoader_Init( &cache->loader ) )
    {
    debug_assert_always();
    return( false );
    }

if( !LinearAllocator_Init( cache_sz, &cache->pool ) )
    {
    return( false );
    }

compiler_assert( cnt_of_array( cache->cache.keys ) == cnt_of_array( cache->cache.values ), RenderModels_cpp );
HashMap_InitImplementation( &cache->cache );

return( true );

} /* ModelCache_Init() */


/*******************************************************************
*
*   Vertex_GetDescriptor()
*
*   DESCRIPTION:
*       Get the vertex descriptor for our vertex.
*
*******************************************************************/

VertexDescriptor Vertex_GetDescriptor()
{
unsigned int offset = 0;
VertexDescriptor ret = {};

/* position */
D3D12_INPUT_ELEMENT_DESC *position = &ret.elements[ ret.element_count++ ];
*position = {};

position->SemanticName         = "POSITION";
position->SemanticIndex        = 0;
position->Format               = DXGI_FORMAT_R32G32B32_FLOAT;
position->InputSlot            = 0;
position->AlignedByteOffset    = offset;
position->InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
position->InstanceDataStepRate = 0;

offset += 3 * sizeof(float);

/* uv0 */
D3D12_INPUT_ELEMENT_DESC *uv0 = &ret.elements[ ret.element_count++ ];
*uv0 = {};

uv0->SemanticName         = "TEXCOORD";
uv0->SemanticIndex        = 0;
uv0->Format               = DXGI_FORMAT_R32G32_FLOAT;
uv0->InputSlot            = 0;
uv0->AlignedByteOffset    = offset;
uv0->InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
uv0->InstanceDataStepRate = 0;
    
offset += 2 * sizeof(float);

return( ret );

} /* Vertex_GetDescriptor() */


/*******************************************************************
*
*   AllocateModelMesh()
*
*   DESCRIPTION:
*       Requisition a new mesh from the given model.
*
*******************************************************************/

static ModelMesh * AllocateModelMesh( Model *model )
{
if( model->mesh_count >= cnt_of_array( model->meshes ) )
    {
    return( NULL );
    }

return( &model->meshes[ model->mesh_count++ ] );

} /* AllocateModelMesh() */


/*******************************************************************
*
*   LoadModelFromFile()
*
*   DESCRIPTION:
*       Load the model from file, cache it, then return the cache.
*
*******************************************************************/
 
static Model * LoadModelFromFile( const char *asset_name, const uint32_t asset_id, ModelCache *cache )
{
Model *model = (Model*)HashMap_Insert( asset_id, NULL, &cache->cache.map );
if( !model )
    {
    return( NULL );
    }

model->asset_id = asset_id;

ResourceLoaderModelStats stats = {};
if( !ResourceLoader_GetModelStats( asset_id, &stats, &cache->loader ) )
    {
    hard_assert( HashMap_Delete( asset_id, &cache->cache.map ) );
    return( NULL );
    }

uint32_t total_index_sz  = stats.index_count  * stats.index_stride;
uint32_t total_vertex_sz = stats.vertex_count * stats.vertex_stride;

AssetFileModelVertex *vertices = (AssetFileModelVertex*)LinearAllocator_AllocateAligned( total_index_sz + total_vertex_sz, align_of_t( AssetFileModelVertex ), &cache->pool );
if( vertices == NULL )
    {
    hard_assert( HashMap_Delete( asset_id, &cache->cache.map ) );
    return( NULL );
    }

AssetFileModelIndex *indices = (AssetFileModelIndex*)&vertices[ stats.vertex_count ];

uint32_t vertex_start = 0;
uint32_t index_start = 0;
for( uint32_t i = 0; i < stats.mesh_count; i++ )
    {
    ModelMesh *mesh = AllocateModelMesh( model );
    if( !mesh )
        {
        debug_assert_always();
        hard_assert( HashMap_Delete( asset_id, &cache->cache.map ) );
        break;
        }
    
    uint32_t vertex_count = ResourceLoader_GetModelMeshVertices( asset_id, i, stats.vertex_count - vertex_start, &vertices[ vertex_start ], &cache->loader );
    uint32_t index_count  = ResourceLoader_GetModelMeshIndices(  asset_id, i, stats.index_count  - index_start,  &indices[ index_start ],   &cache->loader );
    
    mesh->vertex_offset = vertex_start;
    mesh->index_first   = index_start;
    mesh->index_count   = index_count;
    memcpy( &mesh->transform, &FLOAT4x4_IDENTITY, sizeof( mesh->transform ) );

    vertex_start += vertex_count;
    index_start  += index_count;
    }

uint32_t node_count = ResourceLoader_GetModelMeshNodes( asset_id, cnt_of_array( cache->node_scratch.nodes ), cache->node_scratch.nodes, &cache->loader );
SolveMeshTransforms( node_count, &cache->node_scratch, model );

return( model );

} /* LoadModelFromFile() */


/*******************************************************************
*
*   SolveMeshTransforms()
*
*   DESCRIPTION:
*       Fill out each mesh's transforms by solving the node tree.
*
*******************************************************************/

static void SolveMeshTransforms( const uint32_t node_count, NodeScratch *nodes, Model *model )
{
uint32_t remain_count = node_count;
for( uint32_t i = 0; i < node_count; i++ )
    {
    nodes->remaining[ i ] = i;
    }

HashMap_InitImplementation( &nodes->seen );
while( remain_count > 0 )
    {
    HashMap_Clear( &nodes->seen.map );
    for( uint32_t i = 0; i < remain_count; i++ )
        {
        AssetFileModelIndex this_node_index = nodes->remaining[ i ];
        AssetFileModelNode *this_node = &nodes->nodes[ this_node_index ];
        for( uint32_t j = 0; j < this_node->child_node_count; j++ )
            {
            AssetFileModelIndex child_node_index = this_node->child_nodes[ j ];
            HashMap_Insert( Utilities_HashU32( child_node_index ), NULL, &nodes->seen.map );
            }
        }

    for( int32_t i = remain_count - 1; i >= 0; i-- )
        {
        AssetFileModelIndex this_node_index = nodes->remaining[ i ];
        AssetFileModelNode *this_node = &nodes->nodes[ this_node_index ];
        if( HashMap_At( this_node_index, &nodes->seen.map ) )
            {
            continue;
            }

        Float4x4 *this_matrix = (Float4x4*)&this_node->transform;
        for( uint32_t j = 0; j < this_node->child_mesh_count; j++ )
            {
            ModelMesh *child_mesh = &model->meshes[ this_node->child_meshes[ j ] ];
            memcpy( &child_mesh->transform, this_matrix, sizeof(child_mesh->transform) );
            }

        for( uint32_t j = 0; j < this_node->child_node_count; j++ )
            {
            Float4x4 *child_matrix = (Float4x4*)&nodes->nodes[ this_node->child_nodes[ j ] ].transform;
            Math_Float4x4MultiplyByFloat4x4( this_matrix, child_matrix, child_matrix );
            }

        memcpy( &nodes->remaining[ i ], &nodes->remaining[ remain_count-- ], sizeof(*nodes->remaining) );
        }
    }

} /* SolveMeshTransforms() */


} /* namespace RenderModels */