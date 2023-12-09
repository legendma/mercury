#pragma once
#include <climits>
#include <cstdint>


typedef union _Float2
    {
    float f[ 2 ];
    struct
        {
        float           x;
        float           y;
        } v;
    } Float2;

typedef union _Float3
    {
    float f[ 3 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        } v;
    } Float3;

typedef union _Float4
    {
    float f[ 4 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        float           w;
        } v;
    } Float4;

typedef Float4 Quaternion;

typedef union _Float4x4
    {
    float f[ 4 ][ 4 ];
    struct
        {
        float           _11;
        float           _12;
        float           _13;
        float           _14;
        float           _21;
        float           _22;
        float           _23;
        float           _24;
        float           _31;
        float           _32;
        float           _33;
        float           _34;
        float           _41;
        float           _42;
        float           _43;
        float           _44;
        } v;
    struct
        {
        Float2          x_basis;
        Float2          y_basis;
        Float2          z_basis;
        Float2          translation;
        } n;
    } Float4x4;

/**************************/
/* Vector Math Functions*/
/**************************/

Float2 Math_Float2Addition( const Float2 A, const Float2 B );
Float3 Math_Float3Addition( const Float3 A, const Float3 B );
Float4 Math_Float4Addition( const Float4 A, const Float4 B );

Float2 Math_Float2Subtraction( const Float2 A, const Float2 B );
Float3 Math_Float3Subtraction( const Float3 A, const Float3 B );
Float4 Math_Float4Subtraction( const Float4 A, const Float4 B );

Float2 Math_Float2ScalarMultiply( const Float2 A, const float scalar );
Float3 Math_Float3ScalarMultiply( const Float3 A, const float scalar );
Float4 Math_Float4ScalarMultiply( const Float4 A, const float scalar );

Float2 Math_Float2Negative( const Float2 A );
Float3 Math_Float3Negative( const Float3 A );
Float4 Math_Float4Negative( const Float4 A );

float Math_Float2DotProduct( const Float2 A, const Float2 B );
float Math_Float3DotProduct( const Float3 A, const Float3 B );
float Math_Float4DotProduct( const Float4 A, const Float4 B );


Float3 Math_Float3CrossProduct( const Float3 A, const Float3 B );
/* TODO Add 4D cross product function */

float Math_Float2PseudoCrossProduct( const Float2 A, const Float2 B );
float Math_Float3CrossProductMagnitude( const Float3 A, const Float3 B );
float Math_Float4CrossProductMagnitude( const Float4 A, const Float4 B );

float Math_Float3BoxProduct( const Float3 A, const Float3 B, const Float3 C );
/* TODO Add 4D box product */

Float2 Math_Float2TrippleProduct( const Float2 A, const Float2 B, const Float2 C );
Float3 Math_Float3TrippleProduct( const Float3 A, const Float3 B, const Float3 C );
Float4 Math_Float4TrippleProduct( const Float4 A, const Float4 B, const Float4 C );

float Math_Float2Magnitude( const Float2 A );
float Math_Float3Magnitude( const Float3 A );
float Math_Float4Magnitude( const Float4 A );

float Math_Float2SquareMagnitude( const Float2 A );
float Math_Float3SquareMagnitude( const Float3 A );
float Math_Float4SquareMagnitude( const Float4 A );

float Math_AngleBetweenFloat2( const Float2 A, const Float2 B );
float Math_AngleBetweenFloat3( const Float3 A, const Float3 B );
float Math_AngleBetweenFloat4( const Float4 A, const Float4 B );

bool Math_IsAngleObtuseFloat2( const Float2 A, const Float2 B );
bool Math_IsAngleObtuseFloat3( const Float3 A, const Float3 B );
bool Math_IsAngleObtuseFloat4( const Float4 A, const Float4 B );

