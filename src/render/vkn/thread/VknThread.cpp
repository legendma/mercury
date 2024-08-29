#if !defined( _VKN_NO_PTHREADS )
#define HAVE_STRUCT_TIMESPEC
#include "pthread.h"
#endif /* !defined( _VKN_NO_PTHREADS ) */

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknThread.hpp"
#include "VknThreadTypes.hpp"

#if !defined( _VKN_NO_PTHREADS )


typedef struct
    {
    pthread_mutex_t     mutex;
    } mutex_priv_type;
compiler_assert( sizeof( mutex_priv_type ) <= sizeof( size_t ) * VKN_THREAD_MUTEX_SZ, VKN_THREAD_C );


static VKN_thread_mutex_lock_proc_type mutex_lock;
static VKN_thread_mutex_unlock_proc_type mutex_unlock;


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_thread_mutex_create
*
*   DESCRIPTION:
*       Create a mutex.
*
*********************************************************************/

void VKN_thread_mutex_create
    (
    VKN_thread_mutex_type
                       *mutex       /* output new mutex             */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_thread_mutex_api_type API =
    {
    mutex_lock,
    mutex_unlock
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mutex_priv_type        *priv;       /* private data                 */

clr_struct( mutex );
mutex->i = &API;

priv = (mutex_priv_type*)mutex->priv;
priv->mutex = PTHREAD_MUTEX_INITIALIZER;

}   /* VKN_thread_mutex_create() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_thread_mutex_destroy
*
*   DESCRIPTION:
*       Destroy a mutex.
*
*********************************************************************/

void VKN_thread_mutex_destroy
    (
    VKN_thread_mutex_type
                       *mutex       /* mutex to destroy             */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mutex_priv_type        *priv;       /* private data                 */

priv = (mutex_priv_type*)mutex->priv;
do_debug_assert( pthread_mutex_destroy( &priv->mutex ) );
clr_struct( mutex );

}   /* VKN_thread_mutex_destroy() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       mutex_lock
*
*********************************************************************/

static void mutex_lock
    (
    struct _VKN_thread_mutex_type
                       *mutex       /* mutex to lock                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mutex_priv_type        *priv;       /* private data                 */

priv = (mutex_priv_type*)mutex->priv;
do_debug_assert( !pthread_mutex_lock( &priv->mutex ) );

}   /* mutex_lock() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       mutex_unlock
*
*********************************************************************/

static void mutex_unlock
    (
    struct _VKN_thread_mutex_type
                       *mutex       /* mutex to unlock              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mutex_priv_type        *priv;       /* private data                 */

priv = (mutex_priv_type*)mutex->priv;
do_debug_assert( !pthread_mutex_unlock( &priv->mutex ) );

}   /* mutex_unlock() */


#endif /* !defined( _VKN_NO_PTHREADS ) */