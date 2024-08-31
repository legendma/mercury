//#include <cassert>
//#include <cstdio>

//#include <cstring>
//#include <winbase.h>
//#include <winuser.h>
//
//#include "AssetFile.hpp"
//#include "Command.hpp"
//#include "ComUtilities.hpp"
//#include "HashMap.hpp"
//#include "NonOwningGroup.hpp"
#include "Global.hpp"
#include "Render.hpp"
#include "ShaderGen.hpp"
//#include "RenderEngine.hpp"
//#include "RenderInitializers.hpp"
//#include "RenderPipelines.hpp"
//#include "RenderScene.hpp"
//#include "RenderShaders.hpp"
//#include "ResourceLoader.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"
#include "Vkn.hpp"


using namespace ECS;

#define FRAME_RELEASER_VARS_CNT     ( 1000 )
#define DEFAULT_SWAP_CHAIN_WIDTH    ( 1024 )
#define DEFAULT_SWAP_CHAIN_HEIGHT   ( 768 )
#define FRAME_ARENA_SZ              ( 2ull * 1024ull * 1024ull )
#define FRAME_CNT                   ( 2 )
#define PERMANENT_ARENA_SZ          ( 1ull * 1024ull * 1024ull + FRAME_CNT * FRAME_ARENA_SZ )
#define VERTEX_MAX_ATTRIBUTES_CNT   ( 20 )
#define VERTEX_MAX_BINDINGS_CNT     ( 5 )
#define SHADER_NAME_NO_SHADER       MERCURY_SHADER_NAME_CNT
#define MASTER_TRANSITIONER_MAX_RESOURCE_CNT \
                                    ( 100 )
#define MASTER_TRANSITIONER_MAX_SUBRESOURCE_CNT \
                                    ( 200 )
#define MASTER_TRANSITIONER_MAX_UNREGISTER_CNT \
                                    ( 30 )

typedef enum
    {
    VERTEX_TYPE_POS3_TEX2,
    VERTEX_TYPE_POS3_TEX2_NML3,
    /* count */
    VERTEX_TYPE_CNT
    } VertexType;

typedef enum
    {
    PROGRAM_NAME_SIMPLE,
    PROGRAM_NAME_TEST,
    PROGRAM_NAME_TEXTURED,
    PROGRAM_NAME_TEXTURED_LIT
    } ProgramName;

typedef enum
    {
    ATTACH_TYPE_COLOR,
    ATTACH_TYPE_DEPTH,
    ATTACH_TYPE_STENCIL
    } AttachType;

struct
    {
    ProgramName         program;
    const char         *name;
    mercury_shader_name_type
                        shaders[ VKN_SHADER_GFX_STAGE_CNT ];
    } static const EFFECT_DEFINITIONS[]  =
    {
    { PROGRAM_NAME_SIMPLE,       "simple effect",            { MERCURY_SHADER_NAME_VERT_SIMPLE,    SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, MERCURY_SHADER_NAME_FRAG_SIMPLE         } },
    { PROGRAM_NAME_TEST,         "test effect",              { MERCURY_SHADER_NAME_VERT_TEST,      SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, MERCURY_SHADER_NAME_FRAG_TEST           } },
    { PROGRAM_NAME_TEXTURED,     "textured effect",          { MERCURY_SHADER_NAME_VERT_POS2_TEX2, SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, MERCURY_SHADER_NAME_FRAG_TEX1_MOD_COLOR } },
    { PROGRAM_NAME_TEXTURED_LIT, "pos3 textured-lit effect", { MERCURY_SHADER_NAME_VERT_DIFFUSE,   SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, SHADER_NAME_NO_SHADER, MERCURY_SHADER_NAME_FRAG_DIFFUSE        } }
    };

typedef struct
    {
    VKN_arena_type      arena;
    VkSemaphore         acquire;
    VkSemaphore         render;
    VkFence             fence;
    VkCommandBuffer     begin_commands;
    VkCommandBuffer     end_commands;
    VkCommandBuffer     end_prepend_commands;
    VKN_releaser_type   releaser;
    VKN_releaser_variant_type
                        release_vars[ FRAME_RELEASER_VARS_CNT ];
    } Frame;

typedef struct
    {
    VKN_vertex_type     types[ VERTEX_TYPE_CNT ];
    VKN_vertex_attribute_type
                        attributes[ VERTEX_MAX_ATTRIBUTES_CNT ];
    VKN_vertex_binding_type
                        bindings[ VERTEX_MAX_BINDINGS_CNT ];
    } VertexDefines;

typedef struct
    {
    VKN_shader_type     shaders[ MERCURY_SHADER_NAME_CNT ];
    VKN_effect_type     effects[ cnt_of_array( EFFECT_DEFINITIONS ) ];
    } Shaders;

typedef struct
    {
    u32                 next_image_uid;
    VKN_transitioner_type
                        obj;
    VKN_transitioner_resource_type
                        resources[ MASTER_TRANSITIONER_MAX_RESOURCE_CNT ];
    VKN_transitioner_subresource_type
                        subresources[ MASTER_TRANSITIONER_MAX_SUBRESOURCE_CNT ];
    VKN_transitioner_unregister_type
                        unregisters[ MASTER_TRANSITIONER_MAX_UNREGISTER_CNT ];
    } MasterTransitioner;

typedef struct
    {
    VKN_image_build_type
                        image;
    VKN_buffer_index_build_type
                        index_buffer;
    VKN_buffer_uniform_build_type
                        uniform_buffer;
    VKN_buffer_vertex_build_type
                        vertex_buffer;
    VKN_buffer_vertex_dynamic_build_type
                        vertex_buffer_dynamic;
    VKN_descriptor_pool_build_type
                        descriptor_pool;
    VKN_swap_chain_build_type
                        swap_chain;
    } Builders;

typedef struct
    {
    bool                is_allocated : 1;
    bool                is_created : 1;
    u32                 uid;
    VKN_image_type      obj;
    } Image;

typedef struct
    {
    Image              *color_image;
    Image              *depth_image;
    Image              *stencil_image;
    VkRenderingInfo     render_info;/* begin rendering info         */
    VkRenderingAttachmentInfo       /* color buffer attachment      */
                        attach_color;
    VkRenderingAttachmentInfo       /* depth buffer attachment      */
                        attach_depth;
    VkRenderingAttachmentInfo       /* stencil buffer attachment    */
                        attach_stencil;
    } FrameBuffer;

typedef struct
    {
    u32                 image_index;
    VkExtent2D          desired_extent;
    Image               color_images[ VKN_SWAP_CHAIN_MAX_IMAGE_CNT ];
    VKN_swap_chain_type obj;
    Image               depth_stencil;
    FrameBuffer         frame_buffer;
    } SwapChain;

typedef struct
    {
    Image              *image_frees;
    Image              *image_shutdown;
    } Objects;

typedef union
    {
    VKN_thread_mutex_type
                        a[ 1 ];
    struct
        {
        VKN_thread_mutex_type
                        memory;
        } n;
    } Access;

typedef struct
    {
    u8                  frame_index;
    VkInstance          instance;
    VkSurfaceKHR        surface;
    VkCommandPool       command_pool;
    VKN_physical_device_type
                        physical;
    VKN_logical_device_type
                        logical;
    VKN_memory_type     memory;
    VKN_staging_type    staging;
    VKN_arena_type      permanent_arena;
    VKN_arena_word_type arena_memory[ PERMANENT_ARENA_SZ / sizeof( VKN_arena_word_type ) ];
    Frame               frames[ FRAME_CNT ];
    Frame              *current_frame;
    VertexDefines       vertex_defs;
    Shaders             shaders;
    Access              access;
    Builders            builders;
    MasterTransitioner  transitioner;
    SwapChain           swap_chain;
    Objects             objects;
    } RenderEngine;


/*******************************************************************
*
*   AsRenderEngine()
*
*   DESCRIPTION:
*       Get the singleton engine state.
*
*******************************************************************/

static inline RenderEngine * AsRenderEngine( Universe *universe )
{
SingletonRenderComponent *component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
return( (RenderEngine*)component->ptr );

} /* AsRenderEngine() */


///*******************************************************************
//*
//*   AdvanceFrame()
//*
//*   DESCRIPTION:
//*       Advance the render engine one frame.
//*
//*******************************************************************/
//
//static inline void AdvanceFrame( Engine::Engine *engine )
//{
//++engine->current_frame %= cnt_of_array( engine->frames );
//
//} /* AdvanceFrame() */
//
//
///*******************************************************************
//*
//*   CalculateConstantBufferSize()
//*
//*   DESCRIPTION:
//*       The constant buffer must be a multiple of 256 bytes.
//*
//*******************************************************************/
//
//static inline uint32_t CalculateConstantBufferSize( const uint32_t size )
//{
//return( ( size + 255 ) & 0xff );
//
//} /* CalculateConstantBufferSize() */
//
//
///*******************************************************************
//*
//*   GetCurrentBackbuffer()
//*
//*   DESCRIPTION:
//*       Get the current backbuffer texture.
//*
//*******************************************************************/
//
//static inline Engine::Texture * GetCurrentBackbuffer( Engine::Engine *engine )
//{
//return( &engine->surfaces.backbuffers[ engine->surfaces.backbuffer_current ] );
//
//} /* GetCurrentBackbuffer() */


/*******************************************************************
*
*   LockMutex()
*
*******************************************************************/

#define LockMutex( _mutex ) \
    (_mutex)->i->lock( _mutex )

//
//
///*******************************************************************
//*
//*   SweepFrameTrash()
//*
//*   DESCRIPTION:
//*       Cleanup the trash in the given frame.  Frame must not be
//*       in-flight.
//*
//*******************************************************************/
//
//static inline void SweepFrameTrash( Engine::Frame *frame )
//{
//for( uint32_t i = 0; i < frame->trash_can_count; i++ )
//    {
//    ComSafeRelease( &frame->trash_can[ i ] );
//    }
//
//frame->trash_can_count = 0;
//
//} /* SweepFrameTrash() */


