#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"
#include "VknShaderParamTypes.hpp"
#include "VknShaderReflectTypes.hpp"


#define VKN_SHADER_CONFIG_API       const struct _VKN_shader_build_config_type *

#define VKN_SHADER_MAX_DESCRIPTOR_BINDING_CNT \
                                    ( 50 )
#define VKN_SHADER_MAX_PUSH_CONSTANT_CNT \
                                    ( 10 )
#define VKN_SHADER_MAX_OVERRIDE_CNT ( 20 )
#define VKN_SHADER_MAX_UNIFORM_PARAMS_CNT \
                                    ( 30 )

typedef enum
    {
    VKN_SHADER_UNIFORM_PARAM_CLS_INVALID,
    VKN_SHADER_UNIFORM_PARAM_CLS_IMAGE,
    VKN_SHADER_UNIFORM_PARAM_CLS_VECTOR
    } VKN_shader_uniform_param_cls_type;


typedef VKN_SHADER_CONFIG_API VKN_shader_build_add_override_proc_type
    (
    const char         *name,       /* binding string name          */
    const VkDescriptorType
                        descriptor, /* desired descriptor type      */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef VKN_SHADER_CONFIG_API VKN_shader_build_map_uniform_image_proc_type
    (
    const char         *uniform,    /* uniform string name          */
    const VKN_shader_param_image_name_type
                        image,      /* image parameter name         */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef VKN_SHADER_CONFIG_API VKN_shader_build_map_uniform_vector_proc_type
    (
    const char         *uniform,    /* uniform string name          */
    const VKN_shader_param_vector_name_type
                        vector,     /* vector parameter name        */
    const u8            width,      /* number of floats in vector   */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef VKN_SHADER_CONFIG_API VKN_shader_build_reset_proc_type
    (
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef VKN_SHADER_CONFIG_API VKN_shader_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef VKN_SHADER_CONFIG_API VKN_shader_build_set_code_proc_type
    (
    const u32          *code,       /* shader bytecode              */
    const u32           code_sz,    /* shader code byte size        */
    struct _VKN_shader_build_type
                       *builder     /* shader module builder        */
    );

typedef struct _VKN_shader_build_config_type
    {
    VKN_shader_build_add_override_proc_type
                       *add_override;
                                    /* add reflection override      */
    VKN_shader_build_map_uniform_image_proc_type
                       *map_uniform_image;
                                    /* map uniform image param      */
    VKN_shader_build_map_uniform_vector_proc_type
                       *map_uniform_vector;
                                    /* map uniform vector param     */
    VKN_shader_build_reset_proc_type
                       *reset;      /* reset configuration defaults */
    VKN_shader_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_shader_build_set_code_proc_type
                       *set_code;   /* set bytecode to compile      */
    } VKN_shader_build_config_type;

typedef struct
    {
    VKN_shader_reflect_spirv_override_type
                        overrides[ VKN_SHADER_MAX_OVERRIDE_CNT ];
                                    /* descriptor binding overrides */
    u32                 count;      /* number of overrides          */
    } VKN_shader_build_overrides_type;

typedef struct
    {
    u32                 uniform_name;
                                    /* uniform string name hash     */
    VKN_shader_uniform_param_cls_type
                        cls;        /* param class tag              */
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
    } VKN_shader_build_uniform_param_type;

typedef struct
    {
    VKN_shader_build_uniform_param_type
                        params[ VKN_SHADER_MAX_UNIFORM_PARAMS_CNT ];
                                    /* uniform shader parameters    */
    u32                 count;      /* number of parameters         */
    } VKN_shader_build_uniform_params_type;
    
typedef struct
    {
    VKN_shader_reflect_descriptor_binding_type
                        bindings[ VKN_SHADER_MAX_DESCRIPTOR_BINDING_CNT ];
                                    /* layout descriptor bindings   */
    u32                 count;      /* number of descriptor bindings*/
    } VKN_shader_build_layout_bindings_type;

typedef struct
    {
    VKN_shader_reflect_push_constant_type
                        constants[ VKN_SHADER_MAX_PUSH_CONSTANT_CNT ];
                                    /* layout push constants        */
    u32                 count;      /* number of push constants     */
    } VKN_shader_build_layout_push_constants_type;

typedef struct
    {
    u32                 code_sz;    /* shader code byte size        */
    const u32          *code;       /* shader bytecode              */
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    VkDevice            logical;    /* associated logical device    */
    VKN_shader_build_overrides_type
                        overrides;  /* reflection overrides         */
    VKN_shader_build_uniform_params_type
                        uniforms;   /* uniform shader parameter map */
    VKN_shader_build_layout_bindings_type
                        descriptor_bindings;
                                    /* layout descriptor bindings   */
    VKN_shader_build_layout_push_constants_type
                        push_constants;
                                    /* layout push constants        */
    bool                have_layout;/* were able to retrieve layout?*/
    } VKN_shader_build_state_type;

typedef struct _VKN_shader_build_type
    {
    VKN_shader_build_state_type
                        state;      /* builder state                */
    const VKN_shader_build_config_type
                       *config;     /* configuration interface      */
    } VKN_shader_build_type;

typedef struct _VKN_shader_parameter_mapping_type
    {
    VKN_shader_uniform_param_cls_type
                        cls;        /* param class tag              */
    union
        {
        struct
            {
            VKN_shader_param_vector_name_type
                        vector;     /* vector name                  */
            u8          width;      /* vector element width         */
            struct _VKN_shader_parameter_mapping_type
                       *next;       /* next vector                  */
            } vector;
        VKN_shader_param_image_name_type
                        image;      /* image name                   */
        };
    } VKN_shader_parameter_mapping_type;

typedef struct
    {
    u32                 set;        /* set number                   */
    u32                 binding;    /* binding index                */
    VkDescriptorType    kind;       /* descriptor type              */
    u32                 count;      /* array count                  */
    VKN_shader_parameter_mapping_type
                       *parameters; /* shader parameter mapping     */
    } VKN_shader_descriptor_binding_type;

typedef struct
    {
    VKN_shader_descriptor_binding_type
                        bindings[ VKN_SHADER_MAX_DESCRIPTOR_BINDING_CNT ];
                                    /* layout descriptor bindings   */
    u32                 count;      /* number of descriptor bindings*/
    } VKN_shader_layout_bindings_type;

typedef struct
    {
    u32                 offset;     /* data offset                  */
    u32                 size;       /* data size                    */
    VKN_shader_parameter_mapping_type
                       *parameters; /* shader parameter mapping     */
    } VKN_shader_layout_push_constant_type;

typedef struct
    {
    VKN_shader_layout_push_constant_type
                        constants[ VKN_SHADER_MAX_PUSH_CONSTANT_CNT ];
                                    /* layout push constants        */
    u32                 count;      /* number of push constants     */
    } VKN_shader_layout_push_constants_type;

typedef struct
    {
    VkDevice            logical;    /* associated logical device    */
    VkShaderModule      shader;     /* shader handle                */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    const char         *entry_point;/* entry point name string      */
                                    /* vector parameter pool        */
    VKN_shader_layout_bindings_type
                        descriptor_bindings;
                                    /* layout descriptor bindings   */
    VKN_shader_layout_push_constants_type
                        push_constants;
                                    /* layout push constants        */
    VKN_shader_parameter_mapping_type
                        param_pool[ VKN_SHADER_MAX_UNIFORM_PARAMS_CNT ];
                                    /* parameter mapping pool       */
    } VKN_shader_type;
