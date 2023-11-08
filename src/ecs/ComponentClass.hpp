#pragma once

#include <cstdint>

#include "Math.hpp"
#include "Utilities.hpp"

namespace ECS
{

/*******************************************************************
*
*   COMPONENT_HEALTH - HealthComponent
*
*******************************************************************/

typedef struct _HealthComponent
    {
    uint32_t            max_health;
    uint32_t            current_health;
    } HealthComponent;

/*******************************************************************
*
*   COMPONENT_TRANSFORM_3D - Transform3DComponent
*
*******************************************************************/

typedef struct _Transform3DComponent
    {
    Vec4                translation;
    Vec3                scale;
    Quaternion          rotation;
    } Transform3DComponent;


/*******************************************************************
*
*   Component Classes
*
*******************************************************************/

typedef enum
    {
    COMPONENT_HEALTH,
    COMPONENT_TRANSFORM_3D,
    /* count */
    COMPONENT_CNT
    } ComponentClass;
    
typedef struct _ComponentClassSizes
    {
    ComponentClass      cls;
    size_t              size;
    } ComponentClassSizes;

static const ComponentClassSizes COMPONENT_CLASS_SIZES[] =
    {
    { COMPONENT_HEALTH,       sizeof( HealthComponent )      },
    { COMPONENT_TRANSFORM_3D, sizeof( Transform3DComponent ) }
    };
compiler_assert( cnt_of_array( COMPONENT_CLASS_SIZES ) == COMPONENT_CNT, component_class_hpp );


/*******************************************************************
*
*   GetComponentClassSize()
*
*   DESCRIPTION:
*       Handy helper method to search for a component class's
*       component size.
*
*******************************************************************/

static inline size_t GetComponentClassSize( const ComponentClass component )
{
size_t ret = 0;
for( uint32_t i = 0; i < cnt_of_array( COMPONENT_CLASS_SIZES ); i++ )
    {
    if( COMPONENT_CLASS_SIZES[ i ].cls == component )
        {
        ret = COMPONENT_CLASS_SIZES[ i ].size;
        break;
        }
    }

return( ret );

}   /* GetComponentClassSize() */


} /* namespace ECS */