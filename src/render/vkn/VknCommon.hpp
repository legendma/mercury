#pragma once

#include <climits>
#include <cstdio>

#if defined( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "vulkan.h"

#include "Global.hpp"
#include "Utilities.hpp"


#define VKN_MIN_VERSION             VK_API_VERSION_1_3
#define VKN_FRAME_CNT               ( 2 )
#define VKN_INVALID_FAMILY_INDEX    max_uint_value( u32 )
#define VKN_WAIT_INFINITE           max_uint_value( u64 )
#define VKN_DESCRIPTOR_SET_CNT      ( 4 )
#define VKN_HASH_SEED               ( 0x811c9dc5 )

typedef enum
    {
    VKN_MSAA_OFF,                   /* no multi-sampling            */
    VKN_MSAA_4,                     /* MSAA x4                      */
    VKN_MSAA_8,                     /* MSAA x8                      */
    VKN_MSAA_16                     /* MSAA x16                     */
    } VKN_msaa_type;

typedef enum
    {
    VKN_SHADER_GFX_STAGE_VERTEX,
    VKN_SHADER_GFX_STAGE_TESSELLATION_CONTROL,
    VKN_SHADER_GFX_STAGE_TESSELLATION_EVALUATION,
    VKN_SHADER_GFX_STAGE_GEOMETRY,
    VKN_SHADER_GFX_STAGE_FRAGMENT,
    /* count */
    VKN_SHADER_GFX_STAGE_CNT
    } VKN_shader_gfx_stage_type;

