#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "RenderInitializers.hpp"
#include "Utilities.hpp"


#define IS_WINDOWED_MODE            TRUE

#define MSAA_IS_ON                  FALSE
#if MSAA_IS_ON
#define MSAA_SAMPLE_COUNT           ( 4 )
#define MSSA_SAMPLE_QUALITY         ( 1 )
#else
#define MSAA_SAMPLE_COUNT           ( 1 )
#define MSSA_SAMPLE_QUALITY         ( 0 )
#endif


namespace RenderInitializers
{

/*******************************************************************
*
*   GetBufferResourceDescriptor()
*
*******************************************************************/

D3D12_RESOURCE_DESC GetBufferResourceDescriptor( const uint32_t size )
{
D3D12_RESOURCE_DESC ret = {};
ret.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
ret.Format             = DXGI_FORMAT_UNKNOWN;
ret.Width              = (UINT64)size;
ret.Height             = 1;
ret.MipLevels          = 1;
ret.DepthOrArraySize   = 1;
ret.Flags              = D3D12_RESOURCE_FLAG_NONE;
ret.SampleDesc.Count   = 1;
ret.SampleDesc.Quality = 0;
ret.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
ret.Alignment          = 0;

return( ret );

} /* GetBufferResourceDescriptor() */


/*******************************************************************
*
*   GetDefaultHeapProperties()
*
*******************************************************************/

D3D12_HEAP_PROPERTIES GetDefaultHeapProperties()
{
D3D12_HEAP_PROPERTIES ret = {};
ret.Type                 = D3D12_HEAP_TYPE_DEFAULT;
ret.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
ret.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
ret.CreationNodeMask     = NODE_MASK_SINGLE_GPU;
ret.VisibleNodeMask      = NODE_MASK_SINGLE_GPU;

return( ret );

} /* GetDefaultHeapProperties() */


/*******************************************************************
*
*   GetDepthStencilResourceDescriptor()
*
*******************************************************************/

D3D12_RESOURCE_DESC GetDepthStencilResourceDescriptor( const uint16_t width, const uint16_t height )
{
D3D12_RESOURCE_DESC ret = {};
ret.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
ret.Format             = DEPTH_STENCIL_FORMAT;
ret.Width              = width;
ret.Height             = height;
ret.MipLevels          = 1;
ret.DepthOrArraySize   = 1;
ret.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
ret.SampleDesc.Count   = MSAA_SAMPLE_COUNT;
ret.SampleDesc.Quality = MSSA_SAMPLE_QUALITY;
ret.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
ret.Alignment          = 0;

return( ret );

} /* GetDepthStencilResourceDescriptor() */


/*******************************************************************
*
*   GetDescriptorRange()
*
*******************************************************************/

D3D12_DESCRIPTOR_RANGE GetDescriptorRange( const D3D12_DESCRIPTOR_RANGE_TYPE type, const uint8_t count, const uint8_t base_register, const uint8_t space )
{
D3D12_DESCRIPTOR_RANGE ret = {};
ret.RangeType                         = type;
ret.NumDescriptors                    = count;
ret.BaseShaderRegister                = base_register;
ret.RegisterSpace                     = space;
ret.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

return( ret );

} /* GetDescriptorRange() */


/*******************************************************************
*
*   GetHeapDescriptor()
*
*******************************************************************/

D3D12_DESCRIPTOR_HEAP_DESC GetHeapDescriptor( const uint16_t count, const bool is_shader, const D3D12_DESCRIPTOR_HEAP_TYPE type )
{
D3D12_DESCRIPTOR_HEAP_DESC ret = {};
ret.Type           = type;
ret.NumDescriptors = count;
ret.NodeMask       = NODE_MASK_SINGLE_GPU;
ret.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

if( is_shader )
    {
    ret.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    }

return( ret );

} /* GetHeapDescriptor() */


/*******************************************************************
*
*   GetRasterizerDescriptorDefault()
*
*******************************************************************/

D3D12_RASTERIZER_DESC GetRasterizerDescriptorDefault( const D3D12_FILL_MODE fill_mode )
{
D3D12_RASTERIZER_DESC ret = {};
ret.FillMode              = fill_mode;
ret.CullMode              = D3D12_CULL_MODE_BACK; /* TODO <MPA> - Revisit this after compute shader backface culling is in place */
ret.FrontCounterClockwise = FALSE;
ret.DepthBias             = 0;
ret.DepthBiasClamp        = 0.0f;
ret.SlopeScaledDepthBias  = 0.0f;
ret.DepthClipEnable       = FALSE;
ret.MultisampleEnable     = MSAA_IS_ON;
ret.AntialiasedLineEnable = FALSE;
ret.ForcedSampleCount     = 0;
ret.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

return( ret );

} /* GetRasterizerDescriptorDefault() */


/*******************************************************************
*
*   GetRenderTargetTransition()
*
*******************************************************************/

D3D12_RESOURCE_BARRIER GetRenderTargetTransition( const D3D12_RESOURCE_STATES to, ID3D12Resource *target )
{
D3D12_RESOURCE_BARRIER ret = {};
ret.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
ret.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
ret.Transition.pResource   = target;
ret.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
ret.Transition.StateAfter  = to;

if( ret.Transition.StateAfter == D3D12_RESOURCE_STATE_RENDER_TARGET )
    {
    ret.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    }
else
    {
    ret.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

return( ret );

} /* GetRenderTargetTransition() */


/*******************************************************************
*
*   GetRenderTargetViewDescriptor()
*
*******************************************************************/

D3D12_RENDER_TARGET_VIEW_DESC GetRenderTargetViewDescriptor( const DXGI_FORMAT format )
{
D3D12_RENDER_TARGET_VIEW_DESC ret = {};
ret.Format               = format;
ret.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
ret.Texture2D.MipSlice   = 0;
ret.Texture2D.PlaneSlice = 0;

return( ret );

} /* GetRenderTargetViewDescriptor() */


/*******************************************************************
*
*   GetResourceTransition()
*
*******************************************************************/

D3D12_RESOURCE_BARRIER GetResourceTransition( const D3D12_RESOURCE_STATES from, const D3D12_RESOURCE_STATES to, ID3D12Resource *resource )
{
D3D12_RESOURCE_BARRIER ret = {};
ret.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
ret.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
ret.Transition.pResource   = resource;
ret.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
ret.Transition.StateBefore = from;
ret.Transition.StateAfter  = to;

return( ret );

} /* GetResourceTransition() */


/*******************************************************************
*
*   GetRootParameter32BitConstantArray()
*
*******************************************************************/

D3D12_ROOT_PARAMETER GetRootParameter32BitConstantArray( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space, const uint8_t count )
{
D3D12_ROOT_PARAMETER ret = {};
ret.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
ret.ShaderVisibility         = visibility;
ret.Constants.ShaderRegister = shader_register;
ret.Constants.RegisterSpace  = space;
ret.Constants.Num32BitValues = count;

return( ret );

} /* GetRootParameter32BitConstantArray() */


/*******************************************************************
*
*   GetRootParameterDescriptorConstantBufferView()
*
*******************************************************************/

D3D12_ROOT_PARAMETER GetRootParameterDescriptorConstantBufferView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space )
{
D3D12_ROOT_PARAMETER ret = {};
ret.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
ret.ShaderVisibility          = visibility;
ret.Descriptor.ShaderRegister = shader_register;
ret.Descriptor.RegisterSpace  = space;

return( ret );

} /* GetRootParameterDescriptorConstantBufferView() */


/*******************************************************************
*
*   GetRootParameterDescriptorShaderResourceView()
*
*******************************************************************/

D3D12_ROOT_PARAMETER GetRootParameterDescriptorShaderResourceView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space )
{
D3D12_ROOT_PARAMETER ret = {};
ret.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
ret.ShaderVisibility          = visibility;
ret.Descriptor.ShaderRegister = shader_register;
ret.Descriptor.RegisterSpace  = space;

return( ret );

} /* GetRootParameterDescriptorShaderResourceView() */


