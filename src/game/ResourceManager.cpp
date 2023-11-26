#include <cstring>
#include <cstdint>

#include "AssetFile.hpp"
#include "HashMap.hpp"
#include "ResourceManager.hpp"

#define ASSETS_FILE_NAME            "AllAssets.bin"


static bool OpenAssetFile( const char *filename, AssetFileReader *input );


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

HashMap_Init( cnt_of_array( manager->map.keys ), sizeof(*manager->map.values), &manager->map.map, manager->map.keys, manager->map.values );

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