/*------------------------------------------------------------------
Render state flags
------------------------------------------------------------------*/
/*----------------------------------------------------------
Fill mode
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_FILL_MODE_SHIFT                            ( 0ull )
#define VKN_RENDER_FLAG_BITS_FILL_MODE_LENGTH                           ( 1ull )
#define VKN_RENDER_FLAG_BITS_FILL_MODE_MASK                             shift_bits64( VKN_RENDER_FLAG_BITS_FILL_MODE_LENGTH, VKN_RENDER_FLAG_BITS_FILL_MODE_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_FILL_MODE_SOLID                            ( 0ull << VKN_RENDER_FLAG_BITS_FILL_MODE_SHIFT )
#define VKN_RENDER_FLAG_BITS_FILL_MODE_WIREFRAME                        ( 1ull << VKN_RENDER_FLAG_BITS_FILL_MODE_SHIFT )

/*----------------------------------------------------------
Topology
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT                             ( VKN_RENDER_FLAG_BITS_FILL_MODE_SHIFT + VKN_RENDER_FLAG_BITS_FILL_MODE_LENGTH )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_LENGTH                            ( 3ull )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_MASK                              shift_bits64( VKN_RENDER_FLAG_BITS_TOPOLOGY_LENGTH, VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLES                         ( 0ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_POINTS                            ( 1ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_LINES                             ( 2ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_LINE_STRIP                        ( 3ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_STRIP                    ( 4ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )
#define VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_FAN                      ( 5ull << VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT )

/*----------------------------------------------------------
Cull mode
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT                            ( VKN_RENDER_FLAG_BITS_TOPOLOGY_SHIFT + VKN_RENDER_FLAG_BITS_TOPOLOGY_LENGTH )
#define VKN_RENDER_FLAG_BITS_CULL_MODE_LENGTH                           ( 2ull )
#define VKN_RENDER_FLAG_BITS_CULL_MODE_MASK                             shift_bits64( VKN_RENDER_FLAG_BITS_CULL_MODE_LENGTH, VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_CULL_MODE_NONE                             ( 0ull << VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT )
#define VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT                            ( 1ull << VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT )
#define VKN_RENDER_FLAG_BITS_CULL_MODE_BACK                             ( 2ull << VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT )
#define VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT_AND_BACK                   ( 3ull << VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT )

/*----------------------------------------------------------
Front face winding
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT                      ( VKN_RENDER_FLAG_BITS_CULL_MODE_SHIFT + VKN_RENDER_FLAG_BITS_CULL_MODE_LENGTH )
#define VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_LENGTH                     ( 1ull )
#define VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_MASK                       shift_bits64( VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_LENGTH, VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CCW                        ( 0ull << VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT )
#define VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CW                         ( 1ull << VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT )

/*----------------------------------------------------------
Blend operation (color and alpha)
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT                             ( VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT + VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_LENGTH )
#define VKN_RENDER_FLAG_BITS_BLEND_OP_LENGTH                            ( 2ull )
#define VKN_RENDER_FLAG_BITS_BLEND_OP_MASK                              shift_bits64( VKN_RENDER_FLAG_BITS_BLEND_OP_LENGTH, VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_BLEND_OP_ADD                               ( 0ull << VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT )
#define VKN_RENDER_FLAG_BITS_BLEND_OP_SUBTRACT                          ( 1ull << VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT )
#define VKN_RENDER_FLAG_BITS_BLEND_OP_MIN                               ( 2ull << VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT )
#define VKN_RENDER_FLAG_BITS_BLEND_OP_MAX                               ( 3ull << VKN_RENDER_FLAG_BITS_BLEND_OP_SHIFT )

/*----------------------------------------------------------
Blend factor source color
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT               ( VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_SHIFT + VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_LENGTH )
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_LENGTH              ( 2ull )
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_MASK                shift_bits64( VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_LENGTH, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE                 ( 0ull << VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ZERO                ( 1ull << VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SRC_ALPHA           ( 2ull << VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ( 3ull << VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT )

/*----------------------------------------------------------
Blend factor source alpha
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT               ( VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SHIFT + VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_LENGTH )
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_LENGTH              ( 2ull )
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_MASK                shift_bits64( VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_LENGTH, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE                 ( 0ull << VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ZERO                ( 1ull << VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SRC_ALPHA           ( 2ull << VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ( 3ull << VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT )

/*----------------------------------------------------------
Blend factor destination color
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT               ( VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SHIFT + VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_LENGTH )
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_LENGTH              ( 2ull )
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_MASK                shift_bits64( VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_LENGTH, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ZERO                ( 0ull << VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE                 ( 1ull << VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SRC_ALPHA           ( 2ull << VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ( 3ull << VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT )

/*----------------------------------------------------------
Blend factor destination alpha
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT               ( VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SHIFT + VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_LENGTH )
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_LENGTH              ( 2ull )
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_MASK                shift_bits64( VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_LENGTH, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ZERO                ( 0ull << VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE                 ( 1ull << VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SRC_ALPHA           ( 2ull << VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT )
#define VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ( 3ull << VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT )

/*----------------------------------------------------------
Color write enable - red
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_SHIFT               ( VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SHIFT + VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_LENGTH )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_LENGTH              ( 1ull )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_MASK                shift_bits64( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_LENGTH, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_ENABLE              ( 0ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_SHIFT )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_DISABLE             ( 1ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_SHIFT )

/*----------------------------------------------------------
Color write enable - green
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_SHIFT             ( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_SHIFT + VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_LENGTH )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_LENGTH            ( 1ull )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_MASK              shift_bits64( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_LENGTH, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_ENABLE            ( 0ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_SHIFT )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_DISABLE           ( 1ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_SHIFT )

/*----------------------------------------------------------
Color write enable - blue
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_SHIFT              ( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_SHIFT + VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_LENGTH )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_LENGTH             ( 1ull )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_MASK               shift_bits64( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_LENGTH, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_ENABLE             ( 0ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_SHIFT )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_DISABLE            ( 1ull << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_SHIFT )

/*----------------------------------------------------------
Color write enable - alpha
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_SHIFT             ( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_SHIFT + VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_LENGTH )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_LENGTH            ( 1ull )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_MASK              shift_bits64( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_LENGTH, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_ENABLE            ( 0 << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_SHIFT )
#define VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_DISABLE           ( 1 << VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_SHIFT )

/*----------------------------------------------------------
Depth test pass
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT                      ( VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_SHIFT + VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_LENGTH )
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LENGTH                     ( 2ull )
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK                       shift_bits64( VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LENGTH, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_ALWAYS                     ( 0ull << VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_EQUAL                      ( 1ull << VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_GREATER_EQUAL              ( 2ull << VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LESS_EQUAL                 ( 3ull << VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT )

/*----------------------------------------------------------
Depth write enable
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_SHIFT                   ( VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_SHIFT + VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LENGTH )
#define VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_LENGTH                  ( 1ull )
#define VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_MASK                    shift_bits64( VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_LENGTH, VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_ENABLE                  ( 0ull << VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_SHIFT )
#define VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_DISABLE                 ( 1ull << VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_SHIFT )

/*----------------------------------------------------------
Stencil test pass
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT                    ( VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_SHIFT + VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LENGTH                   ( 3ull )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_MASK                     shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_ALWAYS                   ( 0ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NEVER                    ( 1ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_EQUAL                    ( 2ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NOT_EQUAL                ( 3ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER                  ( 4ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER_EQUAL            ( 5ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS                     ( 6ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS_EQUAL               ( 7ull << VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT )

/*----------------------------------------------------------
Stencil test reference value
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_SHIFT                     ( VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_LENGTH                    bit_width64( max_uint_value( u8 ) )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_MASK                      shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_SHIFT )

/*----------------------------------------------------------
Stencil test mask value
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_SHIFT                    ( VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_LENGTH                   bit_width64( max_uint_value( u8 ) )
#define VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_MASK                     shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_SHIFT )

/*----------------------------------------------------------
Stencil write stencil fail
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT                  ( VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_LENGTH                 ( 3ull )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_MASK                   shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_KEEP                   ( 0ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_ZERO                   ( 1ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_REPLACE                ( 2ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR                   ( 3ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR_WRAP              ( 4ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR                   ( 5ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR_WRAP              ( 6ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INVERT                 ( 7ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT )

/*----------------------------------------------------------
Stencil write depth fail
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT                  ( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_LENGTH                 ( 3ull )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_MASK                   shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_KEEP                   ( 0ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_ZERO                   ( 1ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_REPLACE                ( 2ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR                   ( 3ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR_WRAP              ( 4ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR                   ( 5ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR_WRAP              ( 6ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INVERT                 ( 7ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT )

/*----------------------------------------------------------
Stencil write pass
----------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT                   ( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_LENGTH )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_LENGTH                  ( 3ull )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_MASK                    shift_bits64( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_LENGTH, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
/* values */
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_KEEP                    ( 0ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_ZERO                    ( 1ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_REPLACE                 ( 2ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR                    ( 3ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR_WRAP               ( 4ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR                    ( 5ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR_WRAP               ( 6ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )
#define VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INVERT                  ( 7ull << VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT )

/*------------------------------------------------------
Final bit count
------------------------------------------------------*/
#define VKN_RENDER_FLAG_BITS_BIT_COUNT                                  ( VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_SHIFT + VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_LENGTH )
    
typedef u64 VKN_render_flags_type;
compiler_assert( CHAR_BIT * sizeof( VKN_render_flags_type ) >= VKN_RENDER_FLAG_BITS_BIT_COUNT, VKN_PUB_COMMON_H );

typedef struct
    {
    VkBaseOutStructure *head;
    VkPhysicalDeviceFeatures2
                        v1_0;
    VkPhysicalDeviceVulkan11Features
                        v1_1;
    VkPhysicalDeviceVulkan12Features
                        v1_2;
    VkPhysicalDeviceVulkan13Features
                        v1_3;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
                        extended_dynamic_state;
    } VKN_features_type;

compiler_assert( VKN_SHADER_GFX_STAGE_VERTEX                  == 0, VKN_PUB_COMMON );
compiler_assert( VKN_SHADER_GFX_STAGE_TESSELLATION_CONTROL    == 1, VKN_PUB_COMMON );
compiler_assert( VKN_SHADER_GFX_STAGE_TESSELLATION_EVALUATION == 2, VKN_PUB_COMMON );
compiler_assert( VKN_SHADER_GFX_STAGE_GEOMETRY                == 3, VKN_PUB_COMMON );
compiler_assert( VKN_SHADER_GFX_STAGE_FRAGMENT                == 4, VKN_PUB_COMMON );
static const VkShaderStageFlags VKN_SHADER_GFX_STAGE_TO_STAGE_FLAGS[] =
    {
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT
    };
compiler_assert( cnt_of_array( VKN_SHADER_GFX_STAGE_TO_STAGE_FLAGS ) == VKN_SHADER_GFX_STAGE_CNT, VKN_PUB_COMMON );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_failed
*
*   DESCRIPTION:
*       Determine if a Vulkan function returned a failure result.
*
*********************************************************************/

#define VKN_failed( _result ) \
    ( (_result) != VK_SUCCESS )


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_goto_fail
*
*   DESCRIPTION:
*       Jump to label if the operation failed.
*
*********************************************************************/

#define VKN_goto_fail( _op, _label ) \
    do                               \
        {                            \
        if( VKN_failed( _op ) )      \
            {                        \
            debug_assert_always(); \
            goto _label;             \
            }                        \
        } while( 0 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_goto_bfail
*
*   DESCRIPTION:
*       Jump to label if the boolean is FALSE.
*
*********************************************************************/

#define VKN_goto_bfail( _bool, _label ) \
    do                                  \
        {                               \
        if( !(_bool) )                  \
            {                           \
            debug_assert_always();    \
            goto _label;                \
            }                           \
        } while( 0 )


/*****************************************************************************
*
*   PROCEDURE NAME:
*       VKN_hash_blob
*
*   DESCRIPTION:
*       Hash the given data.
*
*****************************************************************************/

static __inline u32 VKN_hash_blob
    (
    const u32           seed,       /* hash seed                        */
    const void         *data,       /* data to hash                     */
    const u32           length      /* length of data                   */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define PRIME                       ( 0x1000193 )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
const u8               *d;          /* data pointer                     */
const u8               *end;        /* past end of data buffer          */
u32                     ret;        /* returned hash                    */

/*----------------------------------------------------------
Calculate the hash from the input data
----------------------------------------------------------*/
d   = (u8*)data;
end = d + length;
ret = seed;
while( d < end )
    {
    ret ^= (u32)*d++;
    ret *= PRIME;
    }

return( ret );

#undef PRIME
}   /* VKN_hash_blob() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_make_clear_color_s
*
*   DESCRIPTION:
*       Make a clear color from float.
*
*********************************************************************/

static __inline VkClearColorValue VKN_make_clear_color_s
    (
    const float         red,        /* red channel                  */
    const float         green,      /* green channel                */
    const float         blue,       /* blue channel                 */
    const float         alpha       /* alpha channel                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkClearColorValue       ret;        /* return color                 */

ret.float32[ 0 ] = red;
ret.float32[ 1 ] = green;
ret.float32[ 2 ] = blue;
ret.float32[ 3 ] = alpha;

return( ret );

}   /* VKN_make_clear_color_s() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_make_clear_color_u32
*
*   DESCRIPTION:
*       Make a clear color from uint32.
*
*********************************************************************/

static __inline VkClearColorValue VKN_make_clear_color_u32
    (
    const u32           red,        /* red channel                  */
    const u32           green,      /* green channel                */
    const u32           blue,       /* blue channel                 */
    const u32           alpha       /* alpha channel                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkClearColorValue       ret;        /* return color                 */

ret.uint32[ 0 ] = red;
ret.uint32[ 1 ] = green;
ret.uint32[ 2 ] = blue;
ret.uint32[ 3 ] = alpha;

return( ret );

}   /* VKN_make_clear_color_u32() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_make_clear_depth_stencil
*
*   DESCRIPTION:
*       Make a clear depth/stencil value.
*
*********************************************************************/

static __inline VkClearDepthStencilValue VKN_make_clear_depth_stencil
    (
    const float         depth,      /* depth value                  */
    const u32           stencil     /* stencil value                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkClearDepthStencilValue
                        ret;        /* return values                */

ret.depth   = depth;
ret.stencil = stencil;

return( ret );

}   /* VKN_make_clear_depth_stencil() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_name_object
*
*   DESCRIPTION:
*       Give an object a name.
*
*********************************************************************/

static __inline VkResult VKN_name_object
    (
    const VkDevice      logical,    /* owning device                */
    const void         *handle,     /* object's handle              */
    const VkObjectType  kind,       /* type of object               */
    const char         *format,     /* string formatting            */
    ...
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
va_list                 args;       /* variable args                */
VkDebugUtilsObjectNameInfoEXT
                        info;       /* naming info                  */
char                    str[ 200 ]; /* name string buffer           */

if( handle == NULL )
    {
    return( VK_SUCCESS );
    }

va_start( args, format );
vsprintf_s( str, cnt_of_array( str ), format, args );
va_end( args );

clr_struct( &info );
info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
info.objectHandle = (uint64_t)handle;
info.objectType   = kind;
info.pObjectName  = str;

return( vkSetDebugUtilsObjectNameEXT( logical, &info ) );

}   /* VKN_name_object() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_buffer
*
*   DESCRIPTION:
*       Safely release the given memory buffer.
*
*********************************************************************/

static __inline void VKN_release_buffer
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkBuffer           *buffer      /* memory buffer to release     */
    )
{
if( *buffer == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyBuffer( logical, *buffer, allocator );
*buffer = VK_NULL_HANDLE;

}   /* VKN_release_buffer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_command_buffer
*
*   DESCRIPTION:
*       Safely release the given command buffer.
*
*********************************************************************/

static __inline void VKN_release_command_buffer
    (
    const VkDevice      logical,    /* logical device               */
    const VkCommandPool pool,       /* command pool                 */
    VkCommandBuffer    *buffer      /* command buffer to release    */
    )
{
if( *buffer == VK_NULL_HANDLE )
    {
    return;
    }

vkFreeCommandBuffers( logical, pool, 1, buffer );
*buffer = VK_NULL_HANDLE;

}   /* VKN_release_command_buffer() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_command_pool
*
*   DESCRIPTION:
*       Safely release the given command pool.
*
*********************************************************************/

static __inline void VKN_release_command_pool
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkCommandPool      *pool        /* command pool to release      */
    )
{
if( *pool == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyCommandPool( logical, *pool, allocator );
*pool = VK_NULL_HANDLE;

}   /* VKN_release_command_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_debug_messenger
*
*   DESCRIPTION:
*       Safely release the given debug messenger.
*
*********************************************************************/

static __inline void VKN_release_debug_messenger
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkDebugUtilsMessengerEXT
                       *messenger   /* messenger to release         */
    )
{
if( *messenger == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyDebugUtilsMessengerEXT( instance, *messenger, allocator );
*messenger = VK_NULL_HANDLE;

}   /* VKN_release_debug_messenger() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_descriptor_pool
*
*   DESCRIPTION:
*       Safely release the given descriptor pool.
*
*********************************************************************/

static __inline void VKN_release_descriptor_pool
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkDescriptorPool   *pool        /* descriptor pool to release   */
    )
{
if( *pool == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyDescriptorPool( logical, *pool, allocator );
*pool = VK_NULL_HANDLE;

}   /* VKN_release_descriptor_pool() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_descriptor_set_layout
*
*   DESCRIPTION:
*       Safely release the given descriptor set layout.
*
*********************************************************************/

static __inline void VKN_release_descriptor_set_layout
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkDescriptorSetLayout
                       *layout      /* layout to release            */
    )
{
if( *layout == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyDescriptorSetLayout( logical, *layout, allocator );
*layout = VK_NULL_HANDLE;

}   /* VKN_release_descriptor_set_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_device
*
*   DESCRIPTION:
*       Safely release the given device.
*
*********************************************************************/

static __inline void VKN_release_device
    (
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkDevice           *device      /* device to release            */
    )
{
if( *device == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyDevice( *device, allocator );
*device = VK_NULL_HANDLE;

}   /* VKN_release_device() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_fence
*
*   DESCRIPTION:
*       Safely release the given fence.
*
*********************************************************************/

static __inline void VKN_release_fence
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkFence            *fence       /* fence to release             */
    )
{
if( *fence == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyFence( logical, *fence, allocator );
*fence = VK_NULL_HANDLE;

}   /* VKN_release_fence() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_image
*
*   DESCRIPTION:
*       Safely release the given image.
*
*********************************************************************/

static __inline void VKN_release_image
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkImage            *image       /* image to release             */
    )
{
if( *image == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyImage( device, *image, allocator );
*image = VK_NULL_HANDLE;

}   /* VKN_release_image() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_image_view
*
*   DESCRIPTION:
*       Safely release the given image view.
*
*********************************************************************/

static __inline void VKN_release_image_view
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkImageView        *view        /* image view to release        */
    )
{
if( *view == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyImageView( device, *view, allocator );
*view = VK_NULL_HANDLE;

}   /* VKN_release_image_view() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_instance
*
*   DESCRIPTION:
*       Safely release the given vulkan instance.
*
*********************************************************************/

static __inline void VKN_release_instance
    (
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkInstance         *instance    /* instance to release          */
    )
{
if( *instance == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyInstance( *instance, allocator );
*instance = VK_NULL_HANDLE;

}   /* VKN_release_instance() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_memory
*
*   DESCRIPTION:
*       Safely release the given device memory.
*
*********************************************************************/

static __inline void VKN_release_memory
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkDeviceMemory     *memory      /* memory to release            */
    )
{
if( *memory == VK_NULL_HANDLE )
    {
    return;
    }

vkFreeMemory( logical, *memory, allocator );
*memory = VK_NULL_HANDLE;

}   /* VKN_release_memory() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_pipeline
*
*   DESCRIPTION:
*       Safely release the given pipeline.
*
*********************************************************************/

static __inline void VKN_release_pipeline
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkPipeline         *pipeline    /* pipeline to release          */
    )
{
if( *pipeline == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyPipeline( logical, *pipeline, allocator );
*pipeline = VK_NULL_HANDLE;

}   /* VKN_release_pipeline() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_pipeline_layout
*
*   DESCRIPTION:
*       Safely release the given pipeline layout.
*
*********************************************************************/

static __inline void VKN_release_pipeline_layout
    (
    const VkDevice      logical,    /* logical device               */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkPipelineLayout   *layout      /* layout to release            */
    )
{
if( *layout == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyPipelineLayout( logical, *layout, allocator );
*layout = VK_NULL_HANDLE;

}   /* VKN_release_pipeline_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_sampler
*
*   DESCRIPTION:
*       Safely release the given sampler.
*
*********************************************************************/

static __inline void VKN_release_sampler
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSampler          *sampler     /* sampler to release         */
    )
{
if( *sampler == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroySampler( device, *sampler, allocator );
*sampler = VK_NULL_HANDLE;

}   /* VKN_release_sampler() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_semaphore
*
*   DESCRIPTION:
*       Safely release the given semaphore.
*
*********************************************************************/

static __inline void VKN_release_semaphore
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSemaphore        *semaphore   /* semaphore to release         */
    )
{
if( *semaphore == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroySemaphore( device, *semaphore, allocator );
*semaphore = VK_NULL_HANDLE;

}   /* VKN_release_semaphore() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_shader_module
*
*   DESCRIPTION:
*       Safely release the given shader module.
*
*********************************************************************/

static __inline void VKN_release_shader_module
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkShaderModule     *shader      /* shader to release            */
    )
{
if( *shader == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroyShaderModule( device, *shader, allocator );
*shader = VK_NULL_HANDLE;

}   /* VKN_release_shader_module() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_surface
*
*   DESCRIPTION:
*       Safely release the given surface.
*
*********************************************************************/

static __inline void VKN_release_surface
    (
    const VkInstance    instance,   /* vulkan instance              */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSurfaceKHR       *surface     /* surface to release           */
    )
{
if( *surface == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroySurfaceKHR( instance, *surface, allocator );
*surface = VK_NULL_HANDLE;

}   /* VKN_release_surface() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_release_swap_chain
*
*   DESCRIPTION:
*       Safely release the given swap chain.
*
*********************************************************************/

static __inline void VKN_release_swap_chain
    (
    const VkDevice      device,     /* owning device                */
    const VkAllocationCallbacks
                       *allocator,  /* allocation callbacks         */
    VkSwapchainKHR     *swap_chain  /* swap chain to release        */
    )
{
if( *swap_chain == VK_NULL_HANDLE )
    {
    return;
    }

vkDestroySwapchainKHR( device, *swap_chain, allocator );
*swap_chain = VK_NULL_HANDLE;

}   /* VKN_release_swap_chain() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_blend_factor_destination_alpha
*
*********************************************************************/

static __inline VkBlendFactor VKN_render_flags_get_blend_factor_destination_alpha
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBlendFactor           ret;        /* return blending factor       */

switch( flags & VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_MASK )
    {
    case VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE:
        ret = VK_BLEND_FACTOR_ONE;
        break;

    case VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_SRC_ALPHA;
        break;

    case VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_MASK ) == VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ZERO );
        ret = VK_BLEND_FACTOR_ZERO;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_blend_factor_destination_alpha() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_blend_factor_destination_color
*
*********************************************************************/

static __inline VkBlendFactor VKN_render_flags_get_blend_factor_destination_color
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBlendFactor           ret;        /* return blending factor       */

switch( flags & VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_MASK )
    {
    case VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE:
        ret = VK_BLEND_FACTOR_ONE;
        break;

    case VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_SRC_ALPHA;
        break;

    case VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_MASK ) == VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ZERO );
        ret = VK_BLEND_FACTOR_ZERO;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_blend_factor_destination_color() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_blend_factor_source_alpha
*
*********************************************************************/

static __inline VkBlendFactor VKN_render_flags_get_blend_factor_source_alpha
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBlendFactor           ret;        /* return blending factor       */

switch( flags & VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_MASK )
    {
    case VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ZERO:
        ret = VK_BLEND_FACTOR_ZERO;
        break;

    case VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_SRC_ALPHA;
        break;

    case VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_MASK ) == VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE );
        ret = VK_BLEND_FACTOR_ONE;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_blend_factor_source_alpha() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_blend_factor_source_color
*
*********************************************************************/

static __inline VkBlendFactor VKN_render_flags_get_blend_factor_source_color
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBlendFactor           ret;        /* return blending factor       */

switch( flags & VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_MASK )
    {
    case VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ZERO:
        ret = VK_BLEND_FACTOR_ZERO;
        break;

    case VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_SRC_ALPHA;
        break;

    case VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        ret = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_MASK ) == VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE );
        ret = VK_BLEND_FACTOR_ONE;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_blend_factor_source_color() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_blend_op
*
*********************************************************************/

static __inline VkBlendOp VKN_render_flags_get_blend_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkBlendOp               ret;        /* return blending operation    */

switch( flags & VKN_RENDER_FLAG_BITS_BLEND_OP_MASK )
    {
    case VKN_RENDER_FLAG_BITS_BLEND_OP_SUBTRACT:
        ret = VK_BLEND_OP_SUBTRACT;
        break;

    case VKN_RENDER_FLAG_BITS_BLEND_OP_MIN:
        ret = VK_BLEND_OP_MIN;
        break;

    case VKN_RENDER_FLAG_BITS_BLEND_OP_MAX:
        ret = VK_BLEND_OP_MAX;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_BLEND_OP_MASK ) == VKN_RENDER_FLAG_BITS_BLEND_OP_ADD );
        ret = VK_BLEND_OP_ADD;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_blend_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_color_mask
*
*********************************************************************/

static __inline VkColorComponentFlags VKN_render_flags_get_color_mask
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkColorComponentFlags   ret;        /* return color channel mask    */

ret = 0;
set_clear_bits( ret, VK_COLOR_COMPONENT_R_BIT, ( flags & VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_MASK   ) == VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_ENABLE );
set_clear_bits( ret, VK_COLOR_COMPONENT_G_BIT, ( flags & VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_MASK ) == VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_ENABLE );
set_clear_bits( ret, VK_COLOR_COMPONENT_B_BIT, ( flags & VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_MASK  ) == VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_ENABLE );
set_clear_bits( ret, VK_COLOR_COMPONENT_A_BIT, ( flags & VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_MASK ) == VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_ENABLE );

return( ret );

}   /* VKN_render_flags_get_color_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_cull_mode
*
*********************************************************************/

static __inline VkCullModeFlags VKN_render_flags_get_cull_mode
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCullModeFlags         ret;        /* return culling mode          */

switch( flags & VKN_RENDER_FLAG_BITS_CULL_MODE_MASK )
    {
    case VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT:
        ret = VK_CULL_MODE_FRONT_BIT;
        break;

    case VKN_RENDER_FLAG_BITS_CULL_MODE_BACK:
        ret = VK_CULL_MODE_BACK_BIT;
        break;

    case VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT_AND_BACK:
        ret = VK_CULL_MODE_FRONT_AND_BACK;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_CULL_MODE_MASK ) == VKN_RENDER_FLAG_BITS_CULL_MODE_NONE );
        ret = VK_CULL_MODE_NONE;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_cull_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_depth_test_compare_op
*
*********************************************************************/

static __inline VkCompareOp VKN_render_flags_get_depth_test_compare_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCompareOp             ret;        /* return compare operation     */

switch( flags & VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK )
    {
    case VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_EQUAL:
        ret = VK_COMPARE_OP_EQUAL;
        break;

    case VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_GREATER_EQUAL:
        ret = VK_COMPARE_OP_GREATER_OR_EQUAL;
        break;

    case VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LESS_EQUAL:
        ret = VK_COMPARE_OP_LESS_OR_EQUAL;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK ) == VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_ALWAYS );
        ret = VK_COMPARE_OP_ALWAYS;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_depth_test_compare_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_depth_write_enabled
*
*********************************************************************/

static __inline bool VKN_render_flags_get_depth_write_enabled
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
return( ( flags & VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_MASK ) == VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_ENABLE );

}   /* VKN_render_flags_get_depth_write_enabled() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_fill_mode
*
*********************************************************************/

static __inline VkPolygonMode VKN_render_flags_get_fill_mode
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPolygonMode           ret;        /* return polygon fill mode     */

switch( flags & VKN_RENDER_FLAG_BITS_FILL_MODE_MASK )
    {
    case VKN_RENDER_FLAG_BITS_FILL_MODE_WIREFRAME:
        ret = VK_POLYGON_MODE_LINE;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_FILL_MODE_MASK ) == VKN_RENDER_FLAG_BITS_FILL_MODE_SOLID );
        ret = VK_POLYGON_MODE_FILL;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_fill_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_front_face_winding
