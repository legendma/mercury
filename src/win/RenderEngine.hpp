#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#include "ComponentClass.hpp"
#include "Math.hpp"
#include "NonOwningGroup.hpp"
#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderPipelines.hpp"

#define RENDER_ENGINE_FRAME_COUNT   ( 2 )
#define RENDER_ENGINE_TRASH_CAN_MAX_COUNT \
                                    ( 50 )
#define RENDER_ENGINE_SCENE_MAX_CNT ( 10 )

namespace RenderEngine
{
HASH_MAP_IMPLEMENT( DrawScenesMap, RENDER_ENGINE_SCENE_MAX_CNT, ECS::SceneComponent* );

typedef struct _SceneDraw
    {
    DrawScenesMap       map;
    ECS::SceneComponent
                       *draw_order[ RENDER_ENGINE_SCENE_MAX_CNT ];
    } SceneDraw;

typedef struct _DescriptorHandle
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_hndl;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        gpu_hndl;
    } DescriptorHandle;

static const bool DESCRIPTOR_HEAP_IS_LINEAR = false;
static const bool DESCRIPTOR_HEAP_IS_RINGBUFFER = true;
typedef struct _DescriptorHeap
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_start;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        gpu_start;
    uint32_t            descriptor_size;
    uint32_t            capacity;
    D3D12_DESCRIPTOR_HEAP_TYPE
                        type;
    uint8_t             frames_since_wrap;
    bool                is_shader_visible;
    bool                is_ring;
    uint32_t            head;
    ID3D12DescriptorHeap
                       *heap;
    } DescriptorHeap;

typedef struct _Frame
    {
    uint64_t            frame_count;
    uint32_t            trash_can_count;
    IUnknown           *trash_can[ RENDER_ENGINE_TRASH_CAN_MAX_COUNT ];
    ID3D12CommandAllocator
                       *command_allocator;
    uint8_t             frame_index;
    } Frame;

typedef struct _Window
    {
    uint32_t            width;
    uint32_t            height;
    HWND                handle;
    } Window;

typedef struct _Device
    {
    ID3D12Device       *ptr;
    IDXGIFactory4      *dxgi;
    uint32_t            descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
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
    DescriptorHeap      cbv_srv_uav_heap;
    } Commands;

typedef struct _Texture
    {
    ID3D12Resource     *resource;
    D3D12_CPU_DESCRIPTOR_HANDLE
                        handle;
    uint16_t            width;
    uint16_t            height;
    } Texture;

typedef struct _Surfaces
    {
    IDXGISwapChain     *swap_chain;
    Texture             depth_stencil;
    Texture             backbuffers[ SWAP_CHAIN_DOUBLE_BUFFER ];
    uint8_t             backbuffer_current;
    DescriptorHeap      rtv_heap;
    DescriptorHeap      dsv_heap;
    } Surfaces;

typedef struct _Engine
    {
    Window              window;
    Commands            commands;
    Device              device;
    Surfaces            surfaces;
    Frame               frames[ RENDER_ENGINE_FRAME_COUNT ];
    uint8_t             current_frame;
    SceneDraw           scene_draw;
    ECS::NonOwningGroupIterator
                        group;
    RenderPipelines::Pipelines
                        pipelines;
    } Engine;

DescriptorHandle DescriptorHeap_Allocate( DescriptorHeap *heap );
bool             DescriptorHeap_AllocateBatch( const uint32_t allocate_cnt, uint32_t out_capacity, DescriptorHandle *out, DescriptorHeap *heap );
bool             DescriptorHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool is_ring, const uint32_t capacity, ID3D12Device *device, DescriptorHeap *heap );
void             DescriptorHeap_Destroy( Engine *engine, DescriptorHeap *heap );
void             DescriptorHeap_NextFrame( DescriptorHeap *heap );
void             DescriptorHeap_Reset( DescriptorHeap *heap );

void    Engine_ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Engine *engine );
Frame * Engine_CurrentFrame( Engine *engine );
void    Engine_AddTrashToCurrentFrame( IUnknown **trash, Engine *engine );



/*******************************************************************
*
*   Engine_TossTrash()
*
*   DESCRIPTION:
*       Defer destruction of the given COM object until the frame
*       is recycled.
*
*******************************************************************/

#define Engine_TossTrash( _trash, _engine ) \
    Engine_AddTrashToCurrentFrame( (IUnknown**)_trash, _engine )


/*******************************************************************
*
*   DescriptorHandle_IsValid()
*
*   DESCRIPTION:
*       Does the given descriptor handle have a valid address?
*
*******************************************************************/

static uint32_t inline DescriptorHandle_IsValid( const DescriptorHandle *handle )
{
return( handle->cpu_hndl.ptr > 0 );

} /* DescriptorHandle_IsValid() */


/*******************************************************************
*
*   DescriptorHandle_IsShaderVisible()
*
*   DESCRIPTION:
*       Does the given descriptor handle have a GPU memory mapping?
*
*******************************************************************/

static uint32_t inline DescriptorHandle_IsShaderVisible( const DescriptorHandle *handle )
{
return( handle->gpu_hndl.ptr > 0 );

} /* DescriptorHandle_IsShaderVisible() */


/*******************************************************************
*
*   DescriptorHeap_NextFrame()
*
*   DESCRIPTION:
*       Indicate the next frame is underway.
*
*******************************************************************/

static inline void DescriptorHeap_NextFrame( DescriptorHeap *heap )
{
if( heap->is_ring )
    {
    heap->frames_since_wrap++;
    }

} /* DescriptorHeap_NextFrame() */


} /* namespace RenderEngine */