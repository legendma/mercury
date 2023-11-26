#pragma once

#include "AssetFile.hpp"
#include "HashMap.hpp"


#define MAX_UNIQUE_RESOURCES        ( 1000 )
#define HASH_MAP_LUFT               ( 1.3f )
#define hash_map_capacity( _count ) \
    (uint32_t)( HASH_MAP_LUFT * (float)(_count) )

typedef struct _RMHashMapValue
    {
    uint32_t            ref_count;  /* reference count              */
    } RMHashMapValue;

typedef struct _RMHashMap
    {
    HashMapKey          keys[ hash_map_capacity( MAX_UNIQUE_RESOURCES ) ];
    RMHashMapValue      values[ hash_map_capacity( MAX_UNIQUE_RESOURCES ) ];
    HashMap             map;
    } RMHashMap;

typedef struct _ResourceManager
    {
    RMHashMap           map;
    AssetFileReader     reader;
    } ResourceManager;

bool ResourceManager_Init( ResourceManager *manager );