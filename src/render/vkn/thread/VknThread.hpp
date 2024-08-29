#pragma once

#include "VknThreadTypes.hpp"


void VKN_thread_mutex_create
    (
    VKN_thread_mutex_type
                       *mutex       /* output new mutex             */
    );

void VKN_thread_mutex_destroy
    (
    VKN_thread_mutex_type
                       *mutex       /* mutex to destroy             */
    );
