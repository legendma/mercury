#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderShaders.hpp"


namespace RenderEngine
{

typedef struct _Engine
    {
    RenderShaders::ShaderCache
                        shaders;
    RenderModels::ModelCache
                        models;
    ID3D12Device       *device;
    IDXGIFactory4      *dxgi_factory;
    UINT                descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
    HWND                hwnd;
    ID3D12Fence        *fence;
    uint64_t            fence_current;
    ID3D12GraphicsCommandList
                       *gfx;
    ID3D12CommandQueue *command_queue;
    ID3D12CommandAllocator
                       *command_allocator;
    IDXGISwapChain     *swap_chain;
    ID3D12DescriptorHeap
                       *render_target_heap;
    ID3D12DescriptorHeap
                       *depth_stencil_heap;
    ID3D12Resource     *depth_stencil_buffer;
    ID3D12Resource     *backbuffers[ SWAP_CHAIN_DOUBLE_BUFFER ];
    uint8_t             backbuffer_current;
    UINT                window_width;
    UINT                window_height;
    } Engine;

} /* namespace RenderEngine */