bool Math_IsAngleAcuteFloat2( const Float2 A, const Float2 B );
bool Math_IsAngleAcuteFloat3( const Float3 A, const Float3 B );
bool Math_IsAngleAcuteFloat4( const Float4 A, const Float4 B );

bool Math_IsAnglePerpendicularFloat2( const Float2 A, const Float2 B );
bool Math_IsAnglePerpendicularFloat3( const Float3 A, const Float3 B );
bool Math_IsAnglePerpendicularFloat4( const Float4 A, const Float4 B );

float Math_DistanceBetweenFloat2( const Float2 A, const Float2 B );
float Math_DistanceBetweenFloat3( const Float3 A, const Float3 B );
float Math_DistanceBetweenFloat4( const Float4 A, const Float4 B );

float Math_SquaredDistanceBetweenFloat2( const Float2 A, const Float2 B );
float Math_SquaredDistanceBetweenFloat3( const Float3 A, const Float3 B );
float Math_SquaredDistanceBetweenFloat4( const Float4 A, const Float4 B );



/*************************/
/* Bit Array Functions */
/*************************/



#define NUM_BITS_PER_BYTE \
    CHAR_BIT

/*******************************************************************
*
*   MATH_BITARRAY_COUNT()
*
*   DESCRIPTION:
*       Get the array count needed to store the given number of
*       bits, with the given type.
*
*******************************************************************/

#define MATH_BITARRAY_COUNT( _type, _bit_count ) \
    ( 1 + ( ( (_bit_count) - 1 ) / ( NUM_BITS_PER_BYTE * sizeof( _type ) ) ) )


/*******************************************************************
*
*   math_bitarray_bit()
*
*   DESCRIPTION:
*       Get the bit mask for the given bit and bit array.
*
*******************************************************************/

#define math_bitarray_bit( _bit_array, _bit ) \
    ( 1 << ( _bit % ( NUM_BITS_PER_BYTE * sizeof( *(_bit_array) ) ) ) )


/*******************************************************************
*
*   math_bitarray_index()
*
*   DESCRIPTION:
*       Get the array index for the given bit.
*
*******************************************************************/

#define math_bitarray_index( _bit_array, _bit ) \
    ( _bit / ( NUM_BITS_PER_BYTE * sizeof( *(_bit_array) ) ) )


/*******************************************************************
*
*   Math_BitArrayClear()
*
*   DESCRIPTION:
*       Clear the requested bit on the bit array.
*
*******************************************************************/

#define Math_BitArrayClear( _bit_array, _bit ) \
    (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] &= ~math_bitarray_bit( _bit_array, _bit )


/*******************************************************************
*
*   Math_BitArraySet()
*
*   DESCRIPTION:
*       Set the requested bit on the bit array.
*
*******************************************************************/

#define Math_BitArraySet( _bit_array, _bit ) \
    (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] |= math_bitarray_bit( _bit_array, _bit )


/*******************************************************************
*
*   Math_BitArrayIsSet()
*
*   DESCRIPTION:
*       Test if the requested bit is set on the array.
*
*******************************************************************/

#define Math_BitArrayIsSet( _bit_array, _bit ) \
    ( ( (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] & math_bitarray_bit( _bit_array, _bit ) ) != 0 )


/*******************************************************************
*
*   Utilities_ClampToMaxU32()
*
*   DESCRIPTION:
*       Return the value clamped to the maximum.
*
*******************************************************************/

static inline uint32_t Utilities_ClampToMaxU32( const uint32_t value, const uint32_t maximum )
{
return( value > maximum ? maximum : value );

} /* Utilities_ClampToMaxU32() */


/*******************************************************************
*
*   Utilities_ClampToMinU32()
*
*   DESCRIPTION:
*       Return the value clamped to the minimum.
*
*******************************************************************/

static inline uint32_t Utilities_ClampToMinU32( const uint32_t value, const uint32_t minimum )
{
return( value < minimum ? minimum : value );

} /* Utilities_ClampToMinU32() */