*
*********************************************************************/

static __inline VkFrontFace VKN_render_flags_get_front_face_winding
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkFrontFace             ret;        /* return front face winding    */

switch( flags & VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_MASK )
    {
    case VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CW:
        ret = VK_FRONT_FACE_CLOCKWISE;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_MASK ) == VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CCW );
        ret = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_front_face_winding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_test_compare_mask
*
*********************************************************************/

static __inline u8 VKN_render_flags_get_stencil_test_compare_mask
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u8                      ret;        /* return compare mask          */

ret = (u8)( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_MASK ) >> VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_SHIFT );

return( ret );

}   /* VKN_render_flags_get_stencil_test_compare_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_test_compare_op
*
*********************************************************************/

static __inline VkCompareOp VKN_render_flags_get_stencil_test_compare_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkCompareOp             ret;        /* return compare operation     */

switch( flags & VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_MASK )
    {
    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NEVER:
        ret = VK_COMPARE_OP_NEVER;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_EQUAL:
        ret = VK_COMPARE_OP_EQUAL;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NOT_EQUAL:
        ret = VK_COMPARE_OP_NOT_EQUAL;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER:
        ret = VK_COMPARE_OP_GREATER;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER_EQUAL:
        ret = VK_COMPARE_OP_GREATER_OR_EQUAL;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS:
        ret = VK_COMPARE_OP_LESS;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS_EQUAL:
        ret = VK_COMPARE_OP_LESS_OR_EQUAL;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_MASK ) == VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_ALWAYS );
        ret = VK_COMPARE_OP_ALWAYS;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_stencil_test_compare_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_test_reference
