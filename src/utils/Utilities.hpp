#pragma once
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>

#include "Math.hpp"

#define MAX_FILEPATH_LENGTH         ( 400 )
static const char *RELATIVE_ROOT_DIRECTORY[] =
    {
    ".\\",                          /* deployed (w/ asset deploy)   */
    "..\\bin\\deploy\\"             /* non-deployed (visual studio) */
    };

typedef Float4 Color4f;


/*******************************************************************
*
*   align_of_t()
*
*   DESCRIPTION:
*       Calculate the alignment of the given type.
*
*******************************************************************/

#if defined( _MSC_VER )
#define align_of_t( _type ) \
    ( alignof( _type ) )
#endif


/*********************************************************************
*
*   PROCEDURE NAME:
*       align_adjust
*
*   DESCRIPTION:
*       Calculate the amount the given address needs adjusting to
*       meet the given required alignment
*
*********************************************************************/

static __inline uint8_t align_adjust
    (
    const void         *address,    /* base address                 */
    const uint8_t       alignment   /* bytes alignment of type      */
    )
{
uint8_t ret = alignment;
ret -= 1 + (uint8_t)( ( (uintptr_t)address + alignment - 1 ) % alignment );

return( ret );

}   /* align_adjust() */


/*******************************************************************
*
*   clr_array()
*
*   DESCRIPTION:
*       Set the array's memory to all zeroes.
*
*******************************************************************/

#define clr_array( _arr ) \
    memset( _arr, 0, cnt_of_array( _arr ) * sizeof(*_arr) )


/*******************************************************************
*
*   clr_struct()
*
*   DESCRIPTION:
*       Set the given structure's memory to all zeroes.
*
*******************************************************************/

#define clr_struct( _ptr ) \
    memset( _ptr, 0, sizeof(*_ptr) )


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
*   cnt_of_array_type()
*
*   DESCRIPTION:
*       Evaluate to the integer number of items in an array type.
*
*******************************************************************/

#define cnt_of_array_type( _type ) \
    ( sizeof( *( (_type*)NULL ) / sizeof( ( (_type*)NULL ) )[ 0 ] ) )


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
*   debug_if()
*
*   DESCRIPTION:
*       Do something if the test is true and in debug mode.
*
*******************************************************************/

#if defined( _DEBUG )
#define debug_if( _test, _statements ) \
    if( _test )                        \
        {                              \
        _statements                    \
        }
#else
#define debug_if( _test, _statements )
#endif


/*******************************************************************
*
*   do_debug_assert()
*
*   DESCRIPTION:
*       Assert an expression, but still execute it in non-debug
*       mode.
*
*******************************************************************/

#if defined( _DEBUG )
#define do_debug_assert( _expression ) \
    assert( _expression )
#else
#define do_debug_assert( _expression ) \
    (void)(_expression)
#endif


/*******************************************************************
*
*   hard_assert()
*
*   DESCRIPTION:
*       Assert an expression, and abort if it fails (even in
*       release).
*
*******************************************************************/

#define hard_assert( _expression ) \
    if( !(_expression) )              \
        {                             \
        abort();                      \
        }


/*******************************************************************
*
*   hard_assert_always()
*
*   DESCRIPTION:
*       Abort program.
*
*******************************************************************/

#define hard_assert_always() \
        abort();


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
*   Utilities_HashString
*
*   DESCRIPTION:
*       Compute a hash key from the given string.
*
*******************************************************************/

static inline uint32_t Utilities_HashString( const char *str, const size_t len )
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

} /* Utilities_HashString() */


/*******************************************************************
*
*   Utilities_HashString2
*
*   DESCRIPTION:
*       Compute a hash key from the given string.
*
*******************************************************************/

#define Utilities_HashString2( _str ) \
    Utilities_HashString( _str, (uint32_t)strlen( _str ) )


/*******************************************************************
*
*   Utilities_HashPointer
*
*   DESCRIPTION:
*       Compute a hash key from the given pointer.
*
*******************************************************************/

static inline uint32_t Utilities_HashPointer( const void *ptr )
{
typedef union
    {
    const void         *ptr;
    struct
        {
        char            arr[ sizeof(void*) ];
        } a;
    } AsChars;

AsChars as;
as.ptr = ptr;

return( Utilities_HashString( as.a.arr, cnt_of_array( as.a.arr ) ) );

} /* Utilities_HashPointer() */


/*******************************************************************
*
*   Utilities_HashU32
*
*   DESCRIPTION:
*       Compute a hash key from the given uint32.
*
*******************************************************************/

static inline uint32_t Utilities_HashU32( const uint32_t value )
{
typedef union
    {
    uint32_t            u32;
    struct
        {
        char            arr[ sizeof(uint32_t) ];
        } a;
    } AsChars;

AsChars as;
as.u32 = value;

return( Utilities_HashString( as.a.arr, cnt_of_array( as.a.arr ) ) );

} /* Utilities_HashU32() */


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