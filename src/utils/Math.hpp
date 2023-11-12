#pragma once

typedef union _Vec2
    {
    float f[ 2 ];
    struct
        {
        float           x;
        float           y;
        } v;
    } Vec2;

typedef union _Vec3
    {
    float f[ 3 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        } v;
    } Vec3;

typedef union _Vec4
    {
    float f[ 4 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        float           w;
        } v;
    } Vec4;

typedef Vec4 Quaternion;

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
    ( 1 + ( ( (_bit_count) - 1 ) / sizeof( _type ) ) )


/*******************************************************************
*
*   math_bitarray_bit()
*
*   DESCRIPTION:
*       Get the bit mask for the given bit and bit array.
*
*******************************************************************/

#define math_bitarray_bit( _bit_array, _bit ) \
    ( 1 << ( _bit % sizeof( (_bit_array ) ) ) )


/*******************************************************************
*
*   math_bitarray_index()
*
*   DESCRIPTION:
*       Get the array index for the given bit.
*
*******************************************************************/

#define math_bitarray_index( _bit_array, _bit ) \
    ( _bit / sizeof( (_bit_array) ) )


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
    (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] & math_bitarray_bit( _bit_array, _bit )