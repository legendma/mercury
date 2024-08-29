#pragma once

#include <cstddef>

#include "Global.hpp"


typedef u32 VKN_arena_token_type;
typedef u64 VKN_arena_word_type;

typedef struct
    {
    VKN_arena_word_type
                       *memory;     /* managed memory               */
    u64                 size;       /* size of managed memory       */
    VKN_arena_token_type
                        caret;      /* current allocation position  */
    } VKN_arena_type;
