#pragma once

#include "Global.hpp"

#include "VknEffectTypes.hpp"
#include "VknProgramTypes.hpp"
#include "VknReleaserTypes.hpp"
#include "VknVertexTypes.hpp"


void VKN_program_create
    (
    const VKN_vertex_type
                       *vertices,   /* vertex type definitions      */
    const VKN_effect_type
                       *effect,     /* underlying effect            */
    VKN_program_type   *program     /* output new program           */
    );

void VKN_program_destroy
    (
    VKN_releaser_type  *releaser,   /* release buffer               */
    VKN_program_type   *program     /* program to destroy           */
    );
