#pragma once
#include <pthread.h>
#include <stdint.h>

#include "D3D12Types.hpp"


namespace D3D12 { namespace Upload
{

static const int MAX_UPLOADS_PER_FRAME = 100;

typedef struct _Frame
    {
    Core::IMResource   *uploads[ MAX_UPLOADS_PER_FRAME ];
    uint32_t            upload_count;
    Core::IMCommandAllocator
                       *command_allocator;
    Core::IMGraphicsCommandList
                       *command_list;
    pthread_mutex_t     client_mutex;
    uint64_t            fence_value;
    bool                is_used;
    } Frame;


Core::IMResource * AllocateUpload( const uint64_t sz, Frame *frame );
Frame *            BeginFrame();
void               CancelFrame( Frame *frame );
void               EndFrame( Frame *frame );
bool               Init();
void               Shutdown();

}} /* namespace D3D12::Upload */