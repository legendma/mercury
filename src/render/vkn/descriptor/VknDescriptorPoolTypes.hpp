#pragma once

#include "Global.hpp"

#include "VknCommon.hpp"


#define VKN_DESCRIPTOR_POOL_CONFIG_API \
                                    const struct _VKN_descriptor_pool_build_config_type *
#define VKN_DESCRIPTOR_POOL_MAX_DESCRIPTOR_POOL_SIZE_CNT \
                                    ( 15 )
#define VKN_DESCRIPTOR_POOL_MAX_RATIO_CNT \
                                    VKN_DESCRIPTOR_POOL_MAX_DESCRIPTOR_POOL_SIZE_CNT

#define VKN_DESCRIPTOR_POOL_MAX_POOL_CNT \
                                    ( 15 )


typedef VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_build_add_default_ratios_proc_type
    (
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    );

typedef VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_build_add_ratio_proc_type
    (
    const VkDescriptorType
                        kind,       /* type of descriptor           */
    const float         ratio,      /* ratio type count to set count*/
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    );

typedef VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_build_set_allocation_callbacks_proc_type
    (
    VkAllocationCallbacks
                       *allocator,  /* custom allocator             */
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    );

typedef VKN_DESCRIPTOR_POOL_CONFIG_API VKN_descriptor_pool_build_set_max_pool_sets_proc_type
    (
    const u32           max_sets_per_pool,
                                    /* maximum number sets per pool */
    struct _VKN_descriptor_pool_build_type
                       *builder     /* descriptor pool builder      */
    );

typedef struct _VKN_descriptor_pool_build_config_type
    {
    VKN_descriptor_pool_build_add_default_ratios_proc_type
                       *add_default_ratios;
                                    /* add default ratio guesses    */
    VKN_descriptor_pool_build_add_ratio_proc_type
                       *add_ratio;  /* add reflection override      */
    VKN_descriptor_pool_build_set_allocation_callbacks_proc_type
                       *set_allocation_callbacks;
                                    /* set custom allocator         */
    VKN_descriptor_pool_build_set_max_pool_sets_proc_type
                       *set_max_pool_sets;
                                    /* set max set count per pool   */
    } VKN_descriptor_pool_build_config_type;

typedef struct
    {
    VkDescriptorType    kind;       /* type of descriptor           */
    float               ratio;      /* ratio of type_cnt to set_cnt */
    } VKN_descriptor_pool_build_ratio_type;

typedef struct
    {
    VKN_descriptor_pool_build_ratio_type
                        ratios[ VKN_DESCRIPTOR_POOL_MAX_RATIO_CNT ];
                                    /* descriptor type ratios       */
    u32                 count;      /* number of ratios             */
    } VKN_descriptor_pool_build_ratios_type;

typedef struct
    {
    VkAllocationCallbacks
                       *allocator;  /* custom allocator             */
    u32                 max_set_count;
                                    /* maximum set count per pool   */
    VkDevice            logical;    /* associated logical device    */
    VKN_descriptor_pool_build_ratios_type
                        ratios;     /* descriptor count ratios      */
    } VKN_descriptor_pool_build_state_type;

typedef struct _VKN_descriptor_pool_build_type
    {
    VKN_descriptor_pool_build_state_type
                        state;      /* builder state                */
    const VKN_descriptor_pool_build_config_type
                       *config;     /* configuration interface      */
    } VKN_descriptor_pool_build_type;
    
typedef VkDescriptorSet VKN_descriptor_pool_allocate_set_proc_type
    (
    const VkDescriptorSetLayout
                        layout,     /* desired layout of set        */
    struct _VKN_descriptor_pool_type
                       *pool        /* descriptor pool              */
    );

typedef void VKN_descriptor_pool_begin_frame_proc_type
    (
    struct _VKN_descriptor_pool_type
                       *pool        /* descriptor pool to wipe      */
    );

typedef struct
    {
    VKN_descriptor_pool_allocate_set_proc_type
                       *allocate_set;
                                    /* allocate set from thsi pool  */
    VKN_descriptor_pool_begin_frame_proc_type
                       *begin_frame;/* indicate this frame starts   */
    } VKN_descriptor_pool_api_type;

typedef struct
    {
    VkDescriptorPoolSize            /* descriptor type cnts per pool*/
                        sizes[ VKN_DESCRIPTOR_POOL_MAX_DESCRIPTOR_POOL_SIZE_CNT ];
    u32                 count;      /* number of pool sizes         */
    } VKN_descriptor_pool_sizes_type;

typedef struct
    {
    VkDescriptorPool    pools[ VKN_DESCRIPTOR_POOL_MAX_POOL_CNT ];
                                    /* descriptor type cnts per pool*/
    u32                 count;      /* number of pool sizes         */
    } VKN_descriptor_pool_pools_type;

typedef struct
    {
    u32                 max_set_count;
                                    /* maximum set count per pool   */
    const VkAllocationCallbacks
                       *allocator;  /* allocation callbacks         */
    VkDevice            logical;    /* associated logical device    */
    VkDescriptorPool    active;     /* active pool                  */
    VKN_descriptor_pool_sizes_type
                        sizes;      /* descriptor type cnts per pool*/
    VKN_descriptor_pool_pools_type
                        clean;      /* clean pools                  */
    VKN_descriptor_pool_pools_type
                        dirty;      /* dirty pools                  */
    } VKN_descriptor_pool_state_type;

typedef struct _VKN_descriptor_pool_type
    {
    const VKN_descriptor_pool_api_type
                       *i;          /* resource staging interface   */
    VKN_descriptor_pool_state_type
                        state;      /* private state                */
    } VKN_descriptor_pool_type;
