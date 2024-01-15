#pragma once
#include <cstdint>

#include "Utilities.hpp"


#define HASH_MAP_RECOMMENDED_LUFT   ( 1.333f )
#define HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) \
    (uint32_t)( 0.5f + HASH_MAP_RECOMMENDED_LUFT * (float)(_count) )    

#define HASH_MAP_IMPLEMENT( _name, _count, _value_type )                          \
    typedef struct _##_name                                                       \
        {                                                                         \
        HashMapKey          keys[ HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) ];   \
        _value_type         values[ HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) ]; \
        HashMap             map;                                                  \
        uint32_t            frees[ HASH_MAP_ADJUST_RECOMMENDED_LUFT( _count ) ];  \
        } _name

typedef struct _HashMapKey
    {
    uint32_t            key;
    uint32_t            value_index;
    bool                is_used;
    bool                was_deleted;
    } HashMapKey;

typedef struct _HashMap
    {
    HashMapKey         *keys;
    void               *values;
    size_t              value_stride;
    uint32_t            size;
    uint32_t            capacity;
    uint32_t            deleted_count;
    uint32_t           *frees;
    } HashMap;


void * HashMap_At( const uint32_t key, HashMap *h );
void   HashMap_Clear( HashMap *h );
bool   HashMap_Delete( const uint32_t key, HashMap *h );
void * HashMap_Insert( const uint32_t key, const void *value, HashMap *h );
void   HashMap_Init( const uint32_t capacity, const size_t value_stride, HashMap *h, HashMapKey *keys, uint32_t* frees, void *values );


/*******************************************************************
*
*   HashMap_InitImplementation()
*
*   DESCRIPTION:
*       Initialize a hash map created by the default implementation.
*
*******************************************************************/

#define HashMap_InitImplementation( _pimpl ) \
    HashMap_Init( cnt_of_array( (_pimpl)->values ), sizeof(*(_pimpl)->values), &(_pimpl)->map, (_pimpl)->keys, (_pimpl)->frees, (_pimpl)->values )
