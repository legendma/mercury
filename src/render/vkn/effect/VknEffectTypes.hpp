#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknShaderTypes.hpp"


#define VKN_EFFECT_MAX_BINDINGS_CNT ( 50 )
#define VKN_EFFECT_MAX_PUSH_CONSTANT_CNT \
                                    ( 10 )
#define VKN_EFFECT_MAX_UNIFORM_PARAMS_CNT \
                                    ( ( 3 * VKN_SHADER_MAX_UNIFORM_PARAMS_CNT ) / 2 )
#define VKN_EFFECT_MAX_NAME_LEN     ( 15 )

#define VKN_EFFECT_CONFIG_API       const struct _VKN_effect_build_config_type *


typedef VKN_EFFECT_CONFIG_API VKN_effect_build_add_stage_proc_type
    (
    const VKN_shader_gfx_stage_type
                        stage,      /* graphics stage name          */
    const VKN_shader_type
                       *shader,     /* shader module                */
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

typedef VKN_EFFECT_CONFIG_API VKN_effect_build_finalize_stages_proc_type
    (
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

typedef VKN_EFFECT_CONFIG_API VKN_effect_build_reset_proc_type
    (
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

typedef VKN_EFFECT_CONFIG_API VKN_effect_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_effect_build_type
                       *builder     /* shader effect builder        */
    );

typedef struct _VKN_effect_build_config_type
    {
    VKN_effect_build_add_stage_proc_type
                       *add_stage;  /* add shader stage             */
    VKN_effect_build_finalize_stages_proc_type
                       *finalize_stages;
                                    /* complete stage description   */
    VKN_effect_build_reset_proc_type
                       *reset;      /* reset the builder state      */
    VKN_effect_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    } VKN_effect_build_config_type;

typedef struct
    {
    u32                 set;        /* set number                   */
    VkDescriptorSetLayoutBinding
                       *binding;    /* descriptor binding           */
    VKN_shader_parameter_mapping_type
                       *parameters; /* shader parameters mapping    */
    } VKN_effect_build_set_binding_type;

typedef struct
    {
    VKN_effect_build_set_binding_type
                        set_bindings[ VKN_EFFECT_MAX_BINDINGS_CNT ];
                                    /* descriptor set bindings      */
    u32                 count;      /* number of set bindings       */
    } VKN_effect_build_set_bindings_type;

typedef struct
    {
    VKN_shader_parameter_mapping_type
                        *parameters[ VKN_EFFECT_MAX_PUSH_CONSTANT_CNT ];
                                    /* push constant shader params  */
    VkPushConstantRange ranges[ VKN_EFFECT_MAX_PUSH_CONSTANT_CNT ];
                                    /* push constant range          */
    u32                 count;      /* number of push constants     */
    } VKN_effect_build_push_constants_type;

typedef struct
    {
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    const VKN_shader_type           /* shader stages                */
                       *stages[ VKN_SHADER_GFX_STAGE_CNT ];
    VkDevice            logical;    /* associated logical device    */
    VkDescriptorSetLayoutBinding
                        bindings[ VKN_EFFECT_MAX_BINDINGS_CNT ];
                                    /* descriptor bindings          */
    VKN_shader_parameter_mapping_type
                        param_pool[ VKN_EFFECT_MAX_UNIFORM_PARAMS_CNT ];
    u32                 param_cnt;  /* number of used parameters    */
    VKN_effect_build_set_bindings_type
                        set_bindings;
                                    /* descriptor set bindings      */
    VKN_effect_build_push_constants_type
                        push_constants;
                                    /* push constant ranges         */
    } VKN_effect_build_state_type;

typedef struct _VKN_effect_build_type
    {
    VKN_effect_build_state_type
                        state;      /* builder state                */
    const VKN_effect_build_config_type
                       *config;     /* configuration interface      */
    } VKN_effect_build_type;

typedef struct
    {
    VkPipelineShaderStageCreateFlags
                        flags;      /* create info flags            */
    VkShaderStageFlags  stage;      /* shader stage                 */
    VkShaderModule      shader;     /* shader object                */
    const char         *entry_point;/* shader entry point           */
    } VKN_effect_pipeline_stage_info_type;

typedef struct _VKN_effect_parameter_mapping_type
    {
    u32                 binding;    /* binding index                */
    VKN_shader_uniform_param_cls_type
                        cls;        /* param class tag              */
    struct _VKN_effect_parameter_mapping_type
                       *next;       /* next vector                  */
    union
        {
        struct
            {
            VKN_shader_param_vector_name_type
                        vector;     /* vector name                  */
            u8          width;      /* vector element width         */
            } vector;
        VKN_shader_param_image_name_type
                        image;      /* image name                   */
        };
    } VKN_effect_parameter_mapping_type;

typedef struct
    {
    u32                 set_fingerprints[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* descriptor set fingerprint   */
    u32                 stage_cnt;  /* pipeline stage create count  */
    u32                 push_constant_cnt;
                                    /* number of push constants     */
    char                debug_name[ VKN_EFFECT_MAX_NAME_LEN + 1 ];
                                    /* effect string name for debug */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VkPipelineLayout    layout;     /* pipeline layout              */
    VkDescriptorSetLayout           /* descriptor set layouts       */
                        sets[ VKN_DESCRIPTOR_SET_CNT ];
    VKN_effect_parameter_mapping_type
                       *set_params[ VKN_DESCRIPTOR_SET_CNT ];
                                    /* desriptor set shader params  */
    VKN_effect_pipeline_stage_info_type
                        stages[ VKN_SHADER_GFX_STAGE_CNT ];
                                    /* pipeline stage info          */
    VkPushConstantRange push_constants[ VKN_EFFECT_MAX_PUSH_CONSTANT_CNT ];
                                    /* push constant ranges         */
    VKN_effect_parameter_mapping_type
                       *push_constant_params[ VKN_EFFECT_MAX_PUSH_CONSTANT_CNT ];
                                    /* push constant shader params  */
    VKN_effect_parameter_mapping_type
                        param_pool[ VKN_EFFECT_MAX_UNIFORM_PARAMS_CNT ];
                                    /* shader parameter map pool    */
    } VKN_effect_type;
