#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>


#define NODE_MASK_SINGLE_GPU        ( 0 )
#define SWAP_CHAIN_DOUBLE_BUFFER    ( 2 )

#define NEAR_DEPTH_VALUE            ( 1.0f )
#define FAR_DEPTH_VALUE             ( 0.0f )

#define RENDER_TARGET_FORMAT        DXGI_FORMAT_R8G8B8A8_UNORM
#define DEPTH_STENCIL_FORMAT        DXGI_FORMAT_D24_UNORM_S8_UINT


namespace RenderInitializers
{

D3D12_RESOURCE_DESC        GetBufferResourceDescriptor( const uint32_t size );
D3D12_HEAP_PROPERTIES      GetDefaultHeapProperties();
D3D12_RESOURCE_DESC        GetDepthStencilResourceDescriptor( const uint16_t width, const uint16_t height );
D3D12_DESCRIPTOR_RANGE     GetDescriptorRange( const D3D12_DESCRIPTOR_RANGE_TYPE type, const uint8_t count, const uint8_t base_register, const uint8_t space );
D3D12_DESCRIPTOR_HEAP_DESC GetHeapDescriptor( const uint16_t count, const bool is_shader, const D3D12_DESCRIPTOR_HEAP_TYPE type );
D3D12_RESOURCE_BARRIER     GetRenderTargetTransition( const D3D12_RESOURCE_STATES to, ID3D12Resource *target );
D3D12_RESOURCE_BARRIER     GetResourceTransition( const D3D12_RESOURCE_STATES from, const D3D12_RESOURCE_STATES to, ID3D12Resource *resource );
D3D12_ROOT_PARAMETER       GetRootParameter32BitConstantArray( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space, const uint8_t count );
D3D12_ROOT_PARAMETER       GetRootParameterConstantBufferView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
D3D12_ROOT_PARAMETER       GetRootParameterDescriptorTable( const D3D12_SHADER_VISIBILITY visibility, const D3D12_DESCRIPTOR_RANGE *ranges, const uint8_t count );
D3D12_ROOT_PARAMETER       GetRootParameterShaderResourceView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
D3D12_ROOT_PARAMETER       GetRootParameterUnorderedAccessView( const D3D12_SHADER_VISIBILITY visibility, const uint8_t shader_register, const uint8_t space );
DXGI_SWAP_CHAIN_DESC       GetSwapChainDescriptor( const uint16_t width, const uint16_t height, HWND hwnd );
D3D12_ROOT_SIGNATURE_DESC  GetRootSignatureDescriptor( const D3D12_ROOT_SIGNATURE_FLAGS flags, const D3D12_ROOT_PARAMETER *parameters, const uint16_t parameter_count, const D3D12_STATIC_SAMPLER_DESC *samplers, const uint16_t sampler_count );
D3D12_HEAP_PROPERTIES      GetUploadHeapProperties();

} /* namespace RenderInitializers */