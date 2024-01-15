#pragma once
#include <stdint.h>

#include "AssetFile.hpp"
#include "LinearAllocator.hpp"
#include "ResourceLoader.hpp"

namespace Render { namespace Asset
{
static const int MIP_MAX_COUNT = 5;

typedef struct _Mip
    {
    uint16_t            width;
    uint16_t            height;
    void               *texels;
    } Mip;

typedef struct _Texture
    {
    AssetFileAssetId    id;
    uint8_t             mip_count;
    uint8_t             channel_count;
    Mip                 mips[ MIP_MAX_COUNT ];
    } Texture;


bool ReadTexture( const AssetFileAssetId texture_id, ResourceLoader *loader, LinearAllocator *out_buffer, Texture *out_definition );


}} /* namespace Render::Asset */