*
*********************************************************************/

static __inline u8 VKN_render_flags_get_stencil_test_reference
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u8                      ret;        /* return compare reference     */

ret = (u8)( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_MASK ) >> VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_SHIFT );

return( ret );

}   /* VKN_render_flags_get_stencil_test_reference() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_write_depth_fail_op
*
*********************************************************************/

static __inline VkStencilOp VKN_render_flags_get_stencil_write_depth_fail_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkStencilOp             ret;        /* return write operation       */

switch( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_MASK )
    {
    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_ZERO:
        ret = VK_STENCIL_OP_ZERO;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_REPLACE:
        ret = VK_STENCIL_OP_REPLACE;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR:
        ret = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR_WRAP:
        ret = VK_STENCIL_OP_INCREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR:
        ret = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR_WRAP:
        ret = VK_STENCIL_OP_DECREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INVERT:
        ret = VK_STENCIL_OP_INVERT;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_MASK ) == VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_KEEP );
        ret = VK_STENCIL_OP_KEEP;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_stencil_write_depth_fail_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_write_fail_op
*
*********************************************************************/

static __inline VkStencilOp VKN_render_flags_get_stencil_write_fail_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkStencilOp             ret;        /* return write operation       */

switch( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_MASK )
    {
    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_ZERO:
        ret = VK_STENCIL_OP_ZERO;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_REPLACE:
        ret = VK_STENCIL_OP_REPLACE;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR:
        ret = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR_WRAP:
        ret = VK_STENCIL_OP_INCREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR:
        ret = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR_WRAP:
        ret = VK_STENCIL_OP_DECREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INVERT:
        ret = VK_STENCIL_OP_INVERT;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_MASK ) == VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_KEEP );
        ret = VK_STENCIL_OP_KEEP;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_stencil_write_fail_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_stencil_write_pass_op