/*******************************************************************
*
*   UnlockMutex()
*
*******************************************************************/

#define UnlockMutex( _mutex ) \
    (_mutex)->i->unlock( _mutex )


static void AttachImage( AttachType attach, Image *image, FrameBuffer *buffer );
static bool BeginFrame( RenderEngine *engine );
//static void             ClearBackbuffer( const Color4f clear_color, Engine::Engine *engine );
//static bool             CreateCommandObjects( Engine::Engine *engine );
//static bool             CreateDefaultBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
//static bool             CreateDepthStencil( Engine::Engine *engine );
//static bool             CreateDescriptorHeaps( Engine::Engine *engine );
//static bool             CreateDevice( Engine::Engine *engine );
//static bool             CreateRenderTargetViews( Engine::Engine *engine );
//static bool             CreateSwapChain( Engine::Engine *engine );
static bool CreateSwapChain( RenderEngine *engine );
//static bool             CreateUploadBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
//static void             DestroyScenes( Engine::Engine *engine, Universe *universe );
static void DestroySwapChain( RenderEngine *engine );
//static void             DrawScenes( Engine::Engine *engine, Universe *universe );
static bool EndFrame( RenderEngine *engine );
//static void             ExecuteCommandLists( Engine::Engine *engine );
//static bool             FlushCommandQueue( Engine::Engine *engine );
//static bool             GetWindowExtent( HWND window, UINT *width, UINT *height );
//static bool             InitDirectX( Engine::Engine *engine );
//static bool             InitFrames( Engine::Engine *engine );
//static bool             InitPipelines( Engine::Engine *engine );
//static UniverseComponentOnAttachProc
//                        OnSceneAttach;
//static UniverseComponentOnAttachProc
//                        OnSceneRemove;
//static void             Reset( Engine::Engine *engine, Universe *universe );
static bool ResizeSwapChain( RenderEngine *engine );
//static bool             ScheduleBufferUpload( const void *data, const uint32_t data_sz, ID3D12Device *device, ID3D12GraphicsCommandList *gfx, ID3D12Resource *upload, ID3D12Resource *gpu );
//static void             SetViewport( const Float2 top_left, const Float2 extent, Engine::Engine *engine );
//static bool             WaitForFrameToFinish( uint64_t frame_num, Engine::Engine *engine );


///*******************************************************************
//*
//*   DescriptorHeap_AllocateBatch()
//*
//*   DESCRIPTION:
//*       Allocate a number of descriptor handles from the given heap.
//*
//*******************************************************************/
//
//Engine::DescriptorHandle Engine::DescriptorHeap_Allocate( Render::Engine::DescriptorHeap *heap )
//{
//Render::Engine::DescriptorHandle ret = {};
//DescriptorHeap_AllocateBatch( 1, 1, &ret, heap );
//
//return( ret );
//
//} /* DescriptorHeap_Allocate() */
//
//
///*******************************************************************
//*
//*   DescriptorHeap_AllocateBatch()
//*
//*   DESCRIPTION:
//*       Allocate a number of descriptor handles from the given heap.
//*
//*******************************************************************/
//
//bool Engine::DescriptorHeap_AllocateBatch( const uint32_t allocate_cnt, uint32_t out_capacity, Render::Engine::DescriptorHandle *out, Render::Engine::DescriptorHeap *heap )
//{
//uint32_t remaining = heap->capacity - heap->head;
//if( remaining < allocate_cnt )
//    {
//    if( heap->is_ring )
//        {
//        /* Ring Allocator */
//        if( heap->frames_since_wrap < RENDER_ENGINE_FRAME_COUNT )
//            {
//            assert( false );
//            return( false );
//            }
//
//        heap->frames_since_wrap = 0;
//        heap->head = 0;
//        }
//    else
//        {
//        /* Linear Allocator */
//        /* Asking count over capacity, and wraps not allowed */
//        assert( false );
//        return( false );
//        }
//
//    }
//
//for( uint32_t i = 0; i < allocate_cnt; i++ )
//    {
//    out[ i ].cpu_hndl.ptr = heap->cpu_start.ptr + ( i + heap->head ) * heap->descriptor_size;
//    out[ i ].gpu_hndl.ptr = heap->gpu_start.ptr + ( i + heap->head ) * heap->descriptor_size;
//    }
//
//heap->head += allocate_cnt;
//
//return( true );
//
//} /* DescriptorHeap_AllocateBatch() */
//
//
///*******************************************************************
//*
//*   DescriptorHeap_Create()
//*
//*   DESCRIPTION:
//*       Create a descriptor heap by allocating it within the given
//*       device.
//*
//*******************************************************************/
//
//bool Engine::DescriptorHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool is_ring, const uint32_t capacity, ID3D12Device *device, Render::Engine::DescriptorHeap *heap )
//{
//*heap = {};
//heap->capacity          = capacity;
//heap->type              = type;
//heap->is_ring           = is_ring;
//heap->is_shader_visible = ( type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
//                         || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
//heap->descriptor_size   = device->GetDescriptorHandleIncrementSize( heap->type );
//
//D3D12_DESCRIPTOR_HEAP_DESC desc = Initializers::GetHeapDescriptor( capacity, heap->is_shader_visible, type );
//if( FAILED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &heap->heap ) ) ) )
//    {
//    assert( false );
//    return( false );
//    }
//
//heap->cpu_start = heap->heap->GetCPUDescriptorHandleForHeapStart();
//if( heap->is_shader_visible )
//    {
//    heap->gpu_start = heap->heap->GetGPUDescriptorHandleForHeapStart();
//    }
//
//return( true );
//
//} /* DescriptorHeap_Create() */
//
//
///*******************************************************************
//*
//*   DescriptorHeap_Destroy()
//*
//*   DESCRIPTION:
//*       Destroy the given descriptor heap.
//*
//*******************************************************************/
//
//void Engine::DescriptorHeap_Destroy( Render::Engine::Engine *engine, Render::Engine::DescriptorHeap *heap )
//{
//if( engine )
//    {
//    Engine_TossTrash( &heap->heap, engine );
//    }
//else
//    {
//    ComSafeRelease( &heap->heap );
//    }
//
//*heap = {};
//
//} /* DescriptorHeap_Destroy() */
//
//
///*******************************************************************
//*
//*   DescriptorHeap_Reset()
//*
//*   DESCRIPTION:
//*       Reset all allocations of the given descriptor heap.
//*
//*******************************************************************/
//
//void Engine::DescriptorHeap_Reset( Render::Engine::DescriptorHeap *heap )
//{
//heap->head = 0;
//
//} /* DescriptorHeap_Reset() */
//
//
///*******************************************************************
//*
//*   Engine_AddTrashToCurrentFrame()
//*
//*   DESCRIPTION:
//*       Add a piece of trash to the current frame's trash can.
//*
//*******************************************************************/
//
//void Engine::Engine_AddTrashToCurrentFrame( IUnknown **trash, Render::Engine::Engine *engine )
//{
//Frame *frame = Engine_CurrentFrame( engine );
//hard_assert( frame->trash_can_count < cnt_of_array( frame->trash_can ) );
//frame->trash_can[ frame->trash_can_count++ ] = *trash;
//*trash = NULL;
//
//} /* Engine_AddTrashToCurrentFrame() */

//
///*******************************************************************
//*
//*   Engine_ClearDepthStencil()
//*
//*   DESCRIPTION:
//*       Clear the depth/stencil buffer to the given depth and
//*       stencil values.
//*
//*******************************************************************/
//
//void Engine::Engine_ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Render::Engine::Engine *engine )
//{
//engine->commands.gfx->ClearDepthStencilView( engine->surfaces.depth_stencil.handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0, NULL );
//
//} /* Engine_ClearDepthStencil() */
//
//
///*******************************************************************
//*
//*   Engine_CurrentFrame()
//*
//*   DESCRIPTION:
//*       Get the frame that is currently being built.
//*
//*******************************************************************/
//
//Engine::Frame * Engine::Engine_CurrentFrame( Engine *engine )
//{
//return( &engine->frames[ engine->current_frame ] );
//
//} /* Engine_CurrentFrame() */


/*******************************************************************
*
*   Render_ChangeResolutions()
*
*   DESCRIPTION:
*       Change the graphics screen resolution.
*
*******************************************************************/

void Render_ChangeResolutions( const uint16_t width, const uint16_t height, Universe *universe )
{
RenderEngine *engine = AsRenderEngine( universe );
engine->swap_chain.desired_extent.width  = width;
engine->swap_chain.desired_extent.height = height;

} /* Render_ChangeResolutions() */


/*******************************************************************
*
*   Render_CreateVulkanInstance()
*
*   DESCRIPTION:
*       Create the Vulkan instance.
*
*******************************************************************/

void Render_CreateVulkanInstance( VKN_instance_type *out )
{
bool use_validation = false;
#if defined( _DEBUG )
use_validation = true;
#endif

VKN_instance_build_type builder;
VKN_instance_init_builder( false, &builder );

builder.config->set_app_name( "Mercury", &builder )->
                enable_validation_layers( use_validation, &builder );

if( use_validation )
    {
    builder.config->use_debug_messenger( &builder );
    }

do_debug_assert( VKN_instance_create( &builder, out ) );

} /* Render_CreateVulkanInstance() */


/*******************************************************************
*
*   Render_Destroy()
*
*   DESCRIPTION:
*       Destroy the Render engine and free its resources.
*
*******************************************************************/

