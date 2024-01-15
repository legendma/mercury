#include <stdint.h>

#include "D3D12Core.hpp"
#include "D3D12Descriptor.hpp"
#include "D3D12Initializers.hpp"
#include "D3D12Texture.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Texture
{


/*******************************************************************
*
*   GetClearValue()
*
*   DESCRIPTION:
*       Get a clear value (or not) from the texture creation
*       parameters.
*
*******************************************************************/

static inline const D3D12_CLEAR_VALUE * GetClearValue( const TextureCreateParams *params )
{
const D3D12_CLEAR_VALUE *ret = NULL;

if( params->desc
 && ( params->desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
   || params->desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ) )
   {
   ret = &params->clear_value;
   }

return( ret );

} /* GetClearValue() */


/*******************************************************************
*
*   DepthBuffer_Create()
*
*   DESCRIPTION:
*       Create a depth buffer resource.
*
*******************************************************************/

bool DepthBuffer_Create( const TextureCreateParams *params, DepthBuffer *buffer )
{
if( !Texture_Create( params, &buffer->texture ) )
    {
    return( false );
    }

debug_assert( params->mip_count > 0 
           && params->mip_count <= cnt_of_array( buffer->dsv ) );
buffer->mip_count = params->mip_count;

D3D12_DEPTH_STENCIL_VIEW_DESC desc = Initializers::GetDepthStencilViewDescriptor( params->desc->Format );
Descriptor::FreeHeap *dsv_heap = Core::GetDSVHeap();
for( uint32_t i = 0; i < params->mip_count; i++ )
    {
    buffer->dsv[ i ] = Descriptor::FreeHeap_Allocate( dsv_heap );
    desc.Texture2D.MipSlice++;
    }

return( true );

} /* DepthBuffer_Create() */


/*******************************************************************
*
*   DepthBuffer_Destroy()
*
*   DESCRIPTION:
*       Release the depth buffer's resources.
*
*******************************************************************/

void DepthBuffer_Destroy( DepthBuffer *buffer )
{
Descriptor::FreeHeap *dsv_heap = Core::GetDSVHeap();
for( uint32_t i = 0; i < buffer->mip_count; i++ )
    {
    Descriptor::FreeHeap_Free( &buffer->dsv[ i ], dsv_heap );
    }

Texture_Destroy( &buffer->texture );
clr_struct( buffer );

} /* DepthBuffer_Destroy() */


/*******************************************************************
*
*   GetPlacedTextureInfo()
*
*   DESCRIPTION:
*       Get the smallest allowed placed memory alignment for the
*       texture described by the given details.
*
*******************************************************************/

D3D12_RESOURCE_ALLOCATION_INFO GetPlacedTextureInfo( const D3D12_RESOURCE_DESC *desc )
{
D3D12_RESOURCE_DESC try_desc = *desc;
try_desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;

Core::IMDevice *device = Core::GetDevice();
D3D12_RESOURCE_ALLOCATION_INFO info = device->GetResourceAllocationInfo( Core::GPU_NODE_MASK, 1, &try_desc );
if( info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT )
    {
    /* small resource alignment not allowed, so query the default */
    try_desc.Alignment = 0;
    info = device->GetResourceAllocationInfo( Core::GPU_NODE_MASK, 1, &try_desc );
    }

return( info );

} /* GetPlacedTextureInfo() */


/*******************************************************************
*
*   GetSRGBFormat()
*
*   DESCRIPTION:
*       Get the sRGB equivalent of the given texture format.
*
*******************************************************************/

DXGI_FORMAT GetSRGBFormat( const DXGI_FORMAT base )
{
DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
switch( base )
    {
    /* DDS formats */
    case DXGI_FORMAT_BC1_UNORM:
        ret = DXGI_FORMAT_BC1_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC2_UNORM:
        ret = DXGI_FORMAT_BC2_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC3_UNORM:
        ret = DXGI_FORMAT_BC3_UNORM_SRGB;
        break;

    case DXGI_FORMAT_BC7_UNORM:
        ret = DXGI_FORMAT_BC7_UNORM_SRGB;
        break;

    /* uncompressed */
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        ret = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        ret = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        ret = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        break;

    /* pass-through */
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        ret = base;
        break;

    /* unexpected */
    default:
        hard_assert_always();
        break;
    }

return( ret );
    

} /* GetSRGBFormat() */


/*******************************************************************
*
*   RenderBuffer_Create()
*
*   DESCRIPTION:
*       Create a render buffer resource.
*
*******************************************************************/

bool RenderBuffer_Create( const TextureCreateParams *params, RenderBuffer *buffer )
{
if( !Texture_Create( params, &buffer->texture ) )
    {
    return( false );
    }

debug_assert( params->mip_count > 0 
           && params->mip_count <= cnt_of_array( buffer->rtv ) );
buffer->mip_count = params->mip_count;

D3D12_RENDER_TARGET_VIEW_DESC desc = Initializers::GetRenderTargetViewDescriptor( params->desc->Format );
Descriptor::FreeHeap *rtv_heap = Core::GetRTVHeap();
for( uint32_t i = 0; i < params->mip_count; i++ )
    {
    buffer->rtv[ i ] = Descriptor::FreeHeap_Allocate( rtv_heap );
    desc.Texture2D.MipSlice++;
    }

return( true );

} /* RenderBuffer_Create() */


/*******************************************************************
*
*   RenderBuffer_Destroy()
*
*   DESCRIPTION:
*       Release the render buffer's resources.
*
*******************************************************************/

void RenderBuffer_Destroy( RenderBuffer *buffer )
{
Descriptor::FreeHeap *rtv_heap = Core::GetRTVHeap();
for( uint32_t i = 0; i < buffer->mip_count; i++ )
    {
    Descriptor::FreeHeap_Free( &buffer->rtv[ i ], rtv_heap );
    }

Texture_Destroy( &buffer->texture );
clr_struct( buffer );

} /* RenderBuffer_Destroy() */


/*******************************************************************
*
*   Texture_Create()
*
*   DESCRIPTION:
*       Create a texture resource.
*
*******************************************************************/

bool Texture_Create( const TextureCreateParams *params, Texture *texture )
{
hard_assert( params
          && texture );

clr_struct( texture );
Core::IMDevice *device = Core::GetDevice();

if( params->resource )
    {
    hard_assert( params->heap == NULL );
    texture->resource = params->resource;
    }
else if( params->heap )
    {
    hard_assert( params->resource == NULL );

    /* custom heap resource */
    if( FAILED( device->CreatePlacedResource( params->heap, params->heap_offset, params->desc, params->initial_state, GetClearValue( params ), IID_PPV_ARGS( &texture->resource ) ) ) )
        {
        return( false );
        }
    }
else
    {
    /* default heap resource */
    hard_assert( params->resource == NULL
              && params->heap == NULL );

    if( FAILED( device->CreateCommittedResource( Initializers::GetDefaultHeapProperties(), D3D12_HEAP_FLAG_NONE, params->desc, params->initial_state, GetClearValue( params ), IID_PPV_ARGS( &texture->resource ) ) ) )
        {
        return( false );
        }
    }

texture->srv = Descriptor::FreeHeap_Allocate( Core::GetSRVHeap() );
device->CreateShaderResourceView( texture->resource, params->srv_desc, texture->srv.hcpu );

return( true );

} /* Texture_Create() */


/*******************************************************************
*
*   Texture_Destroy()
*
*   DESCRIPTION:
*       Release the provided texture's resources.
*
*******************************************************************/

void Texture_Destroy( Texture *texture )
{
Descriptor::FreeHeap_Free( &texture->srv, Core::GetSRVHeap() );
TossTrash( &texture->resource );

clr_struct( texture );

} /* Texture_Destroy() */


} }/* namespace D3D12::Texture */