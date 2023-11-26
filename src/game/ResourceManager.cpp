#include <cstring>
#include <cstdint>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "ResourceManager.hpp"

#define ASSETS_FILE_NAME            "AllAssets.bin"


static bool OpenAssetFile( const char *filename, AssetFileReader *input );


/*******************************************************************
*
*   ResourceManager_Destroy()
*
*   DESCRIPTION:
*       Destroy the resource manager.
*
*******************************************************************/

bool ResourceManager_Destroy( ResourceManager *manager )
{
bool ret = AssetFile_CloseForRead( &manager->reader );
*manager = {};

return( ret );

} /* ResourceManager_Destroy() */


/*******************************************************************
*
*   ResourceManager_GetModelMeshIndices()
*
*   DESCRIPTION:
*       Get the requested model's mesh indices by mesh index.
*
*******************************************************************/

uint32_t ResourceManager_GetModelMeshIndices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t index_capacity, AssetFileModelIndex *indices, ResourceManager *manager )
{
if( !manager->reader.fhnd
 || indices == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &manager->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t index_count;
if( !AssetFile_ReadModelMeshIndices( mesh_index, index_capacity, &index_count, indices, &manager->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );

return( index_count );

} /* ResourceManager_GetModelMeshIndices() */


/*******************************************************************
*
*   ResourceManager_GetModelMeshVertices()
*
*   DESCRIPTION:
*       Get the requested model's mesh vertices by mesh index.
*
*******************************************************************/

uint32_t ResourceManager_GetModelMeshVertices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t vertex_capacity, AssetFileModelVertex *vertices, ResourceManager *manager )
{
if( !manager->reader.fhnd
 || vertices == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &manager->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t vertex_count;
if( !AssetFile_ReadModelMeshVertices( mesh_index, vertex_capacity, &vertex_count, vertices, &manager->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );

return( vertex_count );

} /* ResourceManager_GetModelMeshVertices() */


/*******************************************************************
*
*   ResourceManager_GetModelStats()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

bool ResourceManager_GetModelStats( const AssetFileAssetId asset_id, ResourceManagerModelStats *out_stats, ResourceManager *manager )
{
if( !manager->reader.fhnd
 || out_stats == NULL )
    {
    return( false );
    }

*out_stats = {};
if( !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &manager->reader ) )
    {
    return( false );
    }

if( !AssetFile_ReadModelStorageRequirements( &out_stats->vertex_count, &out_stats->index_count, &out_stats->mesh_count, &out_stats->node_count, &manager->reader ) )
    {
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( false );
    }

out_stats->index_stride  = sizeof(AssetFileModelIndex);
out_stats->vertex_stride = sizeof(AssetFileModelVertex);

do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );

return( true );

} /* ResourceManager_GetModelStats() */


/*******************************************************************
*
*   ResourceManager_GetShader()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

bool ResourceManager_GetShader( const AssetFileAssetId asset_id, uint32_t *sz, uint8_t *bytes, ResourceManager *manager )
{
if( !manager->reader.fhnd
 || ( sz == NULL
   && bytes == NULL ) )
    {
    return( false );
    }

uint32_t storage_sz = 0;
if( sz != NULL )
    {
    storage_sz = *sz;
    *sz = 0;
    }

uint32_t shader_sz = 0;
if( !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_SHADER, &manager->reader ) )
    {
    return( false );
    }

if( !AssetFile_ReadShaderStorageRequirements( &shader_sz, &manager->reader ) )
    {
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( false );
    }
    
if( sz != NULL )
    {
    *sz = shader_sz;
    }

if( bytes == NULL )
    {
    /* just querying the shader size.  not an error */
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( true );
    }

if( storage_sz < shader_sz )
    {
    /* not enough storage to load from file */
    hard_assert_always();
    AssetFile_EndReadingAsset( &manager->reader );
    return( false );
    }
    
uint32_t read_sz = 0;
if( !AssetFile_ReadShaderBinary( storage_sz, &read_sz, bytes, &manager->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );
    return( 0 );
    }

debug_assert( read_sz == shader_sz );
do_debug_assert( AssetFile_EndReadingAsset( &manager->reader ) );

return( true );

} /* ResourceManager_GetShader() */


/*******************************************************************
*
*   ResourceManager_Init()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

bool ResourceManager_Init( ResourceManager *manager )
{
*manager = {};

if( !OpenAssetFile( ASSETS_FILE_NAME, &manager->reader ) )
    {
    printf( "ResourceManager - Failed to open assets package." );
    return( false );
    }

return( true );

} /* ResourceManager_Init() */


/*******************************************************************
*
*   OpenAssetFile()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

static bool OpenAssetFile( const char *filename, AssetFileReader *input )
{
#define MAX_FILEPATH_LEN            ( 200 )
char                    filepath[ MAX_FILEPATH_LENGTH ];

for( uint32_t i = 0; i < cnt_of_array( RELATIVE_ROOT_DIRECTORY ); i++ )
    {
    sprintf_s( filepath, sizeof(filepath), "%s%s", RELATIVE_ROOT_DIRECTORY[ i ], filename );
    if( AssetFile_OpenForRead( filepath, input ) )
        {
        return( true );
        }    
    }

return( false );

#undef MAX_FILEPATH_LEN
} /* OpenAssetFile() */