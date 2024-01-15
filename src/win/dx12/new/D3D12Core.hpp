#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <pthread.h>
#include <stdlib.h>
#include <wchar.h>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#include "D3D12Texture.hpp"
#include "D3D12Types.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Core
{

static const uint64_t EXPECTED_VRAM_8GB  = 8ull * 1024ull * 1024ull * 1024ull;
static const uint64_t NON_LEVEL_VRAM_2GB = 2u * 1024 * 1024 * 1024;

static const GUID CREATOR_ID = { 0xbd5d724e, 0x43df, 0x4523, { 0xa7, 0xa6, 0xe6, 0x40, 0xfe, 0xc5, 0x59, 0xf } };
static const int FRAME_COUNT        = 2;
static const int BACKBUFFER_COUNT   = 2;
static const float NEAR_DEPTH_VALUE = 1.0f;
static const float FAR_DEPTH_VALUE  = 0.0f;
static const unsigned int GPU_NODE_MASK = 0;

static const DXGI_FORMAT DEFAULT_RENDER_TARGET_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT DEFAULT_DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;


void                   AddTrashToCurrentFrame( IUnknown** trash );
void                   BeginNewFrame();
IMHeap *               CreateHeap( const uint64_t sz );
IMResource *           CreatePlacedBuffer( const uint64_t aligned_offset, const uint64_t aligned_sz, D3D12_RESOURCE_STATES initial_state, IMHeap *heap );
IMResource *           CreateUploadBuffer( const uint64_t sz );
void                   ExecuteCommandList( ID3D12CommandList *submission );
IMDevice *             GetDevice();
IMDXGI *               GetDXGI();
Descriptor::FreeHeap * GetDSVHeap();
Descriptor::RingHeap * GetGPUSRVHeap();
Descriptor::FreeHeap * GetSRVHeap();
Descriptor::FreeHeap * GetRTVHeap();
bool                   Init();
void                   Present( const Texture::RenderBuffer *buffer );
void                   Shutdown();


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
    D3D12::Core::_ComSafeRelease( (IUnknown**)_p )

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

#define NameD3DIndex( _obj, _index, _name )                                            \
    {                                                                                  \
    static const size_t MAX_NAME_LEN = 200;                                            \
    wchar_t *name_str = NULL;                                                          \
    name_str = (wchar_t*)malloc( MAX_NAME_LEN * sizeof( name_str ) );                  \
    if( name_str )                                                                     \
        {                                                                              \
        swprintf( name_str, MAX_NAME_LEN, L"%s[%d]", _name, _index );                  \
        (_obj)->SetName( name_str );                                                   \
        swprintf( name_str, MAX_NAME_LEN, L"%s[%d] was created...\n", _name, _index ); \
        ::OutputDebugStringW( name_str );                                              \
        free( name_str );                                                              \
        }                                                                              \
    }


/*******************************************************************
*
*   NameD3DString()
*
*   DESCRIPTION:
*       Name an string prefixed DirectX object.
*
*******************************************************************/

#define NameD3DString( _obj, _str, _name )                                                \
    {                                                                                     \
    static const size_t MAX_NAME_LEN = 200;                                               \
    wchar_t *name_str = NULL;                                                             \
    name_str = (wchar_t*)malloc( MAX_NAME_LEN * sizeof( name_str ) );                     \
    static const size_t convert_count = strlen( _str ) + 1;                               \
    wchar_t *wide_str = (wchar_t*)malloc( convert_count * sizeof(*wide_str) );            \
    size_t dummy;                                                                         \
    mbstowcs_s( &dummy, wide_str, convert_count, _str, convert_count - 1 );               \
    if( name_str )                                                                        \
        {                                                                                 \
        swprintf( name_str, MAX_NAME_LEN, L"%s(%s)", _name, wide_str );                   \
        (_obj)->SetName( name_str );                                                      \
        swprintf( name_str, MAX_NAME_LEN, L"%s(%s) was created...\n", _name, wide_str );  \
        ::OutputDebugStringW( name_str );                                                 \
        free( name_str );                                                                 \
        }                                                                                 \
                                                                                          \
    free( wide_str );                                                                     \
    }


/*******************************************************************
*
*   TossTrash()
*
*   DESCRIPTION:
*       Defer destruction of the given COM object until the frame
*       is recycled.
*
*******************************************************************/

#define TossTrash( _trash ) \
    D3D12::Core::AddTrashToCurrentFrame( (IUnknown**)_trash )


}} /* namspace D3D12::Core */