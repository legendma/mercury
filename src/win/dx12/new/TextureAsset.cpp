#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdint.h>

#include "AssetFile.hpp"
#include "LinearAllocator.hpp"
#include "ResourceLoader.hpp"
#include "TextureAsset.hpp"
#include "Utilities.hpp"


namespace Render { namespace Asset
{


/*******************************************************************
*
*   ReadTexture()
*
*   DESCRIPTION:
*       Read a texture from disk using the given allocator.  Fill
*       out the texture definition describing the texture contained
*       within the allocator.
*
*       The given loader and allocator are expected to be valid and
*       initialized.
*
*******************************************************************/

bool ReadTexture( const AssetFileAssetId texture_id, ResourceLoader *loader, LinearAllocator *out_buffer, Texture *out_definition )
{
*out_definition = {};

uint32_t file_size = 0;
if( !ResourceLoader_GetTexture( texture_id, &file_size, NULL, loader ) )
    {
    debug_assert_always();
    return( false );
    }

// TODO <MPA> - Implement mipmap loading
const uint8_t mip_count = 1;

LinearAllocatorResetToken reset = LinearAllocator_GetResetToken( out_buffer );
uint8_t *file = (uint8_t*)LinearAllocator_Allocate( file_size, out_buffer );
if( !file )
    {
    debug_assert_always();
    return( false );
    }

int width, height, channel_count;
stbi_uc *raw_image = stbi_load_from_memory( (stbi_uc*)file, (int)file_size, &width, &height, &channel_count, 0 );
LinearAllocator_ResetByToken( reset, out_buffer );
if( !raw_image )
    {
    debug_assert_always();
    return( false );
    }

uint32_t texels_size = (uint32_t)( width * height * channel_count);
uint8_t *texels = (uint8_t*)LinearAllocator_Allocate( texels_size, out_buffer );
if( !texels)
    {
    debug_assert_always();
    stbi_image_free( raw_image );
    return( false );
    }

memcpy( texels, raw_image, texels_size );
stbi_image_free( raw_image );

out_definition->id = texture_id;
out_definition->mip_count     = 1;
out_definition->channel_count = (uint8_t)channel_count;
out_definition->mips[ 0 ].width  = (uint16_t)width;
out_definition->mips[ 0 ].height = (uint16_t)height;
out_definition->mips[ 0 ].texels = texels;

return( true );

} /* ReadTexture() */


} }/* namespace Render::Asset */