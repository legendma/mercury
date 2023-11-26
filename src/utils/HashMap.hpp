#pragma once
#include <cstdint>

#include "Utilities.hpp"

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
    size_t              size;
    size_t              capacity;
    } HashMap;


void * HashMap_At( const uint32_t key, HashMap *h );
bool   HashMap_Delete( const uint32_t key, HashMap *h );
void   HashMap_Insert( const uint32_t key, const void *value, HashMap *h );
void   HashMap_Init( const size_t capacity, const size_t value_stride, HashMap *h, HashMapKey *keys, void *values );


/*******************************************************************
*
*   HashMap_KeyFromString
*
*   DESCRIPTION:
*       Compute a hash key from the given string.
*
*******************************************************************/

static inline uint32_t HashMap_KeyFromString( const char *str, const size_t len )
{
static const uint32_t SEED  = 0x811c9dc5;
static const uint32_t PRIME = 0x01000193;

uint32_t ret = SEED;
for( uint32_t i = 0; i < len; i++ )
    {
    ret ^= str[ i ];
    ret *= PRIME;
    }

return( ret );

} /* HashMap_KeyFromString() */


/*******************************************************************
*
*   HashMap_KeyFromU32
*
*   DESCRIPTION:
*       Compute a hash key from the given uint32.
*
*******************************************************************/

static inline uint32_t HashMap_KeyFromU32( const uint32_t key, void *value )
{
typedef union
    {
    uint32_t            u32;
    struct
        {
        char            arr[ 4 ];
        } a;
    } AsChars;

AsChars as;
as.u32 = key;

return( HashMap_KeyFromString( as.a.arr, cnt_of_array( as.a.arr ) ) );

} /* HashMap_KeyFromU32() */