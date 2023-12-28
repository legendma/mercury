#pragma once
#include <cstdint>
#include <d3d12.h>
#include <pthread.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define RENDER_UPLOAD_FRAME_COUNT   ( 4 )

namespace Render { namespace Engine { struct _Engine; } }
namespace Render { namespace Buffers
{
struct _Uploader;
typedef struct _Frame
    {
    ID3D12GraphicsCommandList
                       *copy;
    ID3D12CommandAllocator
                       *allocator;
    uint64_t            fence_value;
    ID3D12Resource     *resource;
    _Uploader          *uploader;
    HANDLE              wait_event;
    void               *cpu_buffer;
    } Frame;

typedef struct _Uploader
    {
    ID3D12CommandQueue *queue;
    ID3D12Fence        *fence;
    uint64_t            fence_value;
    Frame               frames[ RENDER_UPLOAD_FRAME_COUNT ];
    pthread_mutex_t     mutex_frame;
    pthread_mutex_t     mutex_queue;
    Engine::_Engine    *owner;
    } Uploader;


void Frame_EndUpload( Frame *frame );

Frame * Uploader_BeginUpload( const size_t aligned_sz, Uploader *uploader );
void    Uploader_Destroy( Uploader* uploader );
bool    Uploader_Initialize( Engine::_Engine *engine, Uploader *uploader );

} }/* namespace Render::Upload */