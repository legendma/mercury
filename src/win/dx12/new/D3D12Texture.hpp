#pragma once
#include <stdint.h>

#include "D3D12Descriptor.hpp"
#include "D3D12Types.hpp"

namespace D3D12 { namespace Texture
{

static const uint8_t MIPS_MAX_COUNT = 4;

typedef struct _Texture
    {
    Core::IMResource   *resource;
    Descriptor::LocalHandle
                        srv;
    } Texture;

typedef struct _TextureCreateParams
    {
    D3D12_RESOURCE_DESC
                       *desc;
    D3D12_SHADER_RESOURCE_VIEW_DESC
                       *srv_desc;
    Core::IMResource   *resource;
    Core::IMHeap       *heap;
    uint64_t            heap_offset;
    D3D12_RESOURCE_STATES
                        initial_state;
    D3D12_CLEAR_VALUE   clear_value;
    uint8_t             mip_count;
    } TextureCreateParams;

typedef struct _RenderBuffer
    {
    Texture             texture;
    Descriptor::LocalHandle
                        rtv[ MIPS_MAX_COUNT ];
    uint8_t             mip_count;
    } RenderBuffer;

typedef struct _DepthBuffer
    {
    Texture             texture;
    Descriptor::LocalHandle
                        dsv[ MIPS_MAX_COUNT ];
    uint8_t             mip_count;
    } DepthBuffer;

    
bool DepthBuffer_Create( const TextureCreateParams *params, DepthBuffer *buffer );
void DepthBuffer_Destroy( DepthBuffer *buffer );
    
bool RenderBuffer_Create( const TextureCreateParams *params, RenderBuffer *buffer );
void RenderBuffer_Destroy( RenderBuffer *buffer );

bool Texture_Create( const TextureCreateParams *params, Texture *texture );
void Texture_Destroy( Texture *texture );

D3D12_RESOURCE_ALLOCATION_INFO GetPlacedTextureInfo( const D3D12_RESOURCE_DESC *desc );
DXGI_FORMAT                    GetSRGBFormat( const DXGI_FORMAT base );


}} /* namespace D3D12::Texture */