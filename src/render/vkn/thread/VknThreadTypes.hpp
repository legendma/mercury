#pragma once

#include <cstddef>

#if !defined( _VKN_NO_PTHREADS )
#if defined( _WIN64 )
#define VKN_THREAD_MUTEX_SZ_BYTES   ( 8 )
#else
#define VKN_THREAD_MUTEX_SZ_BYTES   ( 4 )
#endif /* defined( _WIN64 ) */
#endif /* !defined( _VKN_NO_PTHREADS ) */

#define VKN_THREAD_MUTEX_SZ          ( ( VKN_THREAD_MUTEX_SZ_BYTES + (sizeof(size_t) - 1) ) / sizeof(size_t) )


typedef void VKN_thread_mutex_lock_proc_type
    (
    struct _VKN_thread_mutex_type
                       *mutex       /* mutex to lock                */
    );

typedef void VKN_thread_mutex_unlock_proc_type
    (
    struct _VKN_thread_mutex_type
                       *mutex       /* mutex to unlock              */
    );

typedef struct
    {
    VKN_thread_mutex_lock_proc_type
                       *lock;
    VKN_thread_mutex_unlock_proc_type
                       *unlock;
    } VKN_thread_mutex_api_type;

typedef struct _VKN_thread_mutex_type
    {
    const VKN_thread_mutex_api_type
                         *i;
    size_t                priv[ VKN_THREAD_MUTEX_SZ ];
    } VKN_thread_mutex_type;