*
*********************************************************************/

static __inline VkStencilOp VKN_render_flags_get_stencil_write_pass_op
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkStencilOp             ret;        /* return write operation       */

switch( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_MASK )
    {
    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_ZERO:
        ret = VK_STENCIL_OP_ZERO;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_REPLACE:
        ret = VK_STENCIL_OP_REPLACE;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR:
        ret = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR_WRAP:
        ret = VK_STENCIL_OP_INCREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR:
        ret = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR_WRAP:
        ret = VK_STENCIL_OP_DECREMENT_AND_WRAP;
        break;

    case VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INVERT:
        ret = VK_STENCIL_OP_INVERT;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_MASK ) == VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_KEEP );
        ret = VK_STENCIL_OP_KEEP;
        break;
    }
    
return( ret );

}   /* VKN_render_flags_get_stencil_write_pass_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_get_topology
*
*********************************************************************/

static __inline VkPrimitiveTopology VKN_render_flags_get_topology
    (
    const VKN_render_flags_type
                        flags       /* render state flags           */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkPrimitiveTopology     ret;        /* return topology              */

switch( flags & VKN_RENDER_FLAG_BITS_TOPOLOGY_MASK )
    {
    case VKN_RENDER_FLAG_BITS_TOPOLOGY_POINTS:
        ret = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        break;

    case VKN_RENDER_FLAG_BITS_TOPOLOGY_LINES:
        ret = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        break;

    case VKN_RENDER_FLAG_BITS_TOPOLOGY_LINE_STRIP:
        ret = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        break;

    case VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLES:
        ret = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        break;

    case VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_STRIP:
        ret = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        break;

    default:
        debug_assert( ( flags & VKN_RENDER_FLAG_BITS_TOPOLOGY_MASK ) == VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_FAN );
        ret = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        break;
    }

return( ret );

}   /* VKN_render_flags_get_topology() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_blend_factor_destination_alpha
*
*********************************************************************/

