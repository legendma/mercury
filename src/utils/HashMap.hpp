#pragma once
#include <cstdint>

#include "Utilities.hpp"


#define HASH_MAP_RECOMMENDED_LUFT   ( 1.3f )
#define HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) \
    (uint32_t)( HASH_MAP_RECOMMENDED_LUFT * (float)(_count) )    

#define HASH_MAP_IMPLEMENT( _name, _count, _value_type )                          \
    typedef struct _##_name                                                       \
        {                                                                         \
        HashMapKey          keys[ HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) ];   \
        _value_type         values[ HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) ]; \
        HashMap             map;                                                  \
        } _name

typedef struct _HashMapKey
    {
    uint32_t            key;
    bool                is_used;
    } HashMapKey;

typedef struct _HashMap
    {
    HashMapKey         *keys;
    void               *values;
    size_t              value_stride;
    uint32_t            size;
    uint32_t            capacity;
    } HashMap;


void * HashMap_At( const uint32_t key, HashMap *h );
bool   HashMap_Delete( const uint32_t key, HashMap *h );
void * HashMap_Insert( const uint32_t key, const void *value, HashMap *h );
void   HashMap_Init( const uint32_t capacity, const size_t value_stride, HashMap *h, HashMapKey *keys, void *values );
