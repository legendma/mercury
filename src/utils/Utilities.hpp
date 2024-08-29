#pragma once
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>

#include "Global.hpp"
#include "Math.hpp"

#define MAX_FILEPATH_LENGTH         ( 400 )
static const char *RELATIVE_ROOT_DIRECTORY[] =
    {
    ".\\",                          /* deployed (w/ asset deploy)   */
    "..\\bin\\deploy\\"             /* non-deployed (visual studio) */
    };

static const u32   MAGIC_INIT = 0xcafebabe;

typedef Float4 Color4f;


/*******************************************************************
*
*   _concat()
*
*   DESCRIPTION:
*       Concatenate two fields.
*
*******************************************************************/

#define _concat( _a, _b ) \
    _a##_b


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
*   align_as()
*
*   DESCRIPTION:
*       Set a type's required alignment.
*
*******************************************************************/

#if defined( _MSC_VER )
#define align_as( _type ) \
    ( _Alignas( _type ) )
#endif


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

static __inline uint64_t align_adjust
    (
    const void         *address,    /* base address                 */
    const u64           alignment   /* bytes alignment of type      */
    )
{
u64 ret = alignment;
ret -= 1 + (u64)( ( (uptr)address + alignment - 1 ) % alignment );

return( ret );

}   /* align_adjust() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       align_size_round_down
*
*   DESCRIPTION:
*       Round a size down to the next given power-of-two.
*
*********************************************************************/

static __inline u64 align_size_round_down
    (
    const u64           sz,    /* address to round             */
    const u64           pow2   /* power of two                 */
    )
{
u64 mask = pow2 - 1;
debug_assert( pow2 > 1 );
debug_assert( !( mask & pow2 ) );
return( sz & ~mask );

}   /* align_size_round_down() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       align_size_round_up
*
*   DESCRIPTION:
*       Round a size up to the next given power-of-two.
*
*********************************************************************/

static __inline u64 align_size_round_up
    (
    const uint64_t      sz,    /* address to round             */
    const uint64_t      pow2   /* power of two                 */
    )
{
u64 mask = pow2 - 1;
debug_assert( pow2 > 1 );
debug_assert( !( mask & pow2 ) );
return( ( sz + mask ) & ~mask );

}   /* align_size_round_up() */


/*******************************************************************
*
*   bit_width64()
*
*   DESCRIPTION:
*       Count the number of bits taken by the given value.
*
*******************************************************************/

#define bit_width64( _value )                                       \
    ( (_value) <     1ull ? 0ull :                                  \
      ( (_value) <     2ull ? 1ull :                                \
        ( (_value) <     4ull ? 2ull :                              \
          ( (_value) <     8ull ? 3ull :                            \
            ( (_value) <    16ull ? 4ull :                          \
              ( (_value) <    32ull ? 5ull :                        \
                ( (_value) <    64ull ? 6ull :                      \
                  ( (_value) <   128ull ? 7ull :                    \
                    ( (_value) <   256ull ? 8ull :                  \
                      ( (_value) <   512ull ? 9ull :                \
                        ( (_value) <  1024ull ? 10ull :             \
                          ( (_value) <  2048ull ? 11ull :           \
                            ( (_value) <  4096ull ? 12ull :         \
                              ( (_value) <  8192ull ? 13ull :       \
                                ( (_value) < 16384ull ? 14ull :     \
                                  ( (_value) < 32768ull ? 15ull :   \
                                    ( (_value) < 65536ull ? 16ull : \
                                     0 ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) )


/*******************************************************************
*
*   clear_bits()
*
*   DESCRIPTION:
*       Clear the given bits
*
*******************************************************************/

#define clear_bits( _field, _bits )                                \
    ( (_field) &= ~(_bits) )


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
*   expand_macro()
*
*   DESCRIPTION:
*       Work around a non-standard preprocessor issue in MSVC.
*
*******************************************************************/

#define expand_macro( _args ) \
    (_args)


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
    expand_macro( _cnt_of_va_args( __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1 ) )


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
    ( ( 1ll << ( sizeof(_t) * CHAR_BIT - 2 ) ) - 1 + ( 1ll << ( sizeof(_t) * CHAR_BIT - 2 ) ) )


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
    ( ( ( 1ull << ( sizeof(_t) * CHAR_BIT - 1 ) ) - 1 + ( 1ull << ( sizeof(_t) * CHAR_BIT - 1 ) ) ) )
            

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
*   set_bits()
*
*   DESCRIPTION:
*       Set the given bits
*
*******************************************************************/

#define set_bits( _field, _bits )                                   \
    ( (_field) |= (_bits) )


/*******************************************************************
*
*   set_clear_bits()
*
*   DESCRIPTION:
*       Conditionally set or clear the given bits.
*
*******************************************************************/

#define set_clear_bits( _field, _bits, _test )                      \
    ( (_field) ^= ( ( -( (_test) != FALSE ) ^ (_field) ) & (_bits) ) )


/*******************************************************************
*
*   shift_bits()
*
*   DESCRIPTION:
*       Set the number of bits, at the given shift.
*
*******************************************************************/

#define shift_bits( _count, _shift ) \
    (u32)( ( ( 1 << (_count) ) - 1 ) << (_shift) )


/*******************************************************************
*
*   shift_bits64()
*
*   DESCRIPTION:
*       Set the number of bits, at the given shift.
*
*******************************************************************/

#define shift_bits64( _count, _shift ) \
    (u64)( ( ( 1ull << (_count) ) - 1ull ) << (_shift) )


/*******************************************************************
*
*   test_any_bits()
*
*   DESCRIPTION:
*       Are any of the given bits set on the given bitfield?
*
*******************************************************************/

#define test_any_bits( _field, _test ) \
    ( ( _field & _test ) != 0 )


/*******************************************************************
*
*   test_bits()
*
*   DESCRIPTION:
*       Are all of the given bits set on the given bitfield?
*
*******************************************************************/

#define test_bits( _field, _test ) \
    ( ( _field & _test ) == (_test) )


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


/*******************************************************************
*
*   Utilities_PointerToByteOffset
*
*   DESCRIPTION:
*       Find the byte offset represented from the given pointer from
*       the given base address.
*
*******************************************************************/

static inline uint64_t Utilities_PointerToByteOffset( const void *base, const void *pointer )
{
hard_assert( pointer >= base );
const uint8_t *base_ptr    = (uint8_t*)base;
const uint8_t *pointer_ptr = (uint8_t*)pointer;

return( (uint64_t)( pointer_ptr - base_ptr ) );

} /* Utilities_PointerToByteOffset() */


/*******************************************************************
*
*   Utilities_ShellSortU32Ascending
*
*   DESCRIPTION:
*       Shell-sort an array of 32-bit unsigned integers into ascending
*       order.
*
*******************************************************************/

static inline void Utilities_ShellSortU32Ascending( const uint32_t count, uint32_t *arr )
{
static const uint32_t CIURA_GAPS[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
for( uint32_t it_gap = 0; it_gap < cnt_of_array( CIURA_GAPS ); it_gap++ )
    {
    uint32_t gap = CIURA_GAPS[ it_gap ];
    uint32_t temp;
    for( uint32_t i = gap; i < count; i++ )
        {
        temp = arr[ i ];
        uint32_t j;
        for( j = i; j >= gap && arr[ j - gap ] > temp; j -= gap )
            {
            arr[ j ] = arr[ j - gap ];
            }

        arr[ j ] = temp;
        }
    }

} /* Utilities_ShellSortU32Ascending() */