void Render_Destroy( Universe *universe )
{
RenderEngine *engine = AsRenderEngine( universe );

if( engine->logical.logical == VK_NULL_HANDLE )
    {
    return;
    }

for( int i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];
    VKN_release_command_buffer( engine->logical.logical, engine->command_pool, &frame->end_prepend_commands );
    VKN_release_command_buffer( engine->logical.logical, engine->command_pool, &frame->end_commands );
    VKN_release_command_buffer( engine->logical.logical, engine->command_pool, &frame->begin_commands );
    VKN_release_fence( engine->logical.logical, NULL, &frame->fence );
    VKN_release_semaphore( engine->logical.logical, NULL, &frame->acquire );
    VKN_release_semaphore( engine->logical.logical, NULL, &frame->render );
    }

VKN_release_command_pool( engine->logical.logical, NULL, &engine->command_pool );
DestroySwapChain( engine );
VKN_staging_destroy( NULL, &engine->staging );
VKN_memory_destroy( NULL, &engine->memory );

for( int i = 0; i < cnt_of_array( engine->shaders.effects ); i++ )
    {
    VKN_effect_destroy( NULL, &engine->shaders.effects[ i ] );
    }

for( int i = 0; i < cnt_of_array( engine->shaders.shaders ); i++ )
    {
    VKN_shader_destroy( NULL, &engine->shaders.shaders[ i ] );
    }

VKN_logical_device_destroy( NULL, &engine->logical );
for( int i = 0; i < cnt_of_array( engine->access.a ); i++ )
    {
    VKN_thread_mutex_destroy( &engine->access.a[ i ] );
    }

free( engine );
SingletonRenderComponent *component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = NULL;

} /* Render_Destroy() */


/*******************************************************************
*
*   Render_DoFrame()
*
*   DESCRIPTION:
*       Advance the engine one frame.
*
*******************************************************************/

void Render_DoFrame( float frame_delta, Universe *universe )
{
RenderEngine *engine = AsRenderEngine( universe );
if( !BeginFrame( engine ) )
    {
    debug_assert_always();
    return;
    }

//DrawScenes( engine, universe );

if( !EndFrame( engine ) )
    {
    debug_assert_always();
    return;
    }

} /* Render_DoFrame() */


///*******************************************************************
//*
//*   Render_FreeModel()
//*
//*   DESCRIPTION:
//*       Free the model attached to the entity.
//*       Returns TRUE if a model existed and was free.  FALSE
//*       otherwise.
//*
//*******************************************************************/
//
//bool Render_FreeModel( const EntityId entity, Universe *universe )
//{
//Engine::Engine *engine = (Engine::Engine *)AsRenderEngine( universe );
//ModelComponent *existing = (ModelComponent*)Universe_TryGetComponent( entity, COMPONENT_MODEL, universe );
//if( !existing )
//    {
//    return( false );
//    }
//    
////Model *model = (Model*)HashMap_At( existing->asset_id, &engine->models.map );
//Universe_RemoveComponentFromEntity( entity, COMPONENT_MODEL, universe );
////if( !model )
////    {
////    /* model should have existed */
////    assert( false );
////    return( true );
////    }
////
////assert( model->ref_count );
////model->ref_count--;
//
//return( true );
//
//} /* Render_FreeModel() */


/*******************************************************************
*
*   Render_Init()
*
*   DESCRIPTION:
*       Initialize the graphics engine.
*
*******************************************************************/

bool Render_Init( VkSurfaceKHR surface, VkInstance instance, ECS::Universe* universe )
{    
SingletonRenderComponent* component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = malloc( sizeof( RenderEngine ) );
if( !component->ptr )
    {
    return( false );
    }

RenderEngine *engine = (RenderEngine*)component->ptr;
clr_struct( engine );
engine->instance                    = instance;
engine->surface                     = surface;
engine->transitioner.next_image_uid = 1;
engine->current_frame               = engine->frames;

VKN_arena_create( sizeof( engine->arena_memory ), engine->arena_memory, &engine->permanent_arena );
for( int i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    VKN_arena_create( FRAME_ARENA_SZ, (VKN_arena_word_type*)VKN_arena_allocate( FRAME_ARENA_SZ, &engine->permanent_arena ), &engine->frames[ i ].arena );
    }

VKN_arena_type *scratch = &engine->frames->arena;

/* access */
for( int i = 0; i < cnt_of_array( engine->access.a ); i++ )
    {
    VKN_thread_mutex_create( &engine->access.a[ i ] );
    }

/* physical device */
VKN_physical_device_build_type *physical_build = VKN_arena_allocate_struct( VKN_physical_device_build_type, scratch );
VKN_return_bfail( physical_build );
VKN_features_type *features = VKN_arena_allocate_struct( VKN_features_type, scratch );
VKN_return_bfail( features );

clr_struct( features );
features->v1_3.dynamicRendering = VK_TRUE;
features->extended_dynamic_state.extendedDynamicState = VK_TRUE;

VKN_physical_device_init_builder( instance, surface, VKN_MIN_VERSION, physical_build )->
    set_required_device_class( VKN_PHYSICAL_DEVICE_BUILD_DEVICE_CLASS_HARDWARE, physical_build )->
    add_extension( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, physical_build )->
    add_extension( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, physical_build )->
    set_required_features( features, physical_build );

VKN_return_bfail( VKN_physical_device_create( physical_build, &engine->physical ) );
physical_build = nullptr;
VKN_arena_rewind( scratch );

/* logical device */
VKN_logical_device_build_type *logical_build = VKN_arena_allocate_struct( VKN_logical_device_build_type, scratch );
VKN_return_bfail( logical_build );

VKN_logical_device_init_builder( engine->physical.physical_device,
                                 surface,
                                 engine->physical.extensions.names,
                                 engine->physical.extensions.count,
                                 &engine->physical.features,
                                 engine->physical.queue_families.queue_families,
                                 engine->physical.queue_families.count,
                                 logical_build )->
    require_graphics_queue( FALSE, FALSE, logical_build );

VKN_return_bfail( VKN_logical_device_create( logical_build, &engine->logical ) );
logical_build = nullptr;
VKN_arena_rewind( scratch );

/* vertex types */
VKN_vertex_build_type *vertex_build = VKN_arena_allocate_struct( VKN_vertex_build_type, scratch );
VKN_return_bfail( vertex_build );

VKN_vertex_init_builder( cnt_of_array( engine->vertex_defs.attributes ),
                         cnt_of_array( engine->vertex_defs.bindings ),
                         engine->vertex_defs.attributes,
                         engine->vertex_defs.bindings,
                         vertex_build );

for( int i = 0; i < cnt_of_array( engine->vertex_defs.types ); i++ )
    {
    switch( i )
        {
        case VERTEX_TYPE_POS3_TEX2:
            vertex_build->config->add_binding( 0, 5 * sizeof( float ), VK_VERTEX_INPUT_RATE_VERTEX, vertex_build )->
                                  add_attribute( 0, VK_FORMAT_R32G32B32_SFLOAT, 0 * sizeof( float ), vertex_build )->
                                  add_attribute( 1, VK_FORMAT_R32G32_SFLOAT,    3 * sizeof( float ), vertex_build );
            break;

        case VERTEX_TYPE_POS3_TEX2_NML3:
            vertex_build->config->add_binding( 0, 8 * sizeof( float ), VK_VERTEX_INPUT_RATE_VERTEX, vertex_build )->
                                  add_attribute( 0, VK_FORMAT_R32G32B32_SFLOAT,    0 * sizeof( float ), vertex_build )->
                                  add_attribute( 1, VK_FORMAT_R32G32_SFLOAT,       3 * sizeof( float ), vertex_build )->
                                  add_attribute( 2, VK_FORMAT_R32G32B32A32_SFLOAT, 5 * sizeof( float ), vertex_build );
            break;

        default:
            debug_assert_always();
            return( false );
        }

    engine->vertex_defs.types[ i ] = vertex_build->config->build( vertex_build );
    }

vertex_build = nullptr;
VKN_arena_rewind( scratch );

/* shaders */
VKN_shader_build_type *shader_build = VKN_arena_allocate_struct( VKN_shader_build_type, scratch );
VKN_return_bfail( shader_build );

VKN_shader_init_builder( engine->logical.logical, shader_build );
for( int i = 0; i < cnt_of_array( MERCURY_SHADER_TABLE ); i++ )
    {
    const mercury_shader_element_type *shader = &MERCURY_SHADER_TABLE[ i ];
    shader_build->config->reset( shader_build )->
                          set_code( shader->bytecode, shader->size, shader_build );

    for( u32 j = 0; j < shader->uniform_cnt; j++ )
        {
        const mercury_shader_uniform_type *uniform = &shader->uniforms[ j ];
        if( uniform->image )
            {
            shader_build->config->map_uniform_image( uniform->str_name, *uniform->image, shader_build );
            }

        for( u32 k = 0; k < uniform->vector_cnt; k++ )
            {
            const mercury_shader_uniform_vector_type *uniform_vector = &uniform->vectors[ k ];
            shader_build->config->map_uniform_vector( uniform->str_name, uniform_vector->name, uniform_vector->num_floats, shader_build );
            }
        }

    VKN_return_bfail( VKN_shader_create( shader_build, &engine->shaders.shaders[ i ] ) );
    }

shader_build = nullptr;
VKN_arena_rewind( scratch );

/* effects */
VKN_effect_build_type *effect_build = VKN_arena_allocate_struct( VKN_effect_build_type, scratch );
VKN_return_bfail( effect_build );

VKN_effect_init_builder( engine->logical.logical, effect_build );
for( int i = 0; i < cnt_of_array( engine->shaders.effects ); i++ )
    {
    effect_build->config->reset( effect_build );
    for( int j = 0; j < cnt_of_array( EFFECT_DEFINITIONS[ i ].shaders ); j++ )
        {
        if( EFFECT_DEFINITIONS[ i ].shaders[ j ] == SHADER_NAME_NO_SHADER )
            {
            continue;
            }

        effect_build->config->add_stage( (VKN_shader_gfx_stage_type)j, &engine->shaders.shaders[ EFFECT_DEFINITIONS[ i ].shaders[ j ] ], effect_build );
        }

    effect_build->config->finalize_stages( effect_build );

    VKN_return_bfail( VKN_effect_create( EFFECT_DEFINITIONS[ i ].name, effect_build, &engine->shaders.effects[ i ] ) );
    }

effect_build = nullptr;
VKN_arena_rewind( scratch );

/* memory */
VKN_memory_build_type *memory_build = VKN_arena_allocate_struct( VKN_memory_build_type, scratch );
VKN_return_bfail( memory_build );

VKN_memory_init_builder( instance,
                         engine->logical.physical,
                         engine->logical.logical,
                         memory_build );

VKN_return_bfail( VKN_memory_create( memory_build, &engine->memory ) );

memory_build = nullptr;
VKN_arena_rewind( scratch );

/* staging */
VKN_staging_build_type *staging_build = VKN_arena_allocate_struct( VKN_staging_build_type, scratch );
VKN_return_bfail( staging_build );

VKN_staging_init_builder( engine->logical.logical,
                          engine->logical.graphics.queue,
                          engine->logical.graphics.family,
                          &engine->memory,
                          staging_build );

VKN_staging_create( staging_build, &engine->staging );

staging_build = nullptr;
VKN_arena_rewind( scratch );

/* builders */
VKN_image_init_builder( engine->logical.logical,
                        engine->logical.physical,
                        &engine->physical.props,
                        &engine->memory,
                        &engine->builders.image );

VKN_buffer_index_init_builder( engine->logical.logical,
                               &engine->physical.props,
                               &engine->memory,
                               &engine->builders.index_buffer );

VKN_buffer_vertex_init_builder( engine->logical.logical,
                                &engine->physical.props,
                                &engine->memory,
                                &engine->builders.vertex_buffer );

VKN_buffer_uniform_init_builder( engine->logical.logical,
                                 &engine->physical.props,
                                 &engine->memory,
                                 &engine->builders.uniform_buffer );

VKN_buffer_vertex_dynamic_init_builder( engine->logical.logical,
                                        &engine->physical.props,
                                        &engine->memory,
                                        &engine->builders.vertex_buffer_dynamic );

VKN_descriptor_pool_init_builder( engine->logical.logical, &engine->builders.descriptor_pool )->
    add_default_ratios( &engine->builders.descriptor_pool );

VKN_swap_chain_init_builder( engine->physical.physical_device, engine->logical.logical, &engine->builders.swap_chain );

/* transitioner */
VKN_transitioner_create( VKN_TRANSITIONER_IS_MASTER,
                         0,
                         cnt_of_array( engine->transitioner.resources ),
                         cnt_of_array( engine->transitioner.subresources ),
                         cnt_of_array( engine->transitioner.unregisters ),
                         nullptr,
                         engine->transitioner.resources,
                         engine->transitioner.subresources,
                         engine->transitioner.unregisters,
                         &engine->transitioner.obj );

/* swap-chain */
engine->swap_chain.desired_extent.width  = DEFAULT_SWAP_CHAIN_WIDTH;
engine->swap_chain.desired_extent.height = DEFAULT_SWAP_CHAIN_HEIGHT;
VKN_return_bfail( CreateSwapChain( engine ) );

/* command pool */
VkCommandPoolCreateInfo ci_command_pool = {};
ci_command_pool.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
ci_command_pool.queueFamilyIndex = engine->logical.graphics.family;
ci_command_pool.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

VKN_return_fail( vkCreateCommandPool( engine->logical.logical, &ci_command_pool, NULL, &engine->command_pool ) );

/* frames */
VkSemaphoreCreateInfo ci_semaphore = {};
ci_semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

VkFenceCreateInfo ci_fence = {};
ci_fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
ci_fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

VkCommandBufferAllocateInfo ai_command_buffer = {};
ai_command_buffer.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
ai_command_buffer.commandPool        = engine->command_pool;
ai_command_buffer.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
ai_command_buffer.commandBufferCount = 1;

for( int i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];

    VKN_releaser_create( cnt_of_array( frame->release_vars ), frame->release_vars, &frame->releaser );

    VKN_return_fail( vkCreateSemaphore( engine->logical.logical, &ci_semaphore, NULL, &frame->acquire ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->acquire, VK_OBJECT_TYPE_SEMAPHORE, "frame[%d].acquire", i ) );
    
    VKN_return_fail( vkCreateSemaphore( engine->logical.logical, &ci_semaphore, NULL, &frame->render ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->render, VK_OBJECT_TYPE_SEMAPHORE, "frame[%d].render", i ) );

    VKN_return_fail( vkCreateFence( engine->logical.logical, &ci_fence, NULL, &frame->fence ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->fence, VK_OBJECT_TYPE_FENCE, "frame[%d].fence", i ) );

    VKN_return_fail( vkAllocateCommandBuffers( engine->logical.logical, &ai_command_buffer, &frame->begin_commands ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->begin_commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "frame[%d].begin_commands", i ) );

    VKN_return_fail( vkAllocateCommandBuffers( engine->logical.logical, &ai_command_buffer, &frame->end_commands ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->end_commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "frame[%d].end_commands", i ) );

    VKN_return_fail( vkAllocateCommandBuffers( engine->logical.logical, &ai_command_buffer, &frame->end_prepend_commands ) );
    VKN_return_fail( VKN_name_object( engine->logical.logical, frame->end_prepend_commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "frame[%d].end_prepend_commands", i ) );
    }