/*******************************************************************
*
*   GetRootParameterDescriptorTable()
*
*******************************************************************/

D3D12_ROOT_PARAMETER GetRootParameterDescriptorTable( const D3D12_SHADER_VISIBILITY visibility, const D3D12_DESCRIPTOR_RANGE *ranges, const uint8_t count )
{
D3D12_ROOT_PARAMETER ret = {};
ret.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
ret.ShaderVisibility                    = visibility;
ret.DescriptorTable.pDescriptorRanges   = ranges;
ret.DescriptorTable.NumDescriptorRanges = count;

return( ret );

} /* GetRootParameterDescriptorTable() */


/*******************************************************************
*
*   GetRootParameterDescriptorUnorderedAccessView()
*
*******************************************************************/

D3D12_ROOT_PARAMETER GetRootParameterDescriptorUnorderedAccessView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space )
{
D3D12_ROOT_PARAMETER ret = {};
ret.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_UAV;
ret.ShaderVisibility          = visibility;
ret.Descriptor.ShaderRegister = shader_register;
ret.Descriptor.RegisterSpace  = space;

return( ret );

} /* GetRootParameterDescriptorUnorderedAccessView() */


/*******************************************************************
*
*   GetRootSignatureDescriptor()
*
*******************************************************************/

D3D12_ROOT_SIGNATURE_DESC GetRootSignatureDescriptor( const D3D12_ROOT_SIGNATURE_FLAGS flags, const D3D12_ROOT_PARAMETER *parameters, const uint16_t parameter_count, const D3D12_STATIC_SAMPLER_DESC *samplers, const uint16_t sampler_count )
{
D3D12_ROOT_SIGNATURE_DESC ret = {};
ret.Flags             = flags;
ret.pParameters       = parameters;
ret.NumParameters     = parameter_count;
ret.pStaticSamplers   = samplers;
ret.NumStaticSamplers = sampler_count;

return( ret );

} /* GetRootSignatureDescriptor() */


