#pragma once
#include <pthread.h>
#include <stdint.h>

#include "HashMap.hpp"
#include "LinearAllocator.hpp"
#include "ResourceLoader.hpp"

namespace Render { namespace Shader
{
typedef struct _CacheEntry
    {
    uint8_t            *code;
    } CacheEntry;

typedef AssetFileAssetId CacheAsyncRequest;

typedef struct _Cache
    {
    struct
        {
        pthread_t       thread;
        pthread_mutex_t request_mutex;
        CacheAsyncRequest
                       *requests;
        uint32_t        request_count;
        uint32_t        request_capacity;    
        bool            is_running;
        } async;

    LinearAllocator     allocator;
    ResourceLoader      loader;
    struct
        {
        HashMap         map;
        HashMapKey     *keys;
        CacheEntry     *values;
        uint32_t       *frees;
        uint32_t        capacity;
        } map;
    } Cache;

    
bool      Cache_AddAsyncRequest( const AssetFileAssetId shader_id, Cache *cache );
bool      Cache_Create( const uint32_t capacity, const uint32_t storage_sz, Cache *cache );
void      Cache_Destroy( Cache *cache );
uint8_t * Cache_GetLoaded( const AssetFileAssetId shader_id, Cache *cache );
void      Cache_LoadAsync( const char *thread_name, Cache* cache );

}} /* namespace Render::ShaderCache */