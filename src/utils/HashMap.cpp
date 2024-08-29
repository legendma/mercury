#include <string.h>
#include <stdlib.h>

#include "HashMap.hpp"
#include "Utilities.hpp"


static bool     At( const uint32_t key, const HashMap *h, uint32_t *out );
static bool     AtAvailable( const uint32_t key, const HashMap *h, uint32_t *out );
static void     CheckForRehash( HashMap *h );
static void     DeleteAtIndex( const uint32_t index, HashMap *h );
static void     DoRehash( HashMap* h );
static uint32_t DoubleHash( const uint32_t hash );
static void     EnterAtIndex( const uint32_t index, const uint32_t key, const void *value, HashMap *h );
static void *   GetStorageAtIndex( const uint32_t index, HashMap *h );


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
*   HashMap_Clear()
*
*   DESCRIPTION:
*       Clear the map of values.
*
*******************************************************************/

void HashMap_Clear( HashMap *h )
{
memset( h->keys, 0, h->size * sizeof(*h->keys) * h->capacity );
memset( h->values, 0, h->value_stride * h->capacity );
h->size = 0;

} /* HashMap_Clear() */


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
    CheckForRehash( h );
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

void * HashMap_Insert( const uint32_t key, const void *value, HashMap *h )
{
if( h->size >= h->capacity )
    {
    /* Exceeded map capacity */
    assert( false );
    return( NULL );
    }

uint32_t found_index;
if( At( key, h, &found_index ) )
    {
    /* key already existed, so overwrite */
    EnterAtIndex( found_index, key, value, h );
    return( GetStorageAtIndex( found_index, h ) );
    }

uint32_t empty_index;
do_debug_assert( AtAvailable( key, h, &empty_index ) );
h->keys[ empty_index ].value_index = h->frees[ h->capacity - ( h->size + 1 ) ];
EnterAtIndex( empty_index, key, value, h );

h->size++;

return( GetStorageAtIndex( empty_index, h ) );

} /* HashMap_Insert() */


/*******************************************************************
*
*   HashMap_Init()
*
*   DESCRIPTION:
*       Initialize the given hash map.
*
*******************************************************************/

void HashMap_Init( const uint32_t capacity, const size_t value_stride, HashMap *h, HashMapKey *keys, uint32_t *frees, void *values )
{
*h = {};
h->capacity     = capacity;
h->keys         = keys;
h->frees        = frees;
h->values       = values;
h->value_stride = value_stride;

memset( keys, 0, capacity * sizeof(*keys) );

for( uint32_t i = 0; i < capacity; i++ )
    {
    h->keys[ i ].value_index = max_uint_value( uint32_t );
    }

for( uint32_t i = 0; i < capacity; i++ )
    {
    h->frees[ i ] = i;
    }

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
for( uint32_t i = 0; i < h->capacity; i++ )
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
    else if( !key_ref->is_used
          && !key_ref->was_deleted )
        {
        /* hit a dead open address, so stop the search */
        return( false );
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
if( h->size >= h->capacity )
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
for( uint32_t i = 0; i < h->capacity; i++ )
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
*   CheckForRehash()
*
*   DESCRIPTION:
*       Check if the hash map needs rehashing, and rehash if so.
*
*******************************************************************/

static void CheckForRehash( HashMap *h )
{
#define LOAD_FACTOR_REHASH_THRESHOLD        ( 0.5f )
#define USED_TO_CAPACITY_REHASH_THRESHOLD   ( 0.7f )
#define DELETED_TO_FREE_REHASH_THRESHOLD    ( 0.5f )

float load_factor = (float)( h->size + h->deleted_count ) / (float)( h->capacity );
if( load_factor < LOAD_FACTOR_REHASH_THRESHOLD )
    {
    /* load factor is acceptable, don't rehash */
    return;
    }

uint32_t free_count = h->capacity - h->size;
float delete_to_free_ratio = (float)h->deleted_count / (float)free_count;
if( delete_to_free_ratio < DELETED_TO_FREE_REHASH_THRESHOLD )
    {
    /* most of the free items are fresh (not deleted), so no rehash */
    return;
    }

float used_to_capacity_ratio = (float)h->size / (float)h->capacity;
if( used_to_capacity_ratio > USED_TO_CAPACITY_REHASH_THRESHOLD )
    {
    /* free count is too low to gain much from rehash */
    return;
    }

DoRehash( h );

#undef LOAD_FACTOR_REHASH_THRESHOLD
#undef USED_TO_CAPACITY_THRESHOLD
} /* CheckForRehash() */


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
h->keys[ index ].was_deleted = true;
h->keys[ index ].value_index = max_uint_value( uint32_t );

h->frees[ h->capacity - h->size ] = h->keys[ index ].value_index;
memset( GetStorageAtIndex( index, h ), 0, h->value_stride );
h->size--;
h->deleted_count++;

} /* DeleteAtIndex() */


/*******************************************************************
*
*   DoRehash()
*
*   DESCRIPTION:
*       Perform a rehash of the hash table.
*
*******************************************************************/

static void DoRehash( HashMap *h )
{
size_t key_table_sz = h->capacity * sizeof(*h->keys);
HashMapKey *saved = (HashMapKey*)malloc( key_table_sz );
if( !saved )
    {
    debug_assert_always();
    return;
    }

memcpy( saved, h->keys, key_table_sz );
memset( h->keys, 0, key_table_sz );
h->size = 0;
h->deleted_count = 0;

uint32_t processed_count = 0;
for( uint32_t i = 0; processed_count < h->size && i < h->capacity; i++ )
    {
    HashMapKey *old = &saved[ i ];
    if( !old->is_used )
        {
        continue;
        }

    processed_count++;

    uint32_t free_index;
    do_debug_assert( AtAvailable( old->key, h, &free_index ) );
    h->keys[ free_index ].value_index = old->value_index;
    EnterAtIndex( free_index, old->key, NULL, h );
    }

free( saved );

} /* DoRehash() */


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
if( h->keys[ index ].was_deleted )
    {
    h->keys[ index ].was_deleted = false;
    h->deleted_count--;
    }

if( value )
    {
    memcpy( GetStorageAtIndex( index, h ), value, h->value_stride );
    }

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
uint32_t value_index = h->keys[ index ].value_index;

void * ret = &( (uint8_t*)h->values )[ value_index * h->value_stride ];
return( ret );

} /* GetStorageAtIndex() */