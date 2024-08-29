#pragma once

#include "Global.hpp"

#include "VknVertexTypes.hpp"


VKN_VERTEX_CONFIG_API VKN_vertex_init_builder
    (
    const u32           attribute_capacity,
                                    /* num elements attribute pool  */
    const u32           binding_capacity,
                                    /* num elements in binding pool */
    VKN_vertex_attribute_type
                       *attributes, /* attribute pool               */
    VKN_vertex_binding_type
                       *bindings,   /* binding pool                 */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );
