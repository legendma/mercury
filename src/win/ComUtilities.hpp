#pragma once

#include <unknwn.h>
#include <winerror.h>

#include "Utilities.hpp"


/*******************************************************************
*
*   ComSafeRelease()
*
*   DESCRIPTION:
*       Utility to free a COM pointer if it is valid, and invalidate
*       it.
*
*******************************************************************/

#define ComSafeRelease( _p ) \
    _ComSafeRelease( (IUnknown**)_p )

static inline void _ComSafeRelease( IUnknown **p )
{
if( !(*p) )
    {
    return;
    }

(*p)->Release();
*p = NULL;

} /* _ComSafeRelease() */


/*******************************************************************
*
*   _hr()
*
*   DESCRIPTION:
*       Hard assert that a COM operation succeeded.
*
*******************************************************************/

#define _hr( _call ) \
    hard_assert( SUCCEEDED( _call ) )


