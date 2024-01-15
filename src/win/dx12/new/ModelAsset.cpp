#include <stdint.h>

#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "ModelAsset.hpp"
#include "Utilities.hpp"


#define MODEL_NODES_MAX_COUNT       ( 20 )


namespace Render { namespace Asset
{
HASH_MAP_IMPLEMENT( NodeSeenMap, MODEL_NODES_MAX_COUNT, uint8_t );


typedef struct _NodeScratch
    {
    AssetFileModelNode  nodes[ MODEL_NODES_MAX_COUNT ];
    NodeSeenMap         seen;
    AssetFileModelIndex remaining[ MODEL_NODES_MAX_COUNT ];
    } NodeScratch;


static Mesh * AllocateModelMesh( Model *model );
static bool   LoadModel( const AssetFileAssetId id, ResourceLoader *loader, LinearAllocator *allocator, Model *model );
static void   SolveMeshTransforms( const uint32_t node_count, NodeScratch *nodes, Model *model );


/*******************************************************************
*
*   ReadModel()
*
*   DESCRIPTION:
*       Read a model from disk using the given allocator.  Fill out
*       the model definition describing the model contained within
*       the allocator.
*
*       The given loader and allocator are expected to be valid and
*       initialized.
*
*******************************************************************/

bool ReadModel( const AssetFileAssetId model_id, ResourceLoader *loader, LinearAllocator *out_buffer, Model *out_definition )
{
if( !loader
 || !out_buffer
 || !out_definition )
    {
    return( false );
    }

clr_struct( out_definition );

return( LoadModel( model_id, loader, out_buffer, out_definition ) );

} /* ReadModel() */


/*******************************************************************
*
*   AllocateModelMesh()
*
*   DESCRIPTION:
*       Requisition a new mesh from the given model.
*
*******************************************************************/

static Mesh * AllocateModelMesh( Model *model )
{
if( model->mesh_count >= cnt_of_array( model->meshes ) )
    {
    return( NULL );
    }

return( &model->meshes[ model->mesh_count++ ] );

} /* AllocateModelMesh() */


/*******************************************************************
*
*   LoadModel()
*
*   DESCRIPTION:
*       Perform the load.
*
*******************************************************************/

static bool LoadModel( const AssetFileAssetId id, ResourceLoader *loader, LinearAllocator *allocator, Model *model )
{
ResourceLoaderModelStats stats = {};
if( !ResourceLoader_GetModelStats( id, &stats, loader ) )
    {
    return( false );
    }

uint32_t total_index_sz    = stats.index_count    * stats.index_stride;
uint32_t total_vertex_sz   = stats.vertex_count   * stats.vertex_stride;
uint32_t total_material_sz = stats.material_count * stats.material_stride;

LinearAllocatorResetToken full_reset = LinearAllocator_GetResetToken( allocator );

model->vertices   = (AssetFileModelVertex*)LinearAllocator_AllocateAligned(   total_vertex_sz,   align_of_t( AssetFileModelVertex   ), allocator );
model->indices    = (AssetFileModelIndex*)LinearAllocator_AllocateAligned(    total_index_sz,    align_of_t( AssetFileModelIndex    ), allocator );
model->materials  = (AssetFileModelMaterial*)LinearAllocator_AllocateAligned( total_material_sz, align_of_t( AssetFileModelMaterial ), allocator );
if( model->vertices  == NULL
 || model->indices   == NULL
 || model->materials == NULL )
    {
    LinearAllocator_ResetByToken( full_reset, allocator );
    return( false );
    }

uint32_t material_count = ResourceLoader_GetModelMaterials( id, stats.material_count, model->materials, loader );

uint32_t vertex_start = 0;
uint32_t index_start = 0;
for( uint32_t i = 0; i < stats.mesh_count; i++ )
    {
    Mesh *mesh = AllocateModelMesh( model );
    if( !mesh )
        {
        LinearAllocator_ResetByToken( full_reset, allocator );
        return( false );
        }
    
    AssetFileModelIndex material_index;
    uint32_t vertex_count = ResourceLoader_GetModelMeshVertices( id, i, stats.vertex_count - vertex_start, &material_index, &model->vertices[ vertex_start ], loader );
    uint32_t index_count  = ResourceLoader_GetModelMeshIndices(  id, i, stats.index_count  - index_start,                   &model->indices[ index_start ],   loader );
    
    mesh->vertex_offset  = vertex_start;
    mesh->vertex_count   = vertex_count;
    mesh->index_first    = index_start;
    mesh->index_count    = index_count;
    mesh->material_index = (uint8_t)material_index;
    memcpy( &mesh->transform, &FLOAT4x4_IDENTITY, sizeof( mesh->transform ) );

    vertex_start += vertex_count;
    index_start  += index_count;
    }

debug_assert( vertex_start == stats.vertex_count );
debug_assert( index_start == stats.index_count );
model->vertex_count = vertex_start;
model->index_count = index_start;

LinearAllocatorResetToken pop_node_scratch = LinearAllocator_GetResetToken( allocator );
NodeScratch *node_scratch = (NodeScratch*)LinearAllocator_AllocateAligned( sizeof(*node_scratch), align_of_t(NodeScratch), allocator);
if( node_scratch == NULL )
    {
    LinearAllocator_ResetByToken( full_reset, allocator );
    return( false );
    }

uint32_t node_count = ResourceLoader_GetModelNodes( id, cnt_of_array( node_scratch->nodes ), node_scratch->nodes, loader );
SolveMeshTransforms( node_count, node_scratch, model );
LinearAllocator_ResetByToken( pop_node_scratch, allocator );

return( true );

} /* ReadModel() */


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
            Mesh *child_mesh = &model->meshes[ this_node->child_meshes[ j ] ];
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


} }/* namespace Render::Asset */