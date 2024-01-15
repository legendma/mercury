#include <cstring>
#include <cstdint>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "ResourceLoader.hpp"

#define ASSETS_FILE_NAME            "AllAssets.bin"


static bool OpenAssetFile( const char *filename, AssetFileReader *input );


/*******************************************************************
*
*   ResourceLoader_Destroy()
*
*   DESCRIPTION:
*       Destroy the resource manager.
*
*******************************************************************/

bool ResourceLoader_Destroy( ResourceLoader *loader )
{
bool ret = AssetFile_CloseForRead( &loader->reader );
*loader = {};

return( ret );

} /* ResourceLoader_Destroy() */


/*******************************************************************
*
*   ResourceLoader_GetModelMaterials()
*
*   DESCRIPTION:
*       Get the requested model's materials.
*
*******************************************************************/

uint32_t ResourceLoader_GetModelMaterials( const AssetFileAssetId asset_id, const uint32_t material_capacity, AssetFileModelMaterial *materials, ResourceLoader *loader )
{
if( !loader->reader.fhnd
 || materials == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &loader->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t material_count;
if( !AssetFile_ReadModelMaterials( material_capacity, &material_count, materials, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( material_count );

} /* ResourceLoader_GetModelMaterials() */


/*******************************************************************
*
*   ResourceLoader_GetModelMeshIndices()
*
*   DESCRIPTION:
*       Get the requested model's mesh indices by mesh index.
*
*******************************************************************/

uint32_t ResourceLoader_GetModelMeshIndices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t index_capacity, AssetFileModelIndex *indices, ResourceLoader *loader )
{
if( !loader->reader.fhnd
 || indices == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &loader->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t index_count;
if( !AssetFile_ReadModelMeshIndices( mesh_index, index_capacity, &index_count, indices, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( index_count );

} /* ResourceLoader_GetModelMeshIndices() */


/*******************************************************************
*
*   ResourceLoader_GetModelMeshVertices()
*
*   DESCRIPTION:
*       Get the requested model's mesh vertices by mesh index.
*
*******************************************************************/

uint32_t ResourceLoader_GetModelMeshVertices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t vertex_capacity, AssetFileModelIndex *material, AssetFileModelVertex *vertices, ResourceLoader *loader )
{
if( !loader->reader.fhnd
 || vertices == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &loader->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t vertex_count;
if( !AssetFile_ReadModelMeshVertices( mesh_index, vertex_capacity, material, &vertex_count, vertices, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( vertex_count );

} /* ResourceLoader_GetModelMeshVertices() */


/*******************************************************************
*
*   ResourceLoader_GetModelNodes()
*
*   DESCRIPTION:
*       Get the requested model's nodes.
*
*******************************************************************/

uint32_t ResourceLoader_GetModelNodes( const AssetFileAssetId asset_id, const uint32_t node_capacity, AssetFileModelNode *nodes, ResourceLoader *loader )
{
if( !loader->reader.fhnd
 || nodes == NULL
 || !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &loader->reader ) )
    {
    assert( false );
    return( 0 );
    }

uint32_t node_count;
if( !AssetFile_ReadModelNodes( node_capacity, &node_count, nodes, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( node_count );

} /* ResourceLoader_GetModelNodes() */


/*******************************************************************
*
*   ResourceLoader_GetModelStats()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

bool ResourceLoader_GetModelStats( const AssetFileAssetId asset_id, ResourceLoaderModelStats *out_stats, ResourceLoader *loader )
{
if( !loader->reader.fhnd
 || out_stats == NULL )
    {
    return( false );
    }

*out_stats = {};
if( !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_MODEL, &loader->reader ) )
    {
    return( false );
    }

if( !AssetFile_ReadModelStorageRequirements( &out_stats->vertex_count, &out_stats->index_count, &out_stats->mesh_count, &out_stats->node_count, &out_stats->material_count, &loader->reader ) )
    {
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( false );
    }

out_stats->vertex_stride   = sizeof(AssetFileModelVertex);
out_stats->index_stride    = sizeof(AssetFileModelIndex);
out_stats->material_stride = sizeof(AssetFileModelMaterial);

do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( true );

} /* ResourceLoader_GetModelStats() */


/*******************************************************************
*
*   ResourceLoader_GetShader()
*
*   DESCRIPTION:
*       Read the shader code as binary.
*
*       Pass NULL for 'bytes' to query the required storage buffer
*       size.
*
*******************************************************************/

bool ResourceLoader_GetShader( const AssetFileAssetId asset_id, uint32_t *sz, uint8_t *bytes, ResourceLoader *loader )
{
if( !loader->reader.fhnd
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
if( !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_SHADER, &loader->reader ) )
    {
    return( false );
    }

if( !AssetFile_ReadShaderStorageRequirements( &shader_sz, &loader->reader ) )
    {
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( false );
    }
    
if( sz != NULL )
    {
    *sz = shader_sz;
    }

if( bytes == NULL )
    {
    /* just querying the shader size.  not an error */
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( true );
    }

if( storage_sz < shader_sz )
    {
    /* not enough storage to load from file */
    hard_assert_always();
    AssetFile_EndReadingAsset( &loader->reader );
    return( false );
    }
    
uint32_t read_sz = 0;
if( !AssetFile_ReadShaderBinary( storage_sz, &read_sz, bytes, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

debug_assert( read_sz == shader_sz );
do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( true );

} /* ResourceLoader_GetShader() */


/*******************************************************************
*
*   ResourceLoader_GetTexture()
*
*   DESCRIPTION:
*       Read the texture data as compressed image binary data.
*
*       Pass NULL for 'bytes' to query the required storage buffer
*       size.
*
*******************************************************************/

bool ResourceLoader_GetTexture( const AssetFileAssetId asset_id, uint32_t *sz, uint8_t *bytes, ResourceLoader *loader )
{
if( !loader->reader.fhnd
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

uint32_t texture_sz = 0;
if( !AssetFile_BeginReadingAsset( asset_id, ASSET_FILE_ASSET_KIND_TEXTURE, &loader->reader ) )
    {
    return( false );
    }

if( !AssetFile_ReadTextureStorageRequirements( &texture_sz, &loader->reader ) )
    {
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( false );
    }
    
if( sz != NULL )
    {
    *sz = texture_sz;
    }

if( bytes == NULL )
    {
    /* just querying the texture size.  not an error */
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( true );
    }

if( storage_sz < texture_sz)
    {
    /* not enough storage to load from file */
    hard_assert_always();
    AssetFile_EndReadingAsset( &loader->reader );
    return( false );
    }
    
uint32_t read_sz = 0;
if( !AssetFile_ReadTextureBinary( storage_sz, &read_sz, bytes, &loader->reader ) )
    {
    assert( false );
    do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );
    return( 0 );
    }

debug_assert( read_sz == texture_sz );
do_debug_assert( AssetFile_EndReadingAsset( &loader->reader ) );

return( true );

} /* ResourceLoader_GetTexture() */


/*******************************************************************
*
*   ResourceLoader_Init()
*
*   DESCRIPTION:
*       Initialize the resource manager.
*
*******************************************************************/

bool ResourceLoader_Init( ResourceLoader *loader )
{
*loader = {};

if( !OpenAssetFile( ASSETS_FILE_NAME, &loader->reader ) )
    {
    printf( "ResourceLoader - Failed to open assets package." );
    return( false );
    }

return( true );

} /* ResourceLoader_Init() */


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