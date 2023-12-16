#include "ComUtilities.hpp"
#include "RenderEngine.hpp"
#include "RenderInitializers.hpp"
#include "RenderPassDefault.hpp"
#include "Utilities.hpp"


namespace RenderPass
{
static bool CreateRenderTarget( Default *pass );
static void FreeRenderTarget( Default *pass );


/*******************************************************************
*
*   Default_Destroy()
*
*   DESCRIPTION:
*       Destroy the default render pass.
*
*******************************************************************/

void Default_Destroy( Default *pass )
{
FreeRenderTarget( pass );

} /* Default_Destroy() */


/*******************************************************************
*
*   Default_Init()
*
*   DESCRIPTION:
*       Initialize the default render pass.
*
*******************************************************************/

bool Default_Init( const D3D12_CPU_DESCRIPTOR_HANDLE rtv, const D3D12_CPU_DESCRIPTOR_HANDLE srv, const DXGI_FORMAT format, RenderEngine::_Engine *engine, Default *pass )
{
clr_struct( pass );
pass->owner     = engine;
pass->rt_width  = engine->window.width;
pass->rt_height = engine->window.height;
pass->rt_format = format;
pass->rtv       = rtv;
pass->srv       = srv;



if( !CreateRenderTarget( pass ) )
    {
    return( false );
    }

return( true );

} /* Default_Init() */


/*******************************************************************
*
*   Default_OnResize()
*
*   DESCRIPTION:
*       Resize the shader render target.  Assumes the command queue
*       is flushed, and the render target is not in-use.
*
*******************************************************************/

void Default_OnResize( Default *pass )
{
CreateRenderTarget( pass );

} /* Default_OnResize() */


/*******************************************************************
*
*   CreateRenderTarget()
*
*   DESCRIPTION:
*       (Re)create the render target.
*
*******************************************************************/

static bool CreateRenderTarget( Default *pass )
{
FreeRenderTarget( pass );
D3D12_RESOURCE_DESC desc = RenderInitializers::GetTexture2DResourceDescriptor( pass->rt_width, pass->rt_height, RenderInitializers::TEXTURE_USAGE_RENDER_TARGET, pass->rt_format );
D3D12_HEAP_PROPERTIES props = RenderInitializers::GetDefaultHeapProperties();
if( FAILED( pass->owner->device.ptr->CreateCommittedResource( &props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, NULL, IID_PPV_ARGS( &pass->rt ) ) ) )
    {
    return( false );
    }

D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = RenderInitializers::GetRenderTargetViewDescriptor( pass->rt_format );
pass->owner->device.ptr->CreateRenderTargetView( pass->rt, &rtv_desc, pass->rtv );

D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = RenderInitializers::GetShaderResourceViewDescriptorTexture2D( pass->rt_format, pass->rt->GetDesc().MipLevels );
pass->owner->device.ptr->CreateShaderResourceView( pass->rt, &srv_desc, pass->srv );

return( true );

} /* CreateRenderTarget() */


/*******************************************************************
*
*   FreeRenderTarget()
*
*   DESCRIPTION:
*       Ensure the render target is freed.
*
*******************************************************************/

static void FreeRenderTarget( Default *pass )
{
ComSafeRelease( &pass->rt );

} /* FreeRenderTarget() */


} /* namespace RenderPass */