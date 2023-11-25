#include <cstring>

#include "HashMap.hpp"


static bool At( const uint32_t key, const HashMap *h, uint32_t *out );
static bool AtAvailable( const uint32_t key, const HashMap *h, uint32_t *out );
static void DeleteAtIndex( const uint32_t index, HashMap *h );
static uint32_t DoubleHash( const uint32_t hash );
static void EnterAtIndex( const uint32_t index, const uint32_t key, const void *value, HashMap *h );
static void * GetStorageAtIndex( const uint32_t index, HashMap *h );


/*******************************************************************
*
*   HashMap_At()
*
*   DESCRIPTION:
*       Get the value storage at the given key.
*
*******************************************************************/

void * HashMap_At( const uint32_t key, HashMap *h )
{
void *ret = NULL;
uint32_t found_index;
if( At( key, h, &found_index ) )
    {
    ret = GetStorageAtIndex( found_index, h );
    }

return( ret );

} /* HashMap_At() */


/*******************************************************************
*
*   HashMap_Delete()
*
*   DESCRIPTION:
*       Delete the given value from the map.
*
*******************************************************************/

bool HashMap_Delete( const uint32_t key, HashMap *h )
{
if( h->size == 0 )
    {
    return( true );
    }

uint32_t found_index;
if( At( key, h, &found_index ) )
    {
    /* found the key, so delete it */
    DeleteAtIndex( found_index, h );
    return( true );
    }

return( false );

} /* HashMap_Delete() */


/*******************************************************************
*
*   HashMap_Insert()
*
*   DESCRIPTION:
*       Insert the given value into the map.
*
*******************************************************************/

void HashMap_Insert( const uint32_t key, const void *value, HashMap *h )
{
if( h->size >= h->capacity )
    {
    /* Exceeded map capacity */
    assert( false );
    return;
    }

uint32_t found_index;
if( At( key, h, &found_index ) )
    {
    /* key already existed, so overwrite */
    EnterAtIndex( found_index, key, value, h );
    return;
    }

uint32_t empty_index;
assert( AtAvailable( key, h, &empty_index ) );
EnterAtIndex( empty_index, key, value, h );

h->size++;

} /* HashMap_Insert() */


/*******************************************************************
*
*   HashMap_Init()
*
*   DESCRIPTION:
*       Initialize the given hash map.
*
*******************************************************************/

void HashMap_Init( const size_t capacity, const size_t value_stride, HashMap *h, HashMapKey *keys, void *values )
{
*h = {};
h->capacity = capacity;
h->keys     = keys;
h->values   = values;

} /* HashMap_Init() */


/*******************************************************************
*
*   At()
*
*   DESCRIPTION:
*       Get the storage index at the given key.
*
*******************************************************************/

static bool At( const uint32_t key, const HashMap *h, uint32_t *out )
{
*out = (uint32_t) - 1;
if( h->size == 0 )
    {
    return( false );
    }

uint32_t try_bucket = key % h->capacity;
HashMapKey *key_ref = &h->keys[ try_bucket ];

if( key_ref->is_used
 && key_ref->key == key )
    {
    /* key found, return the index */
    *out = try_bucket;
    return( true );
    }

/* fall back to linear search */
uint32_t double_hash = DoubleHash( key );
for( uint32_t i = 0; i < h->capacity; h++ )
    {
    try_bucket = ( double_hash + i ) % h->capacity;
    key_ref = &h->keys[ try_bucket ];
    
    if( key_ref->is_used
     && key_ref->key == key )
        {
        /* key found, return the index */
        *out = try_bucket;
        return( true );
        }
    }

return( false );

} /* At() */


/*******************************************************************
*
*   AtAvailable()
*
*   DESCRIPTION:
*       Get the first free storage index at the given key.
*
*******************************************************************/

static bool AtAvailable( const uint32_t key, const HashMap *h, uint32_t *out )
{
*out = (uint32_t) - 1;
if( h->size == 0 )
    {
    return( false );
    }

uint32_t try_bucket = key % h->capacity;
HashMapKey *key_ref = &h->keys[ try_bucket ];

if( !key_ref->is_used )
    {
    /* key found, return the index */
    *out = try_bucket;
    return( true );
    }

/* fall back to linear search */
uint32_t double_hash = DoubleHash( key );
for( uint32_t i = 0; i < h->capacity; h++ )
    {
    try_bucket = ( double_hash + i ) % h->capacity;
    key_ref = &h->keys[ try_bucket ];
    
    if( !key_ref->is_used )
        {
        /* key found, return the index */
        *out = try_bucket;
        return( true );
        }
    }

return( false );

} /* AtAvailable() */


/*******************************************************************
*
*   DeleteAtIndex()
*
*   DESCRIPTION:
*       Erase the entry at the given index.
*
*******************************************************************/

static void DeleteAtIndex( const uint32_t index, HashMap *h )
{
h->keys[ index ].key = 0;
h->keys[ index ].is_used = false;

memset( GetStorageAtIndex( index, h ), 0, h->value_stride );
h->size--;

} /* DeleteAtIndex() */


/*******************************************************************
*
*   DoubleHash()
*
*   DESCRIPTION:
*       Return the second hash of the given key.
*
*******************************************************************/

static uint32_t DoubleHash( const uint32_t key )
{
typedef union
    {
    uint32_t            u32;
    struct
        {
        char            arr[ 4 ];
        } a;
    } AsChars;

static const uint32_t SEED  = 0x811c9dc5;
static const uint32_t PRIME = 0x01000193;

AsChars as;
as.u32 = key;

uint32_t ret = SEED;
for( uint32_t i = 0; i < cnt_of_array( as.a.arr ); i++ )
    {
    ret *= PRIME;
    ret ^= as.a.arr[ i ];
    }

return( ret );

} /* DoubleHash() */


/*******************************************************************
*
*   EnterAtIndex()
*
*   DESCRIPTION:
*       Enter the key and value at the given index.
*
*******************************************************************/

static void EnterAtIndex( const uint32_t index, const uint32_t key, const void *value, HashMap *h )
{
h->keys[ index ].is_used = true;
h->keys[ index ].key     = key;
memcpy( GetStorageAtIndex( index, h ), value, h->value_stride );

} /* EnterAtIndex() */


/*******************************************************************
*
*   GetStorageAtIndex()
*
*   DESCRIPTION:
*       Initialize the given hash map.
*
*******************************************************************/

static void * GetStorageAtIndex( const uint32_t index, HashMap *h )
{
void * ret = &( (uint8_t*)h->values )[index * h->value_stride ];
return( ret );

} /* GetStorageAtIndex() */