static __inline void VKN_render_flags_set_blend_factor_destination_alpha
    (
    const VkBlendFactor factor,     /* destination alpha factor     */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_MASK );
switch( factor )
    {
    case VK_BLEND_FACTOR_ONE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE );
        break;

    case VK_BLEND_FACTOR_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_SRC_ALPHA );
        break;

    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA );
        break;

    default:
        debug_assert( factor == VK_BLEND_FACTOR_ZERO );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_ALPHA_BLEND_FACTOR_ZERO );
        break;        
    }

}   /* VKN_render_flags_set_blend_factor_destination_alpha() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_blend_factor_destination_color
*
*********************************************************************/

static __inline void VKN_render_flags_set_blend_factor_destination_color
    (
    const VkBlendFactor factor,     /* destination color factor     */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_MASK );
switch( factor )
    {
    case VK_BLEND_FACTOR_ONE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE );
        break;

    case VK_BLEND_FACTOR_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_SRC_ALPHA );
        break;

    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA );
        break;

    default:
        debug_assert( factor == VK_BLEND_FACTOR_ZERO );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DST_COLOR_BLEND_FACTOR_ZERO );
        break;        
    }

}   /* VKN_render_flags_set_blend_factor_destination_color() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_blend_factor_source_alpha
*
*********************************************************************/

static __inline void VKN_render_flags_set_blend_factor_source_alpha
    (
    const VkBlendFactor factor,     /* source alpha factor          */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_MASK );
switch( factor )
    {
    case VK_BLEND_FACTOR_ZERO:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ZERO );
        break;

    case VK_BLEND_FACTOR_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_SRC_ALPHA );
        break;

    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA );
        break;

    default:
        debug_assert( factor == VK_BLEND_FACTOR_ONE );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_ALPHA_BLEND_FACTOR_ONE );
        break;        
    }

}   /* VKN_render_flags_set_blend_factor_source_alpha() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_blend_factor_source_color
*
*********************************************************************/

static __inline void VKN_render_flags_set_blend_factor_source_color
    (
    const VkBlendFactor factor,     /* source color factor          */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_MASK );
switch( factor )
    {
    case VK_BLEND_FACTOR_ZERO:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ZERO );
        break;

    case VK_BLEND_FACTOR_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_SRC_ALPHA );
        break;

    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA );
        break;

    default:
        debug_assert( factor == VK_BLEND_FACTOR_ONE );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_SRC_COLOR_BLEND_FACTOR_ONE );
        break;        
    }

}   /* VKN_render_flags_set_blend_factor_source_color() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_blend_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_blend_op
    (
    const VkBlendOp     blend,      /* blending operation           */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_BLEND_OP_MASK );
