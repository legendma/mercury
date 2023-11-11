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
*   COMPONENT_CONTROLLER_INPUT - ControllerInputComponent
*
*******************************************************************/

typedef enum _ControllerButtons 
    {
    CONTROLLER_BUTTON_DPAD_CENTER,
    CONTROLLER_BUTTON_DPAD_UP,
    CONTROLLER_BUTTON_DPAD_UPRIGHT,
    CONTROLLER_BUTTON_DPAD_RIGHT,
    CONTROLLER_BUTTON_DPAD_DOWNRIGHT,
    CONTROLLER_BUTTON_DPAD_DOWN,
    CONTROLLER_BUTTON_DPAD_DOWNLEFT,
    CONTROLLER_BUTTON_DPAD_LEFT,
    CONTROLLER_BUTTON_DPAD_UPLEFT,
    CONTROLLER_BUTTON_LEFT_STICK_CLICK,
    CONTROLLER_BUTTON_RIGHT_STICK_CLICK,
    CONTROLLER_BUTTON_L1,
    CONTROLLER_BUTTON_R1,
    CONTROLLER_BUTTON_L2,
    CONTROLLER_BUTTON_R2,
    CONTROLLER_BUTTON_MENU,
    CONTROLLER_BUTTON_OPTIONS,
    CONTROLLER_BUTTON_CENTER_PAD,
    CONTROLLER_BUTTON_POWER,
    CONTROLLER_BUTTON_MUTE,
    CONTROLLER_BUTTON_BOT_BUTTON,
    CONTROLLER_BUTTON_TOP_BUTTON,
    CONTROLLER_BUTTON_LEFT_BUTTON,
    CONTROLLER_BUTTON_RIGHT_BUTTON,
    /* Count */
    CONTROLLER_BUTTON_COUNT
    } ControllerButtons;

typedef enum _ControllerAxis
    {
    CONTROLLER_AXIS_LEFT_STICK,
    CONTROLLER_AXIS_RIGHT_STICK,
    /* Count */
    CONTROLLER_AXIS_COUNT
    } ControllerAxis;

typedef enum _ControllerTriggers
    {
    CONTROLLER_TRIGGERS_LEFT,
    CONTROLLER_TRIGGERS_RIGHT,
    /* count */
    CONTROLLER_TRIGGERS_COUNT
    } ControllerTriggers;


typedef struct _ControllerInputComponent
    {
    uint32_t            button_state;
    Vec2                axis_state;
    float               trigger_state;
    } ControllerInputComponent;

/*******************************************************************
*
*   COMPONENT_SINGLETON_PLAYER_INPUT - SingletonPlayerInputComponent
*
*******************************************************************/

typedef struct _SingletonPlayerInputComponent
    {
    void               *ptr;
    } SingletonPlayerInputComponent;

/*******************************************************************
*
*   COMPONENT_SINGLETON_RENDER - SingletonRenderComponent
*
*******************************************************************/

typedef struct _SingletonRenderComponent
    {
    void               *ptr;
    } SingletonRenderComponent;

/*******************************************************************
*
*   Component Classes
*
*******************************************************************/

typedef enum
    {
    COMPONENT_HEALTH,
    COMPONENT_TRANSFORM_3D,
    COMPONENT_SINGLETON_PLAYER_INPUT,
    COMPONENT_SINGLETON_RENDER,
    COMPONENT_CONTROLLER_INPUT,
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
    { COMPONENT_HEALTH,                 sizeof( HealthComponent )               },
    { COMPONENT_TRANSFORM_3D,           sizeof( Transform3DComponent )          },
    { COMPONENT_CONTROLLER_INPUT,       sizeof( ControllerInputComponent )      },
    { COMPONENT_SINGLETON_PLAYER_INPUT, sizeof( SingletonPlayerInputComponent ) },
    { COMPONENT_SINGLETON_RENDER,       sizeof( SingletonRenderComponent )      }
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