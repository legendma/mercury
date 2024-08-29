#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


typedef struct
    {
    u32                 name;       /* string name hash             */
    VkDescriptorType    descriptor; /* desired descriptor type      */
    } VKN_shader_reflect_spirv_override_type;

typedef struct
    {
    u32                 name;       /* string name hash             */
    u32                 set;        /* set number                   */
    u32                 binding;    /* binding index                */
    VkDescriptorType    kind;       /* descriptor type              */
    u32                 count;      /* array count                  */
    } VKN_shader_reflect_descriptor_binding_type;

typedef struct
    {
    u32                 name;       /* string name hash             */
    VkPushConstantRange constant;   /* push constant                */
    } VKN_shader_reflect_push_constant_type;
