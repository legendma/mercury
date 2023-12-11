#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Math.hpp"
#include "NonOwningGroup.hpp"
#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderPassDefault.hpp"
#include "RenderScene.hpp"
#include "RenderShaders.hpp"

#define RENDER_ENGINE_FRAME_COUNT   ( 2 )
#define RENDER_ENGINE_TRASH_CAN_MAX_COUNT \
                                    ( 50 )

namespace RenderEngine
{

typedef struct _DefaultPerPassBuffer
    {
    /* single element */
    ID3D12Resource     *cbuffer;
    RenderPass::DefaultPerPass
                       *mapped;
    } DefaultPerPassBuffer;

typedef struct _DefaultPerObjectBuffer
    {
    uint32_t            max_elements;
    ID3D12Resource     *cbuffer;
    RenderPass::DefaultPerObject
                       *mapped;
    } DefaultPerObjectBuffer;

typedef struct _FrameDefaultPass
    {
    DefaultPerObjectBuffer
                        per_object;
    DefaultPerPassBuffer
                        per_pass;
    } FrameDefaultPass;

typedef struct _Frame
    {
    uint64_t            frame_count;
    uint32_t            trash_can_count;
    IUnknown           *trash_can[ RENDER_ENGINE_TRASH_CAN_MAX_COUNT ];
    ID3D12CommandAllocator
                       *command_allocator;
    FrameDefaultPass    pass_default;
    uint8_t             frame_index;
    } Frame;

typedef struct _Window
    {
    UINT                width;
    UINT                height;
    HWND                handle;
    } Window;

typedef struct _Device
    {
    ID3D12Device       *ptr;
    IDXGIFactory4      *dxgi;
    ID3D12DescriptorHeap
                       *rtv_heap;
    ID3D12DescriptorHeap
                       *dsv_heap;
    ID3D12DescriptorHeap
                       *cbv_srv_heap;
    UINT                descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
    } Device;

typedef struct _Commands
    {
    ID3D12GraphicsCommandList
                       *gfx;
    ID3D12CommandQueue *queue;
    ID3D12CommandAllocator
                       *spare_allocator;
    ID3D12Fence        *fence;
    uint64_t            last_submitted_frame;
    } Commands;

typedef struct _SwapChain
    {
    IDXGISwapChain     *ptr;
    ID3D12Resource     *depth_stencil;
    ID3D12Resource     *backbuffers[ SWAP_CHAIN_DOUBLE_BUFFER ];
    uint8_t             backbuffer_current;
    } SwapChain;

typedef struct _Passes
    {
    RenderPass::Default default_pass;
    } Passes;

typedef struct _Engine
    {
    Window              window;
    Commands            commands;
    Device              device;
    SwapChain           swap_chain;
    Frame               frames[ RENDER_ENGINE_FRAME_COUNT ];
    uint8_t             current_frame;
    Passes              passes;
    ECS::NonOwningGroupIterator
                        group;
    RenderShaders::ShaderCache
                        shaders;
    } Engine;


Frame * Engine_CurrentFrame( Engine *engine );
void    Engine_ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Engine *engine );


} /* namespace RenderEngine */