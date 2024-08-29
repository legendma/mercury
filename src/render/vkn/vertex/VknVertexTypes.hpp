#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_VERTEX_CONFIG_API       const struct _VKN_vertex_build_config_type *


typedef struct _VKN_vertex_attribute_type
    {
    VkVertexInputAttributeDescription
                        attribute;  /* vertex attribute             */
    struct _VKN_vertex_attribute_type
                       *next;       /* next attribute               */
    } VKN_vertex_attribute_type;

typedef struct _VKN_vertex_binding_type
    {
    VkVertexInputBindingDescription
                        binding;    /* vertex binding               */
    struct _VKN_vertex_binding_type
                       *next;       /* next binding                 */
    } VKN_vertex_binding_type;

typedef struct
    {
    VKN_vertex_attribute_type
                       *attributes; /* vertex attributes            */
    VKN_vertex_binding_type
                       *bindings;   /* vertex bindings              */
    u32                 fingerprint;/* vertex fingerprint           */
    } VKN_vertex_type;

typedef VKN_VERTEX_CONFIG_API VKN_vertex_build_add_attribute_proc_type
    (
    const u32           location,   /* attribute location           */
    const VkFormat      format,     /* attribute data type          */
    const u32           offset,     /* element binding offset       */
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

typedef VKN_VERTEX_CONFIG_API VKN_vertex_build_add_binding_proc_type
    (
    u32                 binding,    /* binding index                */
    u32                 stride,     /* consecutive element stride   */
    VkVertexInputRate   rate,       /* addressing mode              */
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

typedef VKN_vertex_type VKN_vertex_build_build_proc_type
    (
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

typedef struct _VKN_vertex_build_config_type
    {
    VKN_vertex_build_add_attribute_proc_type
                       *add_attribute;
                                    /* add attribute to working vert*/
    VKN_vertex_build_add_binding_proc_type
                       *add_binding;/* add binding to working vertex*/
    VKN_vertex_build_build_proc_type
                       *build;      /* build working vertex         */
    } VKN_vertex_build_config_type;

typedef struct
    {
    VKN_vertex_attribute_type       /* attribute pool               */
                       *free_attributes;
    VKN_vertex_binding_type         /* binding pool                 */
                       *free_bindings;
    VKN_vertex_attribute_type       /* working attribute head       */
                       *head_attribute;
    VKN_vertex_binding_type         /* working binding head         */
                       *head_binding;
    VKN_vertex_attribute_type       /* working attribute tail       */
                       *tail_attribute;
    VKN_vertex_binding_type         /* working binding tail         */
                       *tail_binding;
    } VKN_vertex_build_state_type;

typedef struct _VKN_vertex_build_type
    {
    VKN_vertex_build_state_type
                        state;      /* builder state                */
    const VKN_vertex_build_config_type
                       *config;     /* configuration interface      */
    } VKN_vertex_build_type;