//Universe_RegisterComponentLifetime( COMPONENT_SCENE, OnSceneAttach, OnSceneRemove, universe );

return( true );

} /* Render_Init() */


///*******************************************************************
//*
//*   Render_LoadModel()
//*
//*   DESCRIPTION:
//*       Load the given model by asset name.
//*
//*******************************************************************/
//
//ModelComponent * Render_LoadModel( const char *asset_name, const EntityId entity, Universe *universe )
//{
//(void)Render_FreeModel( entity, universe );
//ModelComponent *instance = (ModelComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_MODEL, universe );
//instance->asset_name = AssetFile_CopyNameString( asset_name );
//
///* TODO <MPA> TESTING REMOVE */
//Engine::Engine *engine = (Engine::Engine*)AsRenderEngine( universe );
//Shaders::ShaderModule *shader = Shaders::ShaderCache_GetShader( "shader_vs_mvp", &engine->pipelines.shader_cache );
///* TODO <MPA> TESTING REMOVE */
//
//return( instance );
//
//} /* Render_LoadModel() */


/*******************************************************************
*
*   AttachImage()
*
*   DESCRIPTION:
*       Attach the image to the frame buffer.
*
*******************************************************************/

static void AttachImage( AttachType attach, Image *image, FrameBuffer *buffer )
{
if( !image )
    {
    /* detach */
    switch( attach )
        {
        case ATTACH_TYPE_COLOR:
            buffer->color_image = nullptr;
            clr_struct( &buffer->attach_color );
            buffer->render_info.colorAttachmentCount = 0;
            buffer->render_info.pColorAttachments    = nullptr;
            break;

        case ATTACH_TYPE_DEPTH:
            buffer->depth_image = nullptr;
            clr_struct( &buffer->attach_depth );
            buffer->render_info.pDepthAttachment = nullptr;
            break;

        default:
            debug_assert( attach == ATTACH_TYPE_STENCIL );
            buffer->stencil_image = nullptr;
            clr_struct( &buffer->attach_stencil );
            buffer->render_info.pStencilAttachment = nullptr;
            break;
        }

    return;
    }

/* attach */
if( !image->is_allocated
 || !image->is_created )
    {
    debug_assert_always();
    return;
    }

switch( attach )
    {
    case ATTACH_TYPE_COLOR:
        buffer->color_image = image;

        buffer->attach_color.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        buffer->attach_color.imageView   = image->obj.view;
        buffer->attach_color.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        buffer->attach_color.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        buffer->attach_color.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        buffer->render_info.colorAttachmentCount = 1;
        buffer->render_info.pColorAttachments    = &buffer->attach_color;
        break;

    case ATTACH_TYPE_DEPTH:
        buffer->depth_image = image;

        buffer->attach_depth.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        buffer->attach_depth.imageView   = image->obj.view;
        buffer->attach_depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        buffer->attach_depth.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        buffer->attach_depth.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        buffer->render_info.pDepthAttachment = &buffer->attach_depth;
        break;

    default:
        debug_assert( attach == ATTACH_TYPE_STENCIL );
        buffer->stencil_image = image;

        buffer->attach_stencil.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        buffer->attach_stencil.imageView   = image->obj.view;
        buffer->attach_stencil.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        buffer->attach_stencil.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        buffer->attach_stencil.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        buffer->render_info.pStencilAttachment = &buffer->attach_stencil;
        break;
    }

} /* AttachImage() */


/*******************************************************************
*
*   BeginFrame()
*
*   DESCRIPTION:
*       Begin a new render frame.
*
*******************************************************************/

