#pragma once

#include <cstdint>

#include "D3D12Types.hpp"


namespace D3D12 { namespace Initializers
{


static const bool ENABLE_DEPTH_TEST  = true;
static const bool DISABLE_DEPTH_TEST = false;

static const bool ENABLE_DEPTH_WRITE  = true;
static const bool DISABLE_DEPTH_WRITE = false;

#define D3D12_INITIALIZER_DECLARE_STATIC_SAMPLER( _register, _filter, _addrU, _addrV, _addrW ) \
	{																							\
	_filter,                                /* Filter           */								\
	_addrU,                                 /* AddressU         */								\
	_addrV,                                 /* AddressV         */								\
	_addrW,                                 /* AddressW         */								\
	0,                                      /* MipLODBias       */								\
	16,                                     /* MaxAnisotropy    */								\
	D3D12_COMPARISON_FUNC_LESS_EQUAL,       /* ComparisonFunc   */								\
	D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, /* BorderColor      */								\
	0.0f,                                   /* MinLOD           */								\
	D3D12_FLOAT32_MAX,                      /* MaxLOD           */								\
	_register,                              /* ShaderRegister   */								\
	0,                                      /* RegisterSpace    */								\
	D3D12_SHADER_VISIBILITY_ALL             /* ShaderVisibility */                              \
	}

typedef enum _TextureUsage
    {
    TEXTURE_USAGE_SHADER_RESOURCE_ONLY,
    TEXTURE_USAGE_RENDER_TARGET,
    TEXTURE_USAGE_DEPTH_BUFFER
    } TextureUsage;


D3D12_RESOURCE_DESC              GetBufferResourceDescriptor( const uint64_t size );
const D3D12_COMMAND_QUEUE_DESC * GetCommandQueueDescriptorUpload();
const D3D12_HEAP_PROPERTIES *    GetDefaultHeapProperties();
D3D12_DEPTH_STENCIL_DESC         GetDepthStencilDescriptor( const bool is_depth_test, const bool is_depth_write );
D3D12_RESOURCE_DESC              GetDepthStencilResourceDescriptor( const uint16_t width, const uint16_t height );
D3D12_DEPTH_STENCIL_VIEW_DESC    GetDepthStencilViewDescriptor(const DXGI_FORMAT format);
D3D12_DESCRIPTOR_HEAP_DESC       GetDescriptorHeapDescriptor( const uint32_t count, const bool is_shader, const D3D12_DESCRIPTOR_HEAP_TYPE type );
D3D12_DESCRIPTOR_RANGE           GetDescriptorRange( const D3D12_DESCRIPTOR_RANGE_TYPE type, const uint8_t count, const uint8_t base_register, const uint8_t space );
D3D12_HEAP_DESC                  GetHeapDescriptor( const uint64_t size );
D3D12_PIPELINE_STATE_STREAM_DESC GetPipelineStateStreamDescriptor( const size_t sz, void *stream );
D3D12_RASTERIZER_DESC            GetRasterizerDescriptorDefault( const D3D12_FILL_MODE fill_mode );
D3D12_RESOURCE_BARRIER           GetRenderTargetTransition( const D3D12_RESOURCE_STATES to, ID3D12Resource *target );
D3D12_RESOURCE_BARRIER           GetResourceTransition( const D3D12_RESOURCE_STATES from, const D3D12_RESOURCE_STATES to, ID3D12Resource *resource );
D3D12_RENDER_TARGET_VIEW_DESC    GetRenderTargetViewDescriptor( const DXGI_FORMAT format );
D3D12_ROOT_PARAMETER             GetRootParameter32BitConstantArray( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space, const uint8_t count );
D3D12_ROOT_PARAMETER             GetRootParameterDescriptorConstantBufferView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
D3D12_ROOT_PARAMETER             GetRootParameterDescriptorShaderResourceView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
D3D12_ROOT_PARAMETER             GetRootParameterDescriptorTable( const D3D12_SHADER_VISIBILITY visibility, const D3D12_DESCRIPTOR_RANGE *ranges, const uint8_t count );
D3D12_ROOT_PARAMETER             GetRootParameterDescriptorUnorderedAccessView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
D3D12_ROOT_SIGNATURE_DESC        GetRootSignatureDescriptor( const D3D12_ROOT_SIGNATURE_FLAGS flags, const D3D12_ROOT_PARAMETER *parameters, const uint16_t parameter_count, const D3D12_STATIC_SAMPLER_DESC *samplers, const uint16_t sampler_count );
D3D12_SHADER_RESOURCE_VIEW_DESC  GetShaderResourceViewDescriptorTexture2D( const DXGI_FORMAT format, const uint16_t mip_levels );
DXGI_SWAP_CHAIN_DESC1            GetSwapChainDescriptor( const uint16_t width, const uint16_t height, const DXGI_FORMAT format, const HWND hwnd );
D3D12_RESOURCE_DESC              GetTexture2DResourceDescriptor( const uint16_t width, const uint16_t height, const TextureUsage usage, const DXGI_FORMAT format );
const D3D12_HEAP_PROPERTIES *    GetUploadHeapProperties();


} }/* namespace D3D12::Initializers */