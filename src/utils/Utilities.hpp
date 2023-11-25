#pragma once
#include <cassert>
#include <climits>

#include "Math.hpp"

#define MAX_FILEPATH_LENGTH         ( 100 )
static const char *RELATIVE_ROOT_DIRECTORY[] =
    {
    ".\\",                          /* deployed (w/ asset deploy)   */
    "..\\bin\\deploy\\"             /* non-deployed (visual studio) */
    };

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
*   expand_va_args()
*
*   DESCRIPTION:
*       Work around a non-standard preprocessor issue in MSVC.
*
*******************************************************************/

#define expand_va_args( _args ) \
    _args


/*******************************************************************
*
*   cnt_of_va_args()
*
*   DESCRIPTION:
*       Compute the number of variable arguments of a variadic
*       macro.
*
*******************************************************************/

#define _cnt_of_va_args( _1, _2, _3, _4, _5, _6, _7, _8, _9, _count, ... ) \
    _count

#define cnt_of_va_args( ... ) \
    expand_va_args( _cnt_of_va_args( __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1 ) )


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
*   debug_assert()
*
*   DESCRIPTION:
*       Assert an assumption - but only in debug mode.
*
*******************************************************************/

#if defined( _DEBUG )
#define debug_assert( _expression ) \
    assert( _expression )
#else
#define debug_assert( _expression )
#endif


/*******************************************************************
*
*   debug_assert_always()
*
*   DESCRIPTION:
*       Always assert in debug mode.
*
*******************************************************************/

#define debug_assert_always() \
    debug_assert( false )


/*******************************************************************
*
*   max_sint_value()
*
*   DESCRIPTION:
*       Compute the maximum value that can be stored in the given
*       signed type.
*
*******************************************************************/

#define max_sint_value( _t ) \
    ( ( 1LL << ( sizeof(_t) * CHAR_BIT - 2 ) ) - 1 + ( 1LL << ( sizeof(_t) * CHAR_BIT - 2 ) ) )


/*******************************************************************
*
*   max_uint_value()
*
*   DESCRIPTION:
*       Compute the maximum value that can be stored in the given
*       unsigned type.
*
*******************************************************************/

#define max_uint_value( _t ) \
    ( ( ( 1ULL << ( sizeof(_t) * CHAR_BIT - 1 ) ) - 1 + ( 1ULL << ( sizeof(_t) * CHAR_BIT - 1 ) ) ) )
            

/*******************************************************************
*
*   max_of_vals()
*
*   DESCRIPTION:
*       Evaluates to the largest of the given values.
*
*******************************************************************/

#define max_of_vals( _a, _b ) \
    ( _a > _b ? _a : _b )


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