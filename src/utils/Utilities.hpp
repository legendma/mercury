#pragma once


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