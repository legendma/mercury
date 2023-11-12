#pragma once

#include "Math.hpp"

typedef Vec4 Color4f;

/*******************************************************************
*
*   cnt_of_array()
*
*   DESCRIPTION:
*       Evaluate to the integer number of items in an array.
*
*       ONLY WORKS IF YOU HAVE THE ACTUAL ARRAY.  If the array has
*       been saved as a pointer, this macro will evaluate to 1,
*       regardless of the actual size of the array being pointed at.
*
*******************************************************************/

#define cnt_of_array( _arr ) \
    ( sizeof( _arr ) / sizeof( _arr[ 0 ] ) )


/*******************************************************************
*
*   compiler_assert()
*
*   DESCRIPTION:
*       Test the _eval parameter for truth, and generate a compiler
*       error if it does not equal true.
*
*******************************************************************/

#define compiler_assert( _eval, _filename ) \
    _compiler_assert( _eval, _filename, __LINE__ )

#define _compiler_assert( _eval, _filename, _line_num ) \
    typedef char assert_failed_##_filename##_##_line_num [ 2 * !!(_eval) - 1 ]


/*******************************************************************
*
*   max_uint_value()
*
*   DESCRIPTION:
*       Compute the maximum number of values that can be stored in
*       the given unsigned type.
*
*******************************************************************/

#define max_uint_value( _type ) \
    ( (size_t)1 << sizeof( _type ) * 8 )


/*******************************************************************
*
*   Utilities_MakeColor4f()
*
*   DESCRIPTION:
*       Helper function to construct a float4 color.
*
*******************************************************************/

static inline Color4f Utilities_MakeColor4f( float r, float g, float b, float a )
{
Color4f ret = {};
ret.v.x = r;
ret.v.y = g;
ret.v.z = b;
ret.v.w = a;

return( ret );

} /* Utilities_MakeColor4f() */