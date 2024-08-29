#pragma once

#include <cstring>

#include "Utilities.hpp"

#include "VknCommon.hpp"

#include "ShaderParams.hpp"


typedef enum
    {
    VKN_SHADER_PARAM_IMAGE_NAME_0,
    VKN_SHADER_PARAM_IMAGE_NAME_1,
    VKN_SHADER_PARAM_IMAGE_NAME_2,
    VKN_SHADER_PARAM_IMAGE_NAME_3,

    /*----------------------------------------------------------
    Count
    ----------------------------------------------------------*/
    VKN_SHADER_PARAM_IMAGE_NAME_CNT
    } VKN_shader_param_image_name_type;

#define VKN_SHADER_VEC4_FLT_CNT     ( 4 )


#define VKN_SHADER_PARAM_BA_WIDTH( _cnt ) \
    ( ( (_cnt) + sizeof( char ) - 1 ) / sizeof( char ) )

typedef struct
    {
    float               v[ VKN_SHADER_VEC4_FLT_CNT ];
                                    /* vec4                         */
    } VKN_shader_param_vector_type;

typedef struct
    {
    VkImageLayout       layout;     /* image layout                 */
    VkImageView         view;       /* image view                   */
    VkSampler           sampler;    /* image sampler                */
    } VKN_shader_param_image_type;

typedef struct
    {
    VKN_shader_param_vector_type    /* vector values                */
                        vectors[ VKN_SHADER_PARAM_VECTOR_NAME_CNT ];
    char                vector_dirty[ VKN_SHADER_PARAM_BA_WIDTH( VKN_SHADER_PARAM_VECTOR_NAME_CNT ) ];
                                    /* dirty flags for vectors      */
    VKN_shader_param_image_type     /* image values                 */
                        images[ VKN_SHADER_PARAM_IMAGE_NAME_CNT ];
    char                image_dirty[ VKN_SHADER_PARAM_BA_WIDTH( VKN_SHADER_PARAM_IMAGE_NAME_CNT ) ];
                                    /* dirty flags for images       */
    } VKN_shader_param_type;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_get_image_is_dirty
*
*********************************************************************/

static __inline bool VKN_shader_param_get_image_is_dirty
    (
    const VKN_shader_param_image_name_type
                        name,       /* parameter name               */
    VKN_shader_param_type
                       *parameters  /* parameters                   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     index;      /* array index                  */
u32                     offset;     /* dirty flag offset            */

index  = (u32)name / sizeof( *parameters->image_dirty );
offset = (u32)name % sizeof( *parameters->image_dirty );

return( test_bits( parameters->image_dirty[ index ], shift_bits( 1, offset ) ) );

}   /* VKN_shader_param_get_image_is_dirty() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_invalidate_all
*
*********************************************************************/

static __inline void VKN_shader_param_invalidate_all
    (
    VKN_shader_param_type
                       *parameters  /* parameters                   */
    )
{
memset( parameters->image_dirty, 0xff, sizeof( parameters->image_dirty ) );
memset( parameters->vector_dirty, 0xff, sizeof( parameters->vector_dirty ) );

}   /* VKN_shader_param_invalidate_all() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_make_vector
*
*********************************************************************/

static __inline VKN_shader_param_vector_type VKN_shader_param_make_vector
    (
    const float         x,          /* x-coordinate                 */
    const float         y,          /* y-coordinate                 */
    const float         z,          /* z-coordinate                 */
    const float         w           /* w-coordinate                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_shader_param_vector_type
                        ret;        /* return vector                */

ret.v[ 0 ] = x;
ret.v[ 1 ] = y;
ret.v[ 2 ] = z;
ret.v[ 3 ] = w;

return( ret );

}   /* VKN_shader_param_make_vector() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_set_image_is_dirty
*
*********************************************************************/

static __inline void VKN_shader_param_set_image_is_dirty
    (
    const bool          is_dirty,   /* is parameter dirty?          */
    const VKN_shader_param_image_name_type
                        name,       /* parameter name               */
    VKN_shader_param_type
                       *parameters  /* parameters                   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     index;      /* array index                  */
u32                     offset;     /* dirty flag offset            */

index  = (u32)name / sizeof( *parameters->image_dirty );
offset = (u32)name % sizeof( *parameters->image_dirty );

set_clear_bits( parameters->image_dirty[ index ], shift_bits( 1, offset ), is_dirty );

}   /* VKN_shader_param_set_image_is_dirty() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_get_vector_is_dirty
*
*********************************************************************/

static __inline bool VKN_shader_param_get_vector_is_dirty
    (
    const VKN_shader_param_vector_name_type
                        name,       /* parameter name               */
    VKN_shader_param_type
                       *parameters  /* parameters                   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     index;      /* array index                  */
u32                     offset;     /* dirty flag offset            */

index  = (u32)name / sizeof( *parameters->vector_dirty );
offset = (u32)name % sizeof( *parameters->vector_dirty );

return( test_bits( parameters->vector_dirty[ index ], shift_bits( 1, offset ) ) );

}   /* VKN_shader_param_get_vector_is_dirty() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_param_set_vector_is_dirty
*
*********************************************************************/

static __inline void VKN_shader_param_set_vector_is_dirty
    (
    const bool          is_dirty,   /* is parameter dirty?          */
    const VKN_shader_param_vector_name_type
                        name,       /* parameter name               */
    VKN_shader_param_type
                       *parameters  /* parameters                   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     index;      /* array index                  */
u32                     offset;     /* dirty flag offset            */

index  = (u32)name / sizeof( *parameters->vector_dirty );
offset = (u32)name % sizeof( *parameters->vector_dirty );

set_clear_bits( parameters->vector_dirty[ index ], shift_bits( 1, offset ), is_dirty);

}   /* VKN_shader_param_set_vector_is_dirty() */


struct
    {
    VKN_shader_param_vector_name_type
                        first;
    VKN_shader_param_vector_name_type
                        last;
    } static const VKN_SHADER_PARAM_SET_LIMITS[ VKN_DESCRIPTOR_SET_CNT ] =
    {
    { VKN_SHADER_PARAM_SET_0_FIRST, VKN_SHADER_PARAM_SET_0_LAST },
    { VKN_SHADER_PARAM_SET_1_FIRST, VKN_SHADER_PARAM_SET_1_LAST },
    { VKN_SHADER_PARAM_SET_2_FIRST, VKN_SHADER_PARAM_SET_2_LAST },
    { VKN_SHADER_PARAM_SET_3_FIRST, VKN_SHADER_PARAM_SET_3_LAST }
    };
