#pragma once
#include <cstdint>
#include <d3d12.h>

#include "Math.hpp"

namespace RenderEngine { struct _Engine; }
namespace RenderPass
{
static const uint8_t DEFAULT_PASS_RTV_COUNT = 1;
static const uint8_t DEFAULT_PASS_SRV_COUNT = 1;

typedef struct _DefaultPerPass
    {
    Float4x4            xfm_view;
    Float4x4            xfm_projection;
    Float4x4            xfm_view_projection;
    } DefaultPerPass;

typedef struct _DefaultPerObject
    {
    Float4x4            xfm_world;
    } DefaultPerObject;

typedef struct _Default
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        rtv;
    D3D12_CPU_DESCRIPTOR_HANDLE
                        srv;
    ID3D12PipelineState
                       *pso_opaque;
    ID3D12PipelineState
                       *pso_transparent;

    ID3D12Resource     *rt;
    DXGI_FORMAT         rt_format;
    uint16_t            rt_width;
    uint16_t            rt_height;
    RenderEngine::_Engine
                       *owner;
    } Default;

void Default_Destroy( Default *pass );
bool Default_Init( const D3D12_CPU_DESCRIPTOR_HANDLE rtv, const D3D12_CPU_DESCRIPTOR_HANDLE srv, const DXGI_FORMAT format, RenderEngine::_Engine *engine, Default *pass );
void Default_OnResize( Default *pass );


} /* namespace RenderPass */