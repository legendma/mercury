#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>

#include "ComUtilities.hpp"
#include "Universe.hpp"

using namespace ECS;

typedef struct _DX12Render
    {
    ID3D12Device       *device;
    IDXGIFactory1      *dxgi_factory;
    } DX12Render;

static DX12Render * AsDX12Render( Universe *universe );
static bool CreateDevice( DX12Render *render );


/*******************************************************************
*
*   Render_Init()
*
*   DESCRIPTION:
*       Initialize the graphics system.
*       Returns TRUE if the DirectX 12 library was successfully
*       loaded.
*
*******************************************************************/

bool Render_Init( Universe *universe )
{    
SingletonRenderComponent* component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = malloc( sizeof( DX12Render ) );
if( !component->ptr )
    {
    return( false );
    }

DX12Render *render = (DX12Render*)component->ptr;
memset( render, 0, sizeof( *render ) );

if( !CreateDevice( render ) )
    {
    assert( false );
    return( false );
    }

return( true );

} /* Render_Init() */


/*******************************************************************
*
*   Render_Destroy()
*
*   DESCRIPTION:
*       Destroy the Render system and free its resources.
*
*******************************************************************/

void Render_Destroy( Universe* universe )
{
DX12Render *render = AsDX12Render( universe );

ComSafeRelease( render->device );
ComSafeRelease( render->dxgi_factory );

free( render );
render = NULL;

} /* Render_Destroy() */


/*******************************************************************
*
*   Render_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void Render_DoFrame( float frame_delta, Universe *universe )
{

} /* Render_DoFrame() */


/*******************************************************************
*
*   AsDX12Render()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static DX12Render * AsDX12Render( Universe* universe )
{
SingletonRenderComponent * component = (SingletonRenderComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
return( (DX12Render *)component->ptr );

} /* AsDX12Render() */


/*******************************************************************
*
*   CreateDevice()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static bool CreateDevice( DX12Render *render )
{
ID3D12Device           *device;
IDXGIFactory4          *dxgi_factory;

#if defined( _DEBUG )
ID3D12Debug            *debug_info;
if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debug_info ) ) ) )
    {
    debug_info->EnableDebugLayer();
    ComSafeRelease( &debug_info );
    }
#endif

if( FAILED( CreateDXGIFactory2( 0, IID_PPV_ARGS( &dxgi_factory ) ) ) )
    {
    goto failure;
    }

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &device ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( dxgi_factory->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        goto failure;
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &device ) ) ) )
        {
        ComSafeRelease( &warp );
        goto failure;
        }
    }

/* fill out device for the system state */
render->dxgi_factory = dxgi_factory;
render->device       = device;
  
return( true );

failure:
    {
    ComSafeRelease( &dxgi_factory );
    ComSafeRelease( &device );
    return( false );
    }

} /* CreateDevice() */
