#include <cstdio>

#include "Utilities.hpp"


/*******************************************************************
*
*   Utilities_ReadLineFromBuffer()
*
*   DESCRIPTION:
*       Read a trimmed line from the buffer.
*       Returns TRUE if output buffer has a length.
*
*******************************************************************/

bool Utilities_ReadLineFromBuffer( int *read_caret, const char *read, const int read_sz, char *out, const int out_sz )
{
int length = 0;
char read_char = 0;

if( out_sz <= 0 )
    {
    debug_assert_always();
    return( false );
    }

out[ 0 ] = 0;

/* l-trim */
bool end_of_buffer = true;
while( Utilities_ReadCharFromBuffer( read_caret, read, read_sz, &read_char ) )
    {
    end_of_buffer = false;
    if( read_char != ' ' )
        {
        (*read_caret)--;
        break;
        }
    }

/* read the full line */
while( Utilities_ReadCharFromBuffer( read_caret, read, read_sz, &read_char ) )
    {
    if( read_char == '\r' )
        {
        continue;
        }
    else if( read_char == '\n' )
        {
        break;
        }

    if( length >= out_sz - 1 )
        {
        /* output buffer too small */
        debug_assert_always();
        break;
        }

    out[ length++ ] = read_char;
    }

out[ length ] = 0;

/* r-trim */
while( length > 0 )
    {
    if( out[ length - 1 ] != '\r'
     && out[ length - 1 ] != ' ' )
        {
        break;
        }

    length--;
    }

return( !end_of_buffer );

} /* Utilities_ReadLineFromBuffer() */


/*******************************************************************
*
*   Utilities_ReadWholeTextFile()
*
*   DESCRIPTION:
*       Read the given text file to a new malloc'd buffer.
*       Returns TRUE if output buffer has a length.
*
*******************************************************************/

char * Utilities_ReadWholeTextFile( const char *file_path, int *buffer_sz )
{
hard_assert( file_path );
*buffer_sz = 0;

FILE *fhnd = fopen( file_path, "r" );
if( !fhnd )
    {
    return( nullptr );
    }

/* get the file size - would use fseek/ftell here, but it's reporting incorrect file length due to crlf (binary mode doesn't help) */
int file_sz = 0;
char dummy;
while( fread( &dummy, 1, 1, fhnd ) )
    {
    file_sz++;
    }

rewind( fhnd );
if( file_sz <= 0 )
    {
    fclose( fhnd );
    return( nullptr );
    }

/* read the entire file in our buffer then close it */
*buffer_sz = file_sz + 1;
char *ret = (char*)malloc( *buffer_sz );
if( !ret )
    {
    fclose( fhnd );
    hard_assert_always();
    return( nullptr );
    }

size_t read_sz = fread( ret, 1, file_sz, fhnd );
ret[ file_sz ] = 0;
hard_assert( (int)read_sz == file_sz );
fclose( fhnd );
fhnd = nullptr;

return( ret );

} /* Utilities_ReadWholeTextFile() */


/*******************************************************************
*
*   Utilities_StrContainsStr()
*
*   DESCRIPTION:
*       Does the given string contain the search string?
*
*******************************************************************/

bool Utilities_StrContainsStr( const char *str, const bool case_insensitive, const char *search )
{
if( !search
 || search[ 0 ] == 0 )
    {
    debug_assert_always();
    return( false );
    }

int search_i = 0;
for( int i = 0; str[ i ] != 0; i++ )
    {
    char this_char = str[ i ];
    char that_char = search[ search_i ];
    bool matches = ( this_char == that_char );

    /* handle case insensitivety */
    if( !matches
     && case_insensitive )
        {
        if( Utilities_CharIsLetterLowercase( that_char ) )
            {
            that_char = Utilities_CharMakeUppercase( that_char );
            matches = ( this_char == that_char );
            }
        else if( Utilities_CharIsLetterUppercase( that_char ) )
            {
            that_char = Utilities_CharMakeLowercase( that_char );
            matches = ( this_char == that_char );
            }
        }

    if( !matches )
        {
        search_i = 0;
        continue;
        }

    search_i++;
    if( search[ search_i ] == 0 )
        {
        return( true );
        }
    }

return( false );

} /* Utilities_StrContainsStr() */