switch( blend )
    {
    case VK_BLEND_OP_SUBTRACT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_BLEND_OP_SUBTRACT );
        break;

    case VK_BLEND_OP_MIN:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_BLEND_OP_MIN );
        break;

    case VK_BLEND_OP_MAX:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_BLEND_OP_MAX );
        break;

    default:
        debug_assert( blend == VK_BLEND_OP_ADD );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_BLEND_OP_ADD );
        break;
    }

}   /* VKN_render_flags_set_blend_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_color_mask
*
*********************************************************************/

static __inline void VKN_render_flags_set_color_mask
    (
    const VkColorComponentFlags
                        mask,       /* color write channels         */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
set_clear_bits( *flags, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_RED_DISABLE,   !test_bits( *flags, VK_COLOR_COMPONENT_R_BIT ) );
set_clear_bits( *flags, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_GREEN_DISABLE, !test_bits( *flags, VK_COLOR_COMPONENT_G_BIT ) );
set_clear_bits( *flags, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_BLUE_DISABLE,  !test_bits( *flags, VK_COLOR_COMPONENT_B_BIT ) );
set_clear_bits( *flags, VKN_RENDER_FLAG_BITS_COLOR_ENABLE_WRITE_ALPHA_DISABLE, !test_bits( *flags, VK_COLOR_COMPONENT_A_BIT ) );

}   /* VKN_render_flags_set_color_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_cull_mode
*
*********************************************************************/

static __inline void VKN_render_flags_set_cull_mode
    (
    const VkCullModeFlags
                        cull_mode,  /* culling side(s)              */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_CULL_MODE_MASK );
switch( cull_mode )
    {
    case VK_CULL_MODE_FRONT_BIT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT );
        break;

    case VK_CULL_MODE_BACK_BIT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_CULL_MODE_BACK );
        break;

    case VK_CULL_MODE_FRONT_AND_BACK:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_CULL_MODE_FRONT_AND_BACK );
        break;

    default:
        debug_assert( cull_mode == VK_CULL_MODE_NONE );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_CULL_MODE_NONE );
        break;
    }

}   /* VKN_render_flags_set_cull_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_depth_test_compare_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_depth_test_compare_op
    (
    const VkCompareOp   pass,       /* depth pass condition         */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_MASK );
switch( pass )
    {
    case VK_COMPARE_OP_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_EQUAL );
        break;

    case VK_COMPARE_OP_GREATER_OR_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_GREATER_EQUAL );
        break;

    case VK_COMPARE_OP_LESS_OR_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_LESS_EQUAL );
        break;

    default:
        debug_assert( pass == VK_COMPARE_OP_ALWAYS );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_TEST_PASS_ALWAYS );
        break;
    }

}   /* VKN_render_flags_set_depth_test_compare_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_depth_write_enabled
*
*********************************************************************/

static __inline void VKN_render_flags_set_depth_write_enabled
    (
    const bool          enabled,    /* enable depth write?          */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
set_clear_bits( *flags, VKN_RENDER_FLAG_BITS_DEPTH_ENABLE_WRITE_DISABLE, !enabled );

}   /* VKN_render_flags_set_depth_write_enabled() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_fill_mode
*
*********************************************************************/

static __inline void VKN_render_flags_set_fill_mode
    (
    const VkPolygonMode mode,       /* desired fill mode            */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_FILL_MODE_MASK );
switch( mode )
    {
    case VK_POLYGON_MODE_LINE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_FILL_MODE_WIREFRAME );
        break;

    default:
        debug_assert( mode == VK_POLYGON_MODE_FILL );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_FILL_MODE_SOLID );
        break;
    }

}   /* VKN_render_flags_set_fill_mode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_front_face_winding
*
*********************************************************************/

static __inline void VKN_render_flags_set_front_face_winding
    (
    const VkFrontFace   winding,    /* desired front winding        */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_MASK );
switch( winding )
    {
    case VK_FRONT_FACE_CLOCKWISE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CW );
        break;

    default:
        debug_assert( winding == VK_FRONT_FACE_COUNTER_CLOCKWISE );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_FRONT_FACE_WIND_CCW );
        break;
    }

}   /* VKN_render_flags_set_front_face_winding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_test_compare_mask
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_test_compare_mask
    (
    const u8            mask,       /* stencil test mask            */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_MASK );
set_bits( *flags, ( (VKN_render_flags_type)mask ) << VKN_RENDER_FLAG_BITS_STENCIL_TEST_MASK_SHIFT );

}   /* VKN_render_flags_set_stencil_test_compare_mask() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_test_compare_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_test_compare_op
    (
    const VkCompareOp   pass,       /* stencil pass condition       */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_MASK );
switch( pass )
    {
    case VK_COMPARE_OP_NEVER:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NEVER );
        break;

    case VK_COMPARE_OP_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_EQUAL );
        break;

    case VK_COMPARE_OP_NOT_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_NOT_EQUAL );
        break;

    case VK_COMPARE_OP_GREATER:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER );
        break;

    case VK_COMPARE_OP_GREATER_OR_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_GREATER_EQUAL );
        break;

    case VK_COMPARE_OP_LESS:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS );
        break;

    case VK_COMPARE_OP_LESS_OR_EQUAL:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_LESS_EQUAL );
        break;

    default:
        debug_assert( pass == VK_COMPARE_OP_ALWAYS );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_PASS_ALWAYS );
        break;
    }

}   /* VKN_render_flags_set_stencil_test_compare_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_test_reference
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_test_reference
    (
    const u8            reference,  /* stencil test reference value */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_MASK );
set_bits( *flags, ( (VKN_render_flags_type)reference) << VKN_RENDER_FLAG_BITS_STENCIL_TEST_REF_SHIFT );

}   /* VKN_render_flags_set_stencil_test_reference() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_write_depth_fail_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_write_depth_fail_op
    (
    const VkStencilOp   depth_fail, /* depth fail write operation   */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_MASK );
switch( depth_fail )
    {
    case VK_STENCIL_OP_ZERO:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_ZERO );
        break;

    case VK_STENCIL_OP_REPLACE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_REPLACE );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INCR_WRAP );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_DECR_WRAP );
        break;

    case VK_STENCIL_OP_INVERT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_INVERT );
        break;

    default:
        debug_assert( depth_fail == VK_STENCIL_OP_KEEP );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_DFAIL_KEEP );
        break;
    }

}   /* VKN_render_flags_set_stencil_write_depth_fail_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_write_fail_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_write_fail_op
    (
    const VkStencilOp   stencil_fail,
                                    /* stencil fail write operation */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_MASK );