/*******************************************************************
*
*   GetShaderResourceViewDescriptorTexture2D()
*
*******************************************************************/

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDescriptorTexture2D( const DXGI_FORMAT format, const uint16_t mip_levels )
{
D3D12_SHADER_RESOURCE_VIEW_DESC ret = {};
ret.Format                        = format;
ret.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
ret.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
ret.Texture2D.MostDetailedMip     = 0;
ret.Texture2D.MipLevels           = mip_levels;
ret.Texture2D.PlaneSlice          = 0;
ret.Texture2D.ResourceMinLODClamp = 0.0f;

return( ret );

} /* GetShaderResourceViewDescriptorTexture2D() */


/*******************************************************************
*
*   GetSwapChainDescriptor()
*
*******************************************************************/

DXGI_SWAP_CHAIN_DESC GetSwapChainDescriptor( const uint16_t width, const uint16_t height, HWND hwnd )
{
DXGI_SWAP_CHAIN_DESC  ret    = {};
DXGI_MODE_DESC       *mode   = &ret.BufferDesc;
DXGI_SAMPLE_DESC     *sample = &ret.SampleDesc;

ret.BufferCount  = SWAP_CHAIN_DOUBLE_BUFFER;
ret.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
ret.Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
ret.OutputWindow = hwnd;
ret.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
ret.Windowed     = IS_WINDOWED_MODE;

mode->Width                   = width;
mode->Height                  = height;
mode->RefreshRate.Numerator   = 60;
mode->RefreshRate.Denominator = 1;
mode->Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
mode->ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
mode->Format                  = RENDER_TARGET_FORMAT;

sample->Count   = MSAA_SAMPLE_COUNT;
sample->Quality = MSSA_SAMPLE_QUALITY;

return( ret );

} /* GetSwapChainDescriptor() */


/*******************************************************************
*
*   GetTexture2DResourceDescriptor()
*
*******************************************************************/

D3D12_RESOURCE_DESC GetTexture2DResourceDescriptor( const uint16_t width, const uint16_t height, const TextureUsage usage, const DXGI_FORMAT format )
{
D3D12_RESOURCE_DESC ret = {};
ret.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
ret.Alignment = 0;
ret.Width     = width;
ret.Height    = height;
ret.DepthOrArraySize = 1;
ret.MipLevels = 1;
ret.Format    = format;
ret.SampleDesc.Count = MSAA_SAMPLE_COUNT;
ret.SampleDesc.Quality = MSSA_SAMPLE_QUALITY;
ret.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
ret.Flags  = D3D12_RESOURCE_FLAG_NONE;

switch( usage )
    {
    case TEXTURE_USAGE_RENDER_TARGET:
        ret.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        break;

    case TEXTURE_USAGE_DEPTH_BUFFER:
        ret.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        break;

    default:
        debug_assert( usage == TEXTURE_USAGE_SHADER_RESOURCE_ONLY );
        break;
    }

return( ret );

} /* GetTexture2DResourceDescriptor() */


/*******************************************************************
*
*   GetUploadHeapProperties()
*
*******************************************************************/

D3D12_HEAP_PROPERTIES GetUploadHeapProperties()
{
D3D12_HEAP_PROPERTIES ret = {};
ret.Type                 = D3D12_HEAP_TYPE_UPLOAD;
ret.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
ret.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
ret.CreationNodeMask     = NODE_MASK_SINGLE_GPU;
ret.VisibleNodeMask      = NODE_MASK_SINGLE_GPU;

return( ret );

} /* GetUploadHeapProperties() */


} /* namespace RenderInitializers */