static bool BeginFrame( RenderEngine *engine )
{
VkResult result = {};
VkImageMemoryBarrier barrier = {};
VkCommandBufferBeginInfo begin_info = {};
VkPipelineStageFlags wait_stages = {};
VkRect2D scissor = {};
VkSubmitInfo submit = {};
VkViewport viewport = {};

/* +memory */
LockMutex( &engine->access.n.memory );

/* advance frame */
engine->frame_index++;
engine->frame_index %= cnt_of_array( engine->frames );
engine->current_frame = &engine->frames[ engine->frame_index ];

Frame *frame = engine->current_frame;

VKN_goto_fail( vkWaitForFences( engine->logical.logical, 1, &frame->fence, VK_TRUE, VKN_WAIT_INFINITE ), begin_frame_fail );
VKN_goto_fail( vkResetFences( engine->logical.logical, 1, &frame->fence ), begin_frame_fail );
VKN_goto_fail( vkResetCommandBuffer( frame->begin_commands, 0 ), begin_frame_fail );
VKN_goto_fail( vkResetCommandBuffer( frame->end_commands, 0 ), begin_frame_fail );
VKN_goto_fail( vkResetCommandBuffer( frame->end_prepend_commands, 0 ), begin_frame_fail );

engine->current_frame->releaser.i->flush( &frame->releaser );
engine->memory.i->begin_frame( &engine->memory );
engine->transitioner.obj.i->begin_frame( &engine->transitioner.obj );
//while( canvas->current_frame->image_frees )
//    {
//    image = canvas->current_frame->image_frees;
//    canvas->current_frame->image_frees = canvas->current_frame->image_frees->next;
//
//    image->next = canvas->device->objects.image_frees;
//    canvas->device->objects.image_frees = image;
//    }
//
//while( canvas->current_frame->index_buffer_frees )
//    {
//    index_buffer = canvas->current_frame->index_buffer_frees;
//    canvas->current_frame->index_buffer_frees = canvas->current_frame->index_buffer_frees->next;
//
//    index_buffer->next = canvas->device->objects.index_buffer_frees;
//    canvas->device->objects.index_buffer_frees = index_buffer;
//    }
//
//while( canvas->current_frame->vertex_buffer_frees )
//    {
//    vertex_buffer = canvas->current_frame->vertex_buffer_frees;
//    canvas->current_frame->vertex_buffer_frees = canvas->current_frame->vertex_buffer_frees->next;
//
//    vertex_buffer->next = canvas->device->objects.vertex_buffer_frees;
//    canvas->device->objects.vertex_buffer_frees = vertex_buffer;
//    }
//
//while( canvas->current_frame->context_frees )
//    {
//    context = canvas->current_frame->context_frees;
//    canvas->current_frame->context_frees = canvas->current_frame->context_frees->next;
//
//    context->next = canvas->device->objects.context_frees;
//    canvas->device->objects.context_frees = context;
//    VKN_goto_bfail( reset_context( context ), canvas_frame_begin_fail );
//    }

/* prepare swap chain */
VKN_goto_bfail( ResizeSwapChain( engine ), begin_frame_fail );
result = vkAcquireNextImageKHR( engine->logical.logical,
                                engine->swap_chain.obj.swap_chain,
                                VKN_WAIT_INFINITE,
                                frame->acquire,
                                VK_NULL_HANDLE,
                                &engine->swap_chain.image_index );

if( result != VK_SUCCESS
 && result != VK_SUBOPTIMAL_KHR )
    {
    return( false );
    }

AttachImage( ATTACH_TYPE_COLOR, &engine->swap_chain.color_images[ engine->swap_chain.image_index ], &engine->swap_chain.frame_buffer );
engine->swap_chain.frame_buffer.attach_color.loadOp                    = VK_ATTACHMENT_LOAD_OP_CLEAR;
engine->swap_chain.frame_buffer.attach_color.clearValue.color          = VKN_make_clear_color_s( 0.0f, 0.5f, 0.0f, 1.0f );
engine->swap_chain.frame_buffer.attach_depth.clearValue.depthStencil   = VKN_make_clear_depth_stencil( 1.0f, 0 );
engine->swap_chain.frame_buffer.attach_stencil.clearValue.depthStencil = VKN_make_clear_depth_stencil( 1.0f, 0 );

begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
VKN_goto_fail( vkBeginCommandBuffer( frame->begin_commands, &begin_info ), begin_frame_fail );

barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
barrier.srcAccessMask                   = VK_ACCESS_NONE;
barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
barrier.image                           = engine->swap_chain.frame_buffer.color_image->obj.image;
barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
barrier.subresourceRange.baseArrayLayer = 0;
barrier.subresourceRange.layerCount     = 1;
barrier.subresourceRange.baseMipLevel   = 0;
barrier.subresourceRange.levelCount     = 1;

wait_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
vkCmdPipelineBarrier( frame->begin_commands,
                      wait_stages,
                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                      0,/* flags*/
                      0,
                      NULL,
                      0,
                      NULL,
                      1,
                      &barrier );

/* clear the back buffer */
viewport.x        = 0.0f;
viewport.y        = 0.0f;
viewport.width    = (float)engine->swap_chain.obj.extent.width;
viewport.height   = (float)engine->swap_chain.obj.extent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
vkCmdSetViewport( frame->begin_commands, 0, 1, &viewport );

scissor.offset.x = 0;
scissor.offset.y = 0;
scissor.extent   = engine->swap_chain.obj.extent;
vkCmdSetScissor( frame->begin_commands, 0, 1, &scissor );

vkCmdBeginRendering( frame->begin_commands, &engine->swap_chain.frame_buffer.render_info );
vkCmdEndRendering( frame->begin_commands );
VKN_return_fail( vkEndCommandBuffer( frame->begin_commands ) );

clr_struct( &submit );
submit.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submit.commandBufferCount = 1;
submit.pCommandBuffers    = &frame->begin_commands;
submit.waitSemaphoreCount = 1;
submit.pWaitSemaphores    = &frame->acquire;
submit.pWaitDstStageMask  = &wait_stages;
VKN_return_fail( vkQueueSubmit( engine->logical.graphics.queue, 1, &submit, VK_NULL_HANDLE ) );

/* -memory */
UnlockMutex( &engine->access.n.memory );
/* -submit */
//UnlockMutex( &engine->access.n.submit );

return( true );

begin_frame_fail:
    {
    /* -memory */
    UnlockMutex( &engine->access.n.memory );

    debug_assert_always();
    return( false );
    }

} /* BeginFrame() */