switch( stencil_fail )
    {
    case VK_STENCIL_OP_ZERO:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_ZERO );
        break;

    case VK_STENCIL_OP_REPLACE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_REPLACE );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INCR_WRAP );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_DECR_WRAP );
        break;

    case VK_STENCIL_OP_INVERT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_INVERT );
        break;

    default:
        debug_assert( stencil_fail == VK_STENCIL_OP_KEEP );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_SFAIL_KEEP );
        break;
    }

}   /* VKN_render_flags_set_stencil_write_fail_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_stencil_write_pass_op
*
*********************************************************************/

static __inline void VKN_render_flags_set_stencil_write_pass_op
    (
    const VkStencilOp   pass,       /* stencil & depth pass write op*/
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_MASK );
switch( pass )
    {
    case VK_STENCIL_OP_ZERO:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_ZERO );
        break;

    case VK_STENCIL_OP_REPLACE:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_REPLACE );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR );
        break;

    case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INCR_WRAP );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR );
        break;

    case VK_STENCIL_OP_DECREMENT_AND_WRAP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_DECR_WRAP );
        break;

    case VK_STENCIL_OP_INVERT:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_INVERT );
        break;

    default:
        debug_assert( pass == VK_STENCIL_OP_KEEP );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_STENCIL_WRITE_PASS_KEEP );
        break;
    }

}   /* VKN_render_flags_set_stencil_write_pass_op() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_render_flags_set_topology
*
*********************************************************************/

static __inline void VKN_render_flags_set_topology
    (
    const VkPrimitiveTopology
                        primitive,  /* pipeline input primitive     */
    VKN_render_flags_type
                       *flags       /* modified render flags        */
    )
{
clear_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_MASK );
switch( primitive )
    {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_POINTS );
        break;

    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_LINES );
        break;

    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_LINE_STRIP );
        break;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLES );
        break;

    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_STRIP );
        break;

    default:
        debug_assert( primitive == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN );
        set_bits( *flags, VKN_RENDER_FLAG_BITS_TOPOLOGY_TRIANGLE_FAN );
        break;
    }

}   /* VKN_render_flags_set_topology() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_return_bfail
*
*   DESCRIPTION:
*       Returns FALSE if the boolean is FALSE.
*
*********************************************************************/

#define VKN_return_bfail( _bool ) \
    do                            \
        {                         \
        if( !(_bool) )            \
            {                     \
            return( false );      \
            }                     \
        } while( 0 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_return_bnull
*
*   DESCRIPTION:
*       Returns NULL if the boolean is FALSE.
*
*********************************************************************/

#define VKN_return_bnull( _bool ) \
    do                            \
        {                         \
        if( !(_bool) )            \
            {                     \
            return( nullptr );    \
            }                     \
        } while( 0 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_return_fail
*
*   DESCRIPTION:
*       Returns FALSE if the operation failed.
*
*********************************************************************/

#define VKN_return_fail( _op )       \
    do                               \
        {                            \
        if( VKN_failed( _op ) )      \
            {                        \
            debug_assert_always();   \
            return( false );         \
            }                        \
        } while( 0 )


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_size_max
*
*********************************************************************/

static __inline VkDeviceSize VKN_size_max
    (
    const VkDeviceSize  first,      /* take maximum of this...      */
    const VkDeviceSize  second      /* ...and this                  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return maximum size          */

ret = first;
if( second > first )
    {
    ret = second;
    }

return( ret );

}   /* VKN_size_max() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_size_min
*
*********************************************************************/

static __inline VkDeviceSize VKN_size_min
    (
    const VkDeviceSize  first,      /* take minimum of this...      */
    const VkDeviceSize  second      /* ...and this                  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return minimum size          */

ret = first;
if( second < first )
    {
    ret = second;
    }

return( ret );

}   /* VKN_size_min() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_size_min_power_of_two
*
*********************************************************************/

static __inline VkDeviceSize VKN_size_min_power_of_two
    (
    const VkDeviceSize  size        /* find minimum pow2 of this    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return maximum size          */

ret = 1;
while( ret < size )
    {
    ret <<= 1;
    }

return( ret );

}   /* VKN_size_min_power_of_two() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_size_round_down_mult
*
*   DESCRIPTION:
*       Round down a size to the nearest given multiple.
*
*********************************************************************/

static __inline VkDeviceSize VKN_size_round_down_mult
    (
    const VkDeviceSize  size,       /* size to round down           */
    const VkDeviceSize  multiple    /* multiple of this             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return rounded size          */

ret = size / multiple;
ret *= multiple;

return( ret );

}   /* VKN_size_round_down_mult() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_size_round_up_mult
*
*   DESCRIPTION:
*       Round up a size to the nearest given multiple.
*
*********************************************************************/

static __inline VkDeviceSize VKN_size_round_up_mult
    (
    const VkDeviceSize  size,       /* size to round up             */
    const VkDeviceSize  multiple    /* multiple of this             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VkDeviceSize            ret;        /* return rounded size          */

ret = size + multiple - 1;
ret /= multiple;
ret *= multiple;

return( ret );

}   /* VKN_size_round_up_mult() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_uniform_get_align_adjust
* 
*   DESCRIPTION:
*       Get the padding required to the given offset for the given
*       float count;
*
*********************************************************************/
    
static __inline u32 VKN_uniform_get_align_adjust
    (
    const u8            num_floats, /* number of floats in parameter*/
    const u32           offset      /* current offset               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     alignment;  /* required alignment           */

switch( num_floats )
    {
    case 1:
        alignment = sizeof( float );
        break;

    case 2:
        alignment = 2 * sizeof( float );
        break;

    case 3:
    case 4:
        alignment = 4 * sizeof( float );
        break;

    default:
        debug_assert_always();
        return( 0 );
    }

return( (u32)VKN_size_round_up_mult( offset, alignment ) - offset );

}   /* VKN_uniform_get_align_adjust() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_uniform_get_size
* 
*   DESCRIPTION:
*       Calculate the final size of a uniform, by repeatedly giving
*       the number of floats and current sum.
*
*********************************************************************/
    
static __inline u32 VKN_uniform_get_size
    (
    const u8            num_floats, /* number of floats in parameter*/
    const u32           tally_size  /* sum so far                   */
    )
{
return( VKN_uniform_get_align_adjust( num_floats, tally_size ) + num_floats * (u32)sizeof( float ) );

}   /* VKN_uniform_get_size() */
