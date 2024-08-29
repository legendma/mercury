#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknShaderReflectTypes.hpp"


void VKN_shader_reflect_make_override
    (
    const char         *name,       /* binding string name          */
    const VkDescriptorType
                        descriptor, /* desired descriptor type      */
    VKN_shader_reflect_spirv_override_type
                       *out_override/* output override              */
    );

bool VKN_shader_reflect_spirv
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt
                                    /* push constant count          */
    );

bool VKN_shader_reflect_spirv_w_overrides
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt
                                    /* push constant count          */
    );
