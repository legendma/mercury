#include <stdint.h>
#include <stdlib.h>

#include "LinearAllocator.hpp"
#include "ShaderCache.hpp"

namespace Render { namespace Shader
{

static void * AsyncMain( void *args );


/*******************************************************************
*
*   Cache_Create()
*
*   DESCRIPTION:
*       Create a shader cache of the given capacity.
*
*******************************************************************/

bool Cache_Create( const uint32_t capacity, const uint32_t storage_sz, Cache *cache )
{
*cache = {};
if( cache == NULL
 || capacity == 0 )
    {
    return( false );
    }

if( !ResourceLoader_Init( &cache->loader ) )
    {
    return( false );
    }

if( !LinearAllocator_Init( storage_sz, &cache->allocator ) )
    {
    ResourceLoader_Destroy( &cache->loader );
    return( false );
    }

cache->map.capacity = HASH_MAP_ADJUST_RECOMMENDED_LUFT( capacity );

cache->map.keys       = (HashMapKey*)malloc( cache->map.capacity * sizeof(HashMapKey) );
cache->map.frees      = (uint32_t*)malloc( cache->map.capacity * sizeof(uint32_t) );
cache->map.values     = (CacheEntry*)malloc( cache->map.capacity * sizeof(CacheEntry) );
cache->async.requests = (CacheAsyncRequest*)malloc( capacity * sizeof(CacheAsyncRequest) );

if( !cache->map.keys
 || !cache->map.values
 || !cache->async.requests )
    {
    ResourceLoader_Destroy(&cache->loader);
    LinearAllocator_Destroy( &cache->allocator );
    free( cache->map.keys );
    free( cache->map.frees );
    free( cache->map.values );
    free( cache->async.requests );
    return( false );
    }

memset( cache->async.requests, 0, capacity * sizeof(CacheAsyncRequest) );

cache->async.request_capacity = capacity;
cache->async.request_mutex = PTHREAD_MUTEX_INITIALIZER;

HashMap_Init( cache->map.capacity, sizeof(*cache->map.values), &cache->map.map, cache->map.keys, cache->map.frees, cache->map.values );

return( true );

} /* Cache_Create() */


/*******************************************************************
*
*   Cache_AddAsyncRequest()
*
*   DESCRIPTION:
*       Add a new asynchronous load request.
*
*******************************************************************/

bool Cache_AddAsyncRequest( const AssetFileAssetId shader_id, Cache *cache )
{
pthread_mutex_lock( &cache->async.request_mutex );
if( cache->async.request_count >= cache->async.request_capacity )
    {
    pthread_mutex_unlock( &cache->async.request_mutex );
    return( false );
    }

cache->async.requests[ cache->async.request_count++ ] = shader_id;
pthread_mutex_unlock( &cache->async.request_mutex );

return( true );

} /* Cache_AddAsyncRequest() */


/*******************************************************************
*
*   Cache_Destroy()
*
*   DESCRIPTION:
*       Free the resources of the given shader cache.*       
*
*******************************************************************/

void Cache_Destroy( Cache *cache )
{
pthread_join( cache->async.thread, NULL );
pthread_mutex_destroy( &cache->async.request_mutex );

ResourceLoader_Destroy( &cache->loader );
LinearAllocator_Destroy( &cache->allocator );
free( cache->map.keys );
free( cache->map.frees );
free( cache->map.values );
free( cache->async.requests );

*cache = {};

} /* Cache_Destroy() */


/*******************************************************************
*
*   Cache_GetLoaded()
*
*   DESCRIPTION:
*       Get a shader, if it is loaded.  (Need to request it if not)
*
*******************************************************************/

uint8_t * Cache_GetLoaded( const AssetFileAssetId shader_id, Cache *cache )
{
uint8_t *ret = NULL;

pthread_mutex_lock( &cache->async.request_mutex );
ret = (uint8_t*)HashMap_At( shader_id, &cache->map.map );
pthread_mutex_unlock( &cache->async.request_mutex );

return( ret );

} /* Cache_GetLoaded() */


/*******************************************************************
*
*   Cache_LoadAsync()
*
*   DESCRIPTION:
*       Start the asynchrous load thread.
*
*******************************************************************/

void Cache_LoadAsync( const char *thread_name, Cache *cache )
{
pthread_mutex_lock( &cache->async.request_mutex );
cache->async.is_running = true;
pthread_create( &cache->async.thread, NULL, &AsyncMain, cache );

if( thread_name )
    {
    pthread_setname_np( cache->async.thread, thread_name );
    }

} /* Cache_LoadAsync() */


/*******************************************************************
*
*   AsyncMain()
*
*   DESCRIPTION:
*       Asynchronous thread entry point.
*
*******************************************************************/

static void * AsyncMain( void *args )
{
if( !args )
    {
    debug_assert_always();
    pthread_exit( NULL );
    }

Cache *cache = (Cache*)args;

uint32_t request_count = cache->async.request_count;
for( uint32_t i = 0; i < request_count; i++ )
    {
    CacheAsyncRequest request = cache->async.requests[ request_count - ( i + 1 ) ];
    uint32_t storage_required = 0;
    if( !ResourceLoader_GetShader( request, &storage_required, NULL, &cache->loader ) )
        {
        debug_assert_always();
        pthread_exit( NULL );
        }

    CacheEntry entry = {};
    entry.code = (uint8_t*)LinearAllocator_Allocate( storage_required, &cache->allocator );
    if( !entry.code)
        {
        debug_assert_always();
        pthread_exit( NULL );
        }
        
    hard_assert( ResourceLoader_GetShader( request, &storage_required, entry.code, &cache->loader ) );
    hard_assert( HashMap_Insert( request, &entry, &cache->map.map ) != NULL );
    cache->async.request_count--;
    break;
    }

cache->async.is_running = false;
pthread_mutex_unlock( &cache->async.request_mutex );

return( NULL );

} /* AsyncMain() */


} }/* namespace Render::Shader */