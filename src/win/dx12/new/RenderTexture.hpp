#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <pthread.h>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#include "Utilities.hpp"

namespace Render { namespace Core
{

typedef ID3D12Device10 *IMDevice;
typedef IDXGIFactory6  *IMDXGI;

extern IMDevice        *device = NULL;
extern IMDXGI          *dxgi   = NULL;


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


/*******************************************************************
*
*   NameD3D()
*
*   DESCRIPTION:
*       Name a DirectX object.
*
*******************************************************************/

#define NameD3D( _obj, _name )                                      \
    (_obj)->SetName( _name );                                       \
    ::OutputDebugStringW( _name" was created...\n" )


/*******************************************************************
*
*   NameD3DIndex()
*
*   DESCRIPTION:
*       Name an indexed DirectX object.
*
*******************************************************************/

#define NameD3DIndex( _obj, _index, _name )                           \
    {                                                                 \
    static const size_t MAX_NAME_LEN = 200;                           \
    wchar_t *name_str;                                                \
    name_str = (wchar_t*)malloc( MAX_NAME_LEN * sizeof( name_str ) ); \
    swprintf( name_str, MAX_NAME_LEN, L"%s[%d] was created...\n", _name, _index );     \
    (_obj)->SetName( name_str );                                      \
    ::OutputDebugStringW( name_str );                                 \
    free( name_str );                                                 \
    }

}}