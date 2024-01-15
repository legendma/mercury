#include "D3D12Core.hpp"
#include "D3D12Pipelines.hpp"

namespace D3D12 { namespace Pipelines
{

/*******************************************************************
*
*   CreatePipelineState()
*
*   DESCRIPTION:
*       Create a pipeline state object.
*
*******************************************************************/

Core::IMPipelineState * CreatePSO( const D3D12_PIPELINE_STATE_STREAM_DESC *desc )
{
Core::IMPipelineState *ret = NULL;
if( FAILED( Core::GetDevice()->CreatePipelineState( desc, IID_PPV_ARGS( &ret ) ) ) )
    {
    debug_assert_always();
    return( NULL );
    }

return( ret );

} /* CreatePipelineState() */


} }/* namespace D3D12::Pipelines */