///*******************************************************************
//*
//*   ClearBackbuffer()
//*
//*   DESCRIPTION:
//*       Clear the backbuffer to the given color.
//*
//*******************************************************************/
//
//static void ClearBackbuffer( const Color4f clear_color, Engine::Engine *engine )
//{
//engine->commands.gfx->ClearRenderTargetView( GetCurrentBackbuffer( engine )->handle, clear_color.f, 0, NULL );
//
//} /* ClearBackbuffer() */
//
//
///*******************************************************************
//*
//*   CreateCommandObjects()
//*
//*   DESCRIPTION:
//*       Create the objects needed to record and submit render
//*       commands.
//*
//*******************************************************************/
//
//static bool CreateCommandObjects( Engine::Engine *engine )
//{ 
///* command queue */
//D3D12_COMMAND_QUEUE_DESC desc = {};
//desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
//desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//if( FAILED( engine->device.ptr->CreateCommandQueue( &desc, IID_PPV_ARGS( &engine->commands.queue ) ) ) )
//    {
//    return( false );
//    }
//
//NameD3D( engine->commands.queue, L"Engine::CommandQueue" );
//
///* spare allocator - we'll use to the issue 'important' commands which fall outside the realm of per-frame */
//if( FAILED( engine->device.ptr->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &engine->commands.spare_allocator ) ) ) )
//    {
//    return( false );
//    }
//
//NameD3D( engine->commands.spare_allocator, L"Engine::CommandAllocator" );
//
///* command recording list */
//if( FAILED( engine->device.ptr->CreateCommandList( Initializers::NODE_MASK_SINGLE_GPU, D3D12_COMMAND_LIST_TYPE_DIRECT, engine->commands.spare_allocator, NULL, IID_PPV_ARGS( &engine->commands.gfx ) ) ) )
//    {
//    return( false );
//    }
//
//NameD3D( engine->commands.gfx, L"Engine::CommandList" );
//
//return( true );
//
//} /* CreateCommandObjects() */
//
//
///*******************************************************************
//*
//*   CreateDefaultBuffer()
//*
//*   DESCRIPTION:
//*       Create a buffer in the default heap initialized to the
//*       common state.
//*
//*******************************************************************/
//
//static bool CreateDefaultBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out )
//{
//D3D12_RESOURCE_DESC desc = Initializers::GetBufferResourceDescriptor( buffer_size );
//
//return( SUCCEEDED( device->CreateCommittedResource( Initializers::GetDefaultHeapProperties(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS( out ) ) ) );
//
//} /* CreateDefaultBuffer() */
//
//
///*******************************************************************
//*
//*   CreateDepthStencil()
//*
//*   DESCRIPTION:
//*       Create render target views for each backbuffer.
//*
//*******************************************************************/
//
//static bool CreateDepthStencil( Engine::Engine *engine )
//{
//D3D12_RESOURCE_DESC desc = Initializers::GetDepthStencilResourceDescriptor( engine->window.width, engine->window.height );
//
//D3D12_CLEAR_VALUE clear_value = {};
//clear_value.Format               = desc.Format;
//clear_value.DepthStencil.Depth   = Initializers::FAR_DEPTH_VALUE;
//clear_value.DepthStencil.Stencil = 0;
//
//engine->surfaces.depth_stencil.width  = (uint16_t)engine->window.width;
//engine->surfaces.depth_stencil.height = (uint16_t)engine->window.height;
//engine->surfaces.depth_stencil.handle = DescriptorHeap_Allocate( &engine->surfaces.dsv_heap ).cpu_hndl;
//if( FAILED( engine->device.ptr->CreateCommittedResource( Initializers::GetDefaultHeapProperties(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value, IID_PPV_ARGS( &engine->surfaces.depth_stencil.resource ) ) ) )
//    {
//    return( false );
//    }
//
//NameD3D( engine->surfaces.depth_stencil.resource, L"Engine::DepthStencilBuffer" );
//
///* depth/stencil view */
//engine->device.ptr->CreateDepthStencilView( engine->surfaces.depth_stencil.resource, NULL, engine->surfaces.depth_stencil.handle );
//
///* transition the texture for depth/stencil writes */
//D3D12_RESOURCE_BARRIER barrier = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE, engine->surfaces.depth_stencil.resource );
//engine->commands.gfx->ResourceBarrier( 1, &barrier );
//
//return( true );
//
//} /* CreateDepthStencil() */
//
//
///*******************************************************************
//*
//*   CreateDescriptorHeaps()
//*
//*   DESCRIPTION:
//*       Create the render target and depth/stencil descriptor heaps.
//*
//*******************************************************************/
//
//static bool CreateDescriptorHeaps( Engine::Engine *engine )
//{
//uint16_t rtv_count = SWAP_CHAIN_DOUBLE_BUFFER;
//uint16_t dsv_count = 1;
//
//return( DescriptorHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Engine::DESCRIPTOR_HEAP_IS_LINEAR, rtv_count, engine->device.ptr, &engine->surfaces.rtv_heap )
//     && DescriptorHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_DSV, Engine::DESCRIPTOR_HEAP_IS_LINEAR, dsv_count, engine->device.ptr, &engine->surfaces.dsv_heap ) );
//
//} /* CreateDescriptorHeaps() */
//
//
///*******************************************************************
//*
//*   CreateDevice()
//*
//*   DESCRIPTION:
//*       Create the DirectX device.
//*
//*******************************************************************/
//
//static bool CreateDevice( Engine::Engine *engine )
//{
//UINT debug_flags = 0;
//#if defined( _DEBUG )
//ID3D12Debug            *debug_info;
//if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debug_info ) ) ) )
//    {
//    debug_info->EnableDebugLayer();
//    ComSafeRelease( &debug_info );
//    debug_flags = DXGI_CREATE_FACTORY_DEBUG;
//    }
//#endif
//
//if( FAILED( CreateDXGIFactory2( debug_flags, IID_PPV_ARGS( &engine->device.dxgi ) ) ) )
//    {
//    return( false );
//    }
//
//if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device.ptr ) ) ) )
//    {
//    IDXGIAdapter       *warp;
//    if( FAILED( engine->device.dxgi->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
//        {
//        ComSafeRelease( &warp );
//        return( false );
//        }
//
//    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device.ptr ) ) ) )
//        {
//        ComSafeRelease( &warp );
//        return( false );
//        }
//    }
//
//NameD3D( engine->device.ptr, L"Engine::Device" );
//      
//return( true );
//
//} /* CreateDevice() */
//
//
///*******************************************************************
//*
//*   CreateRenderTargetViews()
//*
//*   DESCRIPTION:
//*       Create render target views for each backbuffer.
//*
//*******************************************************************/
//
//static bool CreateRenderTargetViews( Engine::Engine *engine )
//{
//for( uint32_t i = 0; i < cnt_of_array( engine->surfaces.backbuffers ); i++ )
//    {
//    Engine::Texture *backbuffer = &engine->surfaces.backbuffers[ i ];
//    backbuffer->width  = engine->window.width;
//    backbuffer->height = engine->window.height;
//    backbuffer->handle = DescriptorHeap_Allocate( &engine->surfaces.rtv_heap ).cpu_hndl;
//    if( FAILED( engine->surfaces.swap_chain->GetBuffer( (UINT)i, IID_PPV_ARGS( &backbuffer->resource ) ) ) )
//        {
//        return( false );
//        }
//
//    NameD3DIndex( backbuffer->resource, i, L"Engine::RenderBuffer" );
//
//    engine->device.ptr->CreateRenderTargetView( backbuffer->resource, NULL, backbuffer->handle );
//    }
//
//return( true );
//
//} /* CreateRenderTargetViews() */
//
//
///*******************************************************************
//*
//*   CreateSwapChain()
//*
//*   DESCRIPTION:
//*       Create the DirectX device.
//*
//*******************************************************************/
//
//static bool CreateSwapChain( Engine::Engine *engine )
//{
///* fill out the descriptions */
//DXGI_SWAP_CHAIN_DESC desc = Initializers::GetSwapChainDescriptor( engine->window.width, engine->window.height, engine->window.handle );
//if( FAILED( engine->device.dxgi->CreateSwapChain( engine->commands.queue, &desc, &engine->surfaces.swap_chain ) ) )
//    {
//    return( false );
//    }
//      
//return( true );
//
//} /* CreateSwapChain() */
//
//
///*******************************************************************
//*
//*   CreateUploadBuffer()
//*
//*   DESCRIPTION:
//*       Create a upload buffer.
//*
//*******************************************************************/
//
//static bool CreateUploadBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out )
//{
//D3D12_RESOURCE_DESC desc = Initializers::GetBufferResourceDescriptor( buffer_size );
//
//return( SUCCEEDED( device->CreateCommittedResource( Initializers::GetUploadHeapProperties(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS( out ) ) ) );
//
//} /* CreateUploadBuffer() */
//
//
///*******************************************************************
//*
//*   DestroyScenes()
//*
//*   DESCRIPTION:
//*       Destroy the render state for each scene.
//*
//*******************************************************************/
//
//static void DestroyScenes( Engine::Engine *engine, Universe *universe )
//{
//if( !universe )
//    {
//    return;
//    }
//
//NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_SCENE ) );
//SceneComponent *scene;
//while( NonOwningGroup_GetNext( &engine->group, NULL, (void**)&scene ) )
//    {
//    Scene::Scene_Destroy( (Scene::Scene*)scene->render_state );
//    }
//
//} /* DestroyScenes() */
//
//
///*******************************************************************
//*
//*   DrawScenes()
//*
//*   DESCRIPTION:
//*       Update and draw each scene.
//*
//*******************************************************************/
//
//static void DrawScenes( Engine::Engine *engine, Universe *universe )
//{
//HashMap *map = &engine->scene_draw.map.map;
//SceneComponent **draw_order = engine->scene_draw.draw_order;
//
//HashMap_Clear( map );
//
//NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_SCENE ) );
//SceneComponent *seen_scene;
//clr_array( draw_order );
//while( NonOwningGroup_GetNext( &engine->group, NULL, (void**)&seen_scene ) )
//    {
//    debug_assert( HashMap_At( seen_scene->scene_name_hash, map ) == NULL );
//    
//    Scene::Scene *render_scene = (Scene::Scene*)seen_scene->render_state;
//    Scene::Scene_BeginFrame( seen_scene->viewport_extent, render_scene );
//
//    hard_assert( HashMap_Insert( seen_scene->scene_name_hash, &seen_scene, map ) != NULL );
//    draw_order[ map->size - 1 ] = seen_scene;
//    }
//
//if( map->size == 0 )
//    {
//    return;
//    }
//
//EntityId model_entity;
//NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_MODEL, COMPONENT_TRANSFORM ) );
//while( NonOwningGroup_GetNext( &engine->group, &model_entity, NULL ) )
//    {
//    ModelComponent     *model     = (ModelComponent*)NonOwningGroup_GetComponent( COMPONENT_MODEL, &engine->group );
//    TransformComponent *transform = (TransformComponent*)NonOwningGroup_GetComponent( COMPONENT_TRANSFORM, &engine->group );
//
//    Float4x4 mtx_world;
//    Math_Float4x4TransformSpin( transform->position, transform->rotation, transform->scale, &mtx_world );
//
//    SceneComponent *home_scene = *(SceneComponent**)HashMap_At( model->scene_name_hash, map ); // TODO <MPA> - Do we want an entity to be able to belong to more than one scene?
//    Scene::Scene_RegisterObject( model->asset_name.str, model, &mtx_world, (Scene::Scene*)home_scene->render_state );
//    }
//
///* do a quick and dirty bubble sort on the scenes to prioritize them by draw order */
//for( uint32_t i = 0; i < map->size; i++ )
//    {
//    for( uint32_t j = 0; j < map->size - i - 1; j++ )
//        {
//        if( draw_order[ j ]->draw_order < draw_order[ j + 1 ]->draw_order )
//            {
//            continue;
//            }
//
//        SceneComponent *temp = draw_order[ j ];
//        draw_order[ j ] = draw_order[ j + 1 ];
//        draw_order[ j + 1 ] = temp;
//        }
//    }
//
//for( uint32_t i = 0; i < map->size; i++ )
//    {
//    SetViewport( draw_order[ i ]->viewport_top_left, draw_order[ i ]->viewport_extent, engine );
//    Scene::Scene_Draw( (Scene::Scene*)draw_order[ i ]->render_state );
//    }
//
//#undef MAX_SCENE_CNT
//} /* DrawScenes() */


/*******************************************************************
*
*   CreateSwapChain()
*
*   DESCRIPTION:
*       Create new swap chain from desired extent.
*
*******************************************************************/

static bool CreateSwapChain( RenderEngine *engine )
{
SwapChain *swap = &engine->swap_chain;

/* depth-stencil */
VKN_image_build_type *bi = &engine->builders.image;
Image *depth_stencil = &swap->depth_stencil;
bi->config->reset( bi )->
            add_sharing_family( engine->logical.graphics.family, bi )->
            set_extent( engine->swap_chain.desired_extent.width, engine->swap_chain.desired_extent.height, bi )->
            set_image_data_format( VKN_IMAGE_DATA_FORMAT_DEPTH_STENCIL, bi )->
            set_usage( VKN_IMAGE_USAGE_DEPTH_STENCIL_ATTACH, bi );
VKN_return_bfail( VKN_image_create( bi, &depth_stencil->obj ) );
depth_stencil->is_allocated = true;
depth_stencil->is_created   = true;
depth_stencil->uid          = engine->transitioner.next_image_uid++;
engine->transitioner.obj.i->register_new( depth_stencil->uid, depth_stencil->obj.image, VK_IMAGE_LAYOUT_UNDEFINED, 1, &engine->transitioner.obj );

/* swap-chain */
VKN_swap_chain_build_type *bsc = &engine->builders.swap_chain;
bsc->config->reset( bsc )->
             set_extent( engine->swap_chain.desired_extent.width, engine->swap_chain.desired_extent.height, bsc )->
             set_default_present_mode( bsc )->
             set_default_surface_format( bsc )->
             commit_config( engine->surface, bsc );
VKN_return_bfail( VKN_swap_chain_create( engine->logical.present.family, engine->logical.graphics.family, bsc, &swap->obj ) );

for( u32 i = 0; i < swap->obj.images.count; i++ )
    {
    Image *image = &swap->color_images[ i ];
    VKN_image_create_from_swap_chain( swap->obj.images.images[ i ],
                                      swap->obj.images.views[ i ],
                                      swap->obj.extent,
                                      swap->obj.format,
                                      &image->obj );
    image->is_allocated = true;
    image->is_created   = true;
    image->uid          = engine->transitioner.next_image_uid++;
    engine->transitioner.obj.i->register_new( image->uid, image->obj.image, VK_IMAGE_LAYOUT_UNDEFINED, 1, &engine->transitioner.obj );
    }

/* frame buffer */
swap->frame_buffer.render_info.sType               = VK_STRUCTURE_TYPE_RENDERING_INFO;
swap->frame_buffer.render_info.renderArea.offset.x = 0;
swap->frame_buffer.render_info.renderArea.offset.y = 0;
swap->frame_buffer.render_info.renderArea.extent   = swap->obj.extent;
swap->frame_buffer.render_info.layerCount          = 1;

return( true );

}   /* CreateSwapChain() */


/*******************************************************************
*
*   DestroySwapChain()
*
*   DESCRIPTION:
*       Free the existing swap chain.
*
*******************************************************************/

static void DestroySwapChain( RenderEngine *engine )
{
if( engine->swap_chain.depth_stencil.is_created )
    {
    engine->transitioner.obj.i->unregister( engine->swap_chain.depth_stencil.uid, &engine->transitioner.obj );
    VKN_image_destroy( NULL, &engine->swap_chain.depth_stencil.obj );
    engine->swap_chain.depth_stencil.is_allocated = false;
    engine->swap_chain.depth_stencil.is_created   = false;
    engine->swap_chain.depth_stencil.uid          = 0;
    }

for( u32 i = 0; i < engine->swap_chain.obj.images.count; i++ )
    {
    Image *image = &engine->swap_chain.color_images[ i ];
    engine->transitioner.obj.i->unregister( image->uid, &engine->transitioner.obj );
    image->is_allocated = false;
    image->is_created   = false;
    image->uid          = 0;
    clr_struct( image );
    }

VKN_swap_chain_destroy( NULL, &engine->swap_chain.obj );

}   /* DestroySwapChain() */


/*******************************************************************
*
*   EndFrame()
*
*   DESCRIPTION:
*       End the current frame and present.
*
*******************************************************************/

static bool EndFrame( RenderEngine *engine )
{
//context_type           *context;    /* context to submit            */

VkCommandBufferBeginInfo begin_info = {};
begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

u32 submit_cnt = 2;
VkCommandBuffer *submits = nullptr;
Frame *frame = nullptr;
VkImageMemoryBarrier barrier = {};
VkSubmitInfo submit = {};
VkPresentInfoKHR present = {};

/* +submit */
//LockMutex( &engine->access.n.submit );

/* +memory */
LockMutex( &engine->access.n.memory );

frame = engine->current_frame;
engine->staging.i->flush( &engine->staging );

/* build array of command buffers to submit */
//for( context = canvas->current_frame->context_frees; context; context = context->next, submit_cnt++ );
submits = VKN_arena_allocate_array( VkCommandBuffer, submit_cnt, &engine->current_frame->arena );
VKN_goto_bfail( submits, end_frame_fail );

submit_cnt = 0;
submits[ submit_cnt++ ] = frame->end_prepend_commands;
VKN_goto_fail( vkBeginCommandBuffer( frame->end_prepend_commands, &begin_info ), end_frame_fail );

//for( context = canvas->current_frame->context_frees; context; context = context->next )
//    {
//    if( context == canvas->current_frame->context_frees )
//        {
//        /*--------------------------------------------------
//        First context submits prepend barriers on the
//        dedicated command buffer
//        --------------------------------------------------*/
//        context->transitioner.obj.i->commit( canvas->current_frame->commands_end_prepend,
//                                             &canvas->transitioner.obj,
//                                             &context->transitioner.obj );
//        }
//
//    if( context->next )
//        {
//        /*--------------------------------------------------
//        Subsequent contexts submit prepend barriers on the
//        preceding context's command buffer
//        --------------------------------------------------*/
//        context->transitioner.obj.i->commit( context->commands,
//                                             &canvas->transitioner.obj,
//                                             &context->next->transitioner.obj );
//        }
//
//    VKN_goto_fail( vkEndCommandBuffer( context->commands ), canvas_frame_end_fail );
//    submit_arr[ submit_cnt++ ] = context->commands;
//    }

VKN_goto_fail( vkEndCommandBuffer( frame->end_prepend_commands ), end_frame_fail );
submits[ submit_cnt++ ] = frame->end_commands;

/* present */
VKN_goto_fail( vkBeginCommandBuffer( frame->end_commands, &begin_info ), end_frame_fail );

barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
barrier.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
barrier.dstAccessMask                   = VK_ACCESS_NONE;
barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
barrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
barrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
barrier.image                           = engine->swap_chain.frame_buffer.color_image->obj.image;
barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
barrier.subresourceRange.baseArrayLayer = 0;
barrier.subresourceRange.layerCount     = 1;
barrier.subresourceRange.baseMipLevel   = 0;
barrier.subresourceRange.levelCount     = 1;

vkCmdPipelineBarrier( frame->end_commands, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier );
VKN_goto_fail( vkEndCommandBuffer( frame->end_commands ), end_frame_fail );

submit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submit.commandBufferCount   = submit_cnt;
submit.pCommandBuffers      = submits;
submit.signalSemaphoreCount = 1;
submit.pSignalSemaphores    = &frame->render;

VKN_goto_fail( vkQueueSubmit( engine->logical.graphics.queue, 1, &submit, frame->fence ), end_frame_fail );

present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
present.swapchainCount     = 1;
present.pSwapchains        = &engine->swap_chain.obj.swap_chain;
present.pImageIndices      = &engine->swap_chain.image_index;
present.waitSemaphoreCount = 1;
present.pWaitSemaphores    = &frame->render;

VKN_goto_fail( vkQueuePresentKHR( engine->logical.graphics.queue, &present ), end_frame_fail );

/* -memory */
UnlockMutex( &engine->access.n.memory );

return( true );

end_frame_fail:
    {
    /* -memory */
    UnlockMutex( &engine->access.n.memory );
    return( false );
    }

} /* EndFrame() */


///*******************************************************************
//*
//*   ExecuteCommandLists()
//*
//*   DESCRIPTION:
//*       Execute all the commands recorded on our command lists.
//*
//*******************************************************************/
//
//static void ExecuteCommandLists( Engine::Engine *engine )
//{
//ID3D12CommandList *command_list = engine->commands.gfx;
//engine->commands.queue->ExecuteCommandLists( 1, &command_list );
//
//} /* ExecuteCommandLists() */
//
//
///*******************************************************************
//*
//*   FlushCommandQueue()
//*
//*   DESCRIPTION:
//*       Wait for all unfinished GPU commands to complete.
//*
//*******************************************************************/
//
//static bool FlushCommandQueue( Engine::Engine *engine )
//{
//return( WaitForFrameToFinish( engine->commands.last_submitted_frame, engine ) );
//
//} /* FlushCommandQueue() */
//
//
///*******************************************************************
//*
//*   GetWindowExtent()
//*
//*   DESCRIPTION:
//*       Get the width and height of the given window.
//*
//*******************************************************************/
//
//static bool GetWindowExtent( HWND window, UINT *width, UINT *height )
//{
//RECT rect;
//if( !GetWindowRect( window, &rect ) )
//    {
//    return( false );
//    }
//
//*width  = (UINT)( rect.right  - rect.left );
//*height = (UINT)( rect.bottom - rect.top  );
//
//return( true );
//
//} /* GetWindowExtent() */
//
//
///*******************************************************************
//*
//*   InitDirectX()
//*
//*   DESCRIPTION:
//*       Initialize the DirectX driver.
//*
//*******************************************************************/
//
//static bool InitDirectX( Engine::Engine *engine )
//{
//HashMap_InitImplementation( &engine->scene_draw.map );
//
///* obtain handle to main application window */
//engine->window.handle = ::GetActiveWindow();
//if( engine->window.handle == NULL )
//    {
//    return( false );
//    }
//    
///* get the native dimensions of the application window */
//if( !GetWindowExtent( engine->window.handle, &engine->window.width, &engine->window.height ) )
//    {
//    return( false );
//    }
//
///* device and graphics interface factory */
//if( !CreateDevice( engine ) )
//    {
//    goto failure;
//    }
//
///* descriptor sizes */
////for( uint32_t i = 0; i < cnt_of_array( engine->device.descriptor_sizes ); i++ )
////    {
////    engine->device.descriptor_sizes[ i ] = engine->device.ptr->GetDescriptorHandleIncrementSize( (D3D12_DESCRIPTOR_HEAP_TYPE)i );
////    }
//
///* fence */
//if( FAILED( engine->device.ptr->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &engine->commands.fence ) ) ) )
//    {
//    goto failure;
//    }
//
//NameD3D( engine->commands.fence, L"Engine::Fence" );
//
//if( !InitFrames( engine ) )
//    {
//    goto failure;
//    }
//
///* command buffer, command lists, and allocator */
//if( !CreateCommandObjects( engine ) )
//    {
//    goto failure;
//    }
//
///* swap chain */
//if( !CreateSwapChain( engine ) )
//    {
//    goto failure;
//    }
//
///* desciptor heaps */
//if( !CreateDescriptorHeaps( engine ) )
//    {
//    goto failure;
//    }
//
///* backbuffer views */
//if( !CreateRenderTargetViews( engine ) )
//    {
//    goto failure;
//    }
//
///* depth/stencil */
//if( !CreateDepthStencil( engine ) )
//    {
//    goto failure;
//    }
//
//if( !InitPipelines( engine ) )
//    {
//    goto failure;
//    }
//
///* set the viewport */
//SetViewport( Math_Float2Make( 0.0f, 0.0f ), Math_Float2Make( (float)engine->window.width, (float)engine->window.height ), engine );
//
//if( FAILED( engine->commands.gfx->Close() ) )
//    {
//    goto failure;
//    }
//
//ExecuteCommandLists( engine );
//if( !FlushCommandQueue( engine ) )
//    {
//    goto failure;
//    }
//
//return( true );
//
//failure:
//    {
//    assert( false );
//    Reset( engine, NULL );
//    return( false );
//    }
//
//} /* InitDirectX() */
//
//
///*******************************************************************
//*
//*   InitFrames()
//*
//*   DESCRIPTION:
//*       Initialize the per-frame resources.
//*
//*******************************************************************/
//
//static bool InitFrames( Engine::Engine *engine )
//{
//for( uint8_t i = 0; i < cnt_of_array( engine->frames ); i++ )
//    {
//    Engine::Frame *frame = &engine->frames[ i ];
//    frame->frame_index = i;
//
//    /* command allocator */
//    if( FAILED( engine->device.ptr->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &frame->command_allocator ) ) ) )
//        {
//        return( false );
//        }
//
//    NameD3DIndex( frame->command_allocator, i, L"Engine::Frame::CommandAllocator" );
//    }
//
//return( true );
//
//} /* InitFrames() */
//
//
///*******************************************************************
//*
//*   InitPipelines()
//*
//*   DESCRIPTION:
//*       Initialize the render pipelines.
//*
//*******************************************************************/
//
//static bool InitPipelines( Engine::Engine *engine )
//{
//return( Pipelines::Pipelines_Init( engine->device.ptr, &engine->pipelines ) );
//
//} /* InitPipelines() */
//
//
///*******************************************************************
//*
//*   OnSceneAttach()
//*
//*   DESCRIPTION:
//*       Handle a scene component being created, and create its
//*       private render state.
//*
//*******************************************************************/
//
//static void OnSceneAttach( const EntityId entity, const ComponentClass cls, void *component, Universe *universe )
//{
//debug_assert( cls == COMPONENT_SCENE );
//SceneComponent *new_scene = (SceneComponent*)component;
//
//Scene::Scene *render_scene = (Scene::Scene*)malloc( sizeof( Scene::Scene ) );
//new_scene->render_state = render_scene;
//
//Scene::Scene_Init( AsRenderEngine( universe ), render_scene );
//
//} /* OnSceneAttach() */
//
//
///*******************************************************************
//*
//*   OnSceneRemove()
//*
//*   DESCRIPTION:
//*       Handle a scene component being created, and destroy its
//*       private render state.
//*
//*******************************************************************/
//
//static void OnSceneRemove( const EntityId entity, const ComponentClass cls, void *component, Universe *universe )
//{
//debug_assert( cls == COMPONENT_SCENE );
//SceneComponent *dying_scene = (SceneComponent*)component;
//Scene::Scene_Destroy( (Scene::Scene*)dying_scene->render_state );
//
//free( dying_scene->render_state );
//
//} /* OnSceneRemove() */
//
//
///*******************************************************************
//*
//*   Reset()
//*
//*   DESCRIPTION:
//*       Release all renderer resources.
//*
//*******************************************************************/
//
//static void Reset( Engine::Engine *engine, Universe *universe )
//{
//DestroyScenes( engine, universe );
//FlushCommandQueue( engine );
//Pipelines::Pipelines_Destroy( &engine->pipelines );
//for( uint32_t i = 0; i < cnt_of_array( engine->frames ); i++ )
//    {
//    Engine::Frame *frame = &engine->frames[ i ];
//    SweepFrameTrash( frame );
//    ComSafeRelease( &frame->command_allocator );
//    }
//
//ComSafeRelease( &engine->surfaces.depth_stencil );
//for( uint32_t i = 0; i < cnt_of_array( engine->surfaces.backbuffers ); i++ )
//    {
//    ComSafeRelease( &engine->surfaces.backbuffers[ i ] );
//    }
//
//DescriptorHeap_Destroy( NULL, &engine->surfaces.dsv_heap );
//DescriptorHeap_Destroy( NULL, &engine->surfaces.rtv_heap );
//
//ComSafeRelease( &engine->surfaces.rtv_heap );
//ComSafeRelease( &engine->surfaces.swap_chain );
//ComSafeRelease( &engine->commands.fence );
//ComSafeRelease( &engine->commands.gfx );
//ComSafeRelease( &engine->commands.spare_allocator );
//ComSafeRelease( &engine->commands.queue );
//ComSafeRelease( &engine->device.ptr );
//ComSafeRelease( &engine->device.dxgi );
//
//clr_struct( engine );
//
//} /* Reset() */


/*******************************************************************
*
*   ResizeSwapChain()
*
*   DESCRIPTION:
*       Resize the swap chain to meet the desired extent.
*
*******************************************************************/

static bool ResizeSwapChain( RenderEngine *engine )
{
if( engine->swap_chain.obj.extent.width  == engine->swap_chain.desired_extent.width
 && engine->swap_chain.obj.extent.height == engine->swap_chain.desired_extent.height )
    {
    return( true );
    }

/* flush and recreate */
VKN_return_fail( vkDeviceWaitIdle( engine->logical.logical ) );
DestroySwapChain( engine );

return( CreateSwapChain( engine ) );

}   /* ResizeSwapChain() */


///*******************************************************************
//*
//*   SetViewport()
//*
//*   DESCRIPTION:
//*       Set the given viewport.
//*
//*******************************************************************/
//
//static void SetViewport( const Float2 top_left, const Float2 extent, Engine::Engine *engine )
//{
//Float2 window_s   = Math_Float2Make( (float)engine->window.width, (float)engine->window.height );
//Float2 top_left_s = Math_Float2HadamardProduct( top_left, window_s );
//Float2 extent_s   = Math_Float2HadamardProduct( extent,   window_s );
//
//D3D12_VIEWPORT viewport = {};
//viewport.TopLeftX = top_left_s.v.x;
//viewport.TopLeftY = top_left_s.v.y;
//viewport.Width    = extent_s.v.x;
//viewport.Height   = extent_s.v.y;
//viewport.MinDepth = 0.0f;
//viewport.MaxDepth = 1.0f;
//
//engine->commands.gfx->RSSetViewports( 1, &viewport );
//
//} /* SetViewport() */
//
//
///*******************************************************************
//*
//*   ScheduleBufferUpload()
//*
//*   DESCRIPTION:
//*       Schedule a buffer to be uploaded to the GPU.  Assumes the
//*       destination buffer is in the common state.
//*
//*******************************************************************/
//
//static bool ScheduleBufferUpload( const void *data, const uint32_t data_sz, ID3D12Device *device, ID3D12GraphicsCommandList *gfx, ID3D12Resource *upload, ID3D12Resource *gpu )
//{
///* Sanity check */
//debug_if( true,
//    ID3D12Device *dest_device = NULL;
//    gpu->GetDevice( IID_PPV_ARGS( &dest_device ) );
//    assert( device == dest_device );
//    );
//
//D3D12_RESOURCE_DESC gpu_desc    = gpu->GetDesc();
//D3D12_RESOURCE_DESC upload_desc = upload->GetDesc();
//
//D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
//unsigned int num_rows;
//uint64_t row_sz;
//uint64_t required_sz;
//device->GetCopyableFootprints( &gpu_desc, 0, 1, 0, &layout, &num_rows, &row_sz, &required_sz );
//if( upload_desc.Width < required_sz
// || upload_desc.Width < data_sz
// || !gpu )
//    {
//    return( false );
//    }
//
//void *uploading;
//if( FAILED( upload->Map( 0, NULL, &uploading ) ) )
//    {
//    return( false );
//    }
//
//memcpy( uploading, data, data_sz );
//upload->Unmap( 0, NULL );
//
//D3D12_RESOURCE_BARRIER to_write = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST, gpu );
//gfx->ResourceBarrier( 1, &to_write );
//
//gfx->CopyBufferRegion( gpu, 0, upload, 0, data_sz );
//
//D3D12_RESOURCE_BARRIER to_read = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, gpu );
//gfx->ResourceBarrier( 1, &to_write );
//
//return( true );
//
//} /* ScheduleBufferUpload() */
//
//
///*******************************************************************
//*
//*   WaitForFrameToFinish()
//*
//*   DESCRIPTION:
//*       Block until all the graphics commands in the queue have
//*       been completed.
//*
//*******************************************************************/
//
//static bool WaitForFrameToFinish( uint64_t frame_num, Engine::Engine *engine )
//{
//HANDLE blocking = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
//if( blocking == NULL
// || FAILED( engine->commands.fence->SetEventOnCompletion( frame_num, blocking ) ) )
//    {
//    return( false );
//    }
//
//::WaitForSingleObject( blocking, INFINITE );
//::CloseHandle( blocking );
//
//return( true );
//
//} /* WaitForFrameToFinish() */
