#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "Math.hpp"
#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderShaders.hpp"

#define RENDER_ENGINE_FRAME_COUNT   ( 2 )
#define RENDER_ENGINE_TRASH_CAN_MAX_COUNT \
                                    ( 50 )

namespace RenderEngine
{

typedef struct _GPUForwardPassData
    {
    Float4x4            mat_view;
    Float4x4            mat_projection;
    Float4x4            mat_view_projection;
    } GPUForwardPassData;

typedef union _MappedGPUPassData
    {
    GPUForwardPassData *forward;
    void               *none;
    } MappedGPUPassData;

typedef struct _GPUPassDataBuffer
    {
    /* single element */
    ID3D12Resource     *elements;
    MappedGPUPassData   mapped;
    } GPUPassDataBuffer;

typedef struct _GPUObjectData
    {
    Float4x4            mat_world;
    } GPUObjectData;

typedef struct GPUObjectDataBuffer
    {
    uint32_t            max_elements;
    ID3D12Resource     *elements;
    GPUObjectData      *mapped;
    } GPUObjectDataBuffer;

typedef struct _Frame
    {
    uint64_t            frame_count;
    GPUObjectDataBuffer per_object_data;
    GPUPassDataBuffer   per_pass_data;
    uint32_t            trash_can_count;
    IUnknown           *trash_can[ RENDER_ENGINE_TRASH_CAN_MAX_COUNT ];
    ID3D12CommandAllocator
                       *command_allocator;
    } Frame;

typedef struct _Engine
    {
    Frame               frames[ RENDER_ENGINE_FRAME_COUNT ];
    uint8_t             current_frame;
    RenderShaders::ShaderCache
                        shaders;
    RenderModels::ModelCache
                        models;
    ID3D12Device       *device;
    IDXGIFactory4      *dxgi_factory;
    UINT                descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
    HWND                hwnd;
    ID3D12GraphicsCommandList
                       *gfx;
    ID3D12CommandQueue *command_queue;
    ID3D12CommandAllocator
                       *spare_command_allocator;
    ID3D12Fence        *fence;
    uint64_t            last_submitted_frame;
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


Frame * Engine_CurrentFrame( Engine *engine );


} /* namespace RenderEngine */