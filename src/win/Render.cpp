#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "universe.hpp"

using namespace ECS;

typedef struct _DX12Render
    {
    int i;
    } DX12Render;

static DX12Render * AsDX12Render( Universe *universe );


/*******************************************************************
*
*   Render_Init()
*
*   DESCRIPTION:
*       Initialize the graphics system.
*       Returns TRUE if the DirectX 12 library was successfully
*       loaded.
*
*******************************************************************/

bool Render_Init( Universe *universe )
{    
SingletonRenderComponent* component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe);
component->ptr = malloc( sizeof( DX12Render ) );
if( !component->ptr )
    {
    return( false );
    }

DX12Render *render = (DX12Render*)component->ptr;
memset( render, 0, sizeof( *render ) );

  
return( true );

} /* PlayerInput_Init() */


/*******************************************************************
*
*   Render_Destroy()
*
*   DESCRIPTION:
*       Destroy the Render system and free its resources.
*
*******************************************************************/

void Render_Destroy(Universe* universe)
{
DX12Render *render = AsDX12Render( universe );

free( render );
render = NULL;

} /* Render_Destroy() */


/*******************************************************************
*
*   Render_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void Render_DoFrame( float frame_delta, Universe *universe )
{

} /* Render_DoFrame() */


/*******************************************************************
*
*   AsDX12Render()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static DX12Render * AsDX12Render( Universe* universe )
{
SingletonRenderComponent * component = (SingletonRenderComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
return( (DX12Render *)component->ptr );

} /* AsDX12Render() */
