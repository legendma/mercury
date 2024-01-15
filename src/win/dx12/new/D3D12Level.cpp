#include <string.h>

#include "D3D12Core.hpp"
#include "D3D12Initializers.hpp"
#include "D3D12Level.hpp"
#include "D3D12Upload.hpp"
#include "ModelAsset.hpp"
#include "TextureAsset.hpp"
#include "Utilities.hpp"

// TODO - <MPA> TESTING REMOVE
static const AssetFileAssetId PREFAB_MODELS[] = 
    {
    1336217753
    };
// END TODO

#define MAX_MODEL_ALLOCATOR_SZ      ( 1024 * 1024 )


namespace D3D12 { namespace Level
{

typedef struct _UnmappedModelBuffers
    {
    AssetFileAssetId    model_id;
    AssetFileModelVertex
                       *vertices;
    AssetFileModelIndex 
                       *indices;
    } UnmappedModelBuffers;


/*******************************************************************
*
*   EnsureTextureAsset()
*
*   DESCRIPTION:
*       Check if the given texture is in the asset map, otherwise
*       insert it.
* 
*       Returns TRUE if the asset already existed.
*
*******************************************************************/

static inline bool EnsureTextureAsset( const AssetFileAssetId texture_id, Level *level, GPUAsset **asset )
{
if( asset )
    {
    *asset = NULL;
    }

GPUAsset *entry = NULL;
bool ret = false;

entry = (GPUAsset*)HashMap_At( texture_id, &level->assets.map );
if( entry )
    {
    ret = true;
    }
else
    {
    entry = (GPUAsset*)HashMap_Insert( texture_id, NULL, &level->assets.map );
    if( !entry )
        {
        debug_assert_always();
        return( false );
        }

    entry->id = texture_id;
    entry->kind = ASSET_FILE_ASSET_KIND_TEXTURE;
    }

if( asset )
    {
    *asset = entry;
    }

return( ret );

} /* EnsureTextureAsset() */


/*******************************************************************
*
*   GetTotalIndexCount()
*
*   DESCRIPTION:
*       Get the total number of indices used by all the models in
*       this level.
*
*******************************************************************/

static inline uint32_t GetTotalIndexCount( Level *level )
{
uint32_t ret = 0;
for( uint32_t it_model = 0; it_model < level->model_count; it_model++ )
    {
    GPUAsset *asset = (GPUAsset*)HashMap_At( level->models[ it_model ], &level->assets.map );
    if( !asset)
        {
        debug_assert_always();
        return( 0 );
        }

    debug_assert( asset->kind == ASSET_FILE_ASSET_KIND_MODEL );
    Model *model = &asset->t.model;
    ret += model->index_count;
    }

return( ret );

} /* GetTotalIndexCount() */


/*******************************************************************
*
*   GetTotalVertexCount()
*
*   DESCRIPTION:
*       Get the total number of vertices used by all the models in
*       this level.
*
*******************************************************************/

static inline uint32_t GetTotalVertexCount( Level *level )
{
uint32_t ret = 0;
for( uint32_t it_model = 0; it_model < level->model_count; it_model++ )
    {
    GPUAsset *asset = (GPUAsset*)HashMap_At( level->models[ it_model ], &level->assets.map );
    if( !asset)
        {
        debug_assert_always();
        return( 0 );
        }

    debug_assert( asset->kind == ASSET_FILE_ASSET_KIND_MODEL );
    Model *model = &asset->t.model;
    ret += model->vertex_count;
    }

return( ret );

} /* GetTotalVertexCount() */


/*******************************************************************
*
*   GetUnmappedBuffersForModel()
*
*   DESCRIPTION:
*       Get the requested model's unmapped buffers.
*
*******************************************************************/

static inline const UnmappedModelBuffers * GetUnmappedBuffersForModel( const AssetFileAssetId model_id, const uint32_t count, const UnmappedModelBuffers *arr )
{
const UnmappedModelBuffers *ret = NULL;
for( uint32_t i = 0; i < count; i++ )
    {
    if( arr[ i ].model_id == model_id )
        {
        ret = &arr[ i ];
        break;
        }
    }

return( ret );

} /* GetUnmappedBuffersForModel() */


/*******************************************************************
*
*   InsertModelAsset()
*
*   DESCRIPTION:
*       Insert a new model into the assets maps.
*
*******************************************************************/

static inline GPUAsset * InsertModelAsset( const AssetFileAssetId model_id, Level *level )
{
debug_assert( HashMap_At( model_id, &level->assets.map ) == NULL );
GPUAsset *ret = (GPUAsset *)HashMap_Insert( model_id, NULL, &level->assets.map );
if( !ret )
    {
    return( NULL );
    }

ret->id = model_id;
ret->kind = ASSET_FILE_ASSET_KIND_MODEL;

return( ret );

} /* InsertModelAsset() */


static void * AsyncMain( void *args );
static bool   LoadModel( const AssetFileAssetId model_id, ResourceLoader *loader, LinearAllocator *scratch, LinearAllocator *upload, UnmappedModelBuffers *unmapped, Level *level );
static void   Release( Level *level );
static bool   WriteModelsGeometry( const UnmappedModelBuffers *unmapped, const void *upload_base_address, LinearAllocator *heap, Core::IMResource *upload, Upload::Frame *frame, Level *level );


/*******************************************************************
*
*   Destroy()
*
*   DESCRIPTION:
*       Release the resources owned by the level.
*
*******************************************************************/

void Destroy( Level *level )
{
Release( level );

} /* Destroy() */


/*******************************************************************
*
*   Init()
*
*   DESCRIPTION:
*       Initialize a hash map created by the default implementation.
*
*******************************************************************/

bool Init( const char *name, Level *level )
{
if( !level )
    {
    debug_assert_always();
    return( false );
    }

*level = {};

level->name  = AssetFile_CopyNameString( name );
level->id    = AssetFile_MakeAssetIdFromNameString( &level->name );
level->mutex = PTHREAD_MUTEX_INITIALIZER;
HashMap_InitImplementation( &level->assets );
/* heap will be created by worker thread, once the required size is known */

return( true );

} /* Init() */


/*******************************************************************
*
*   Destroy()
*
*   DESCRIPTION:
*       Release the resources owned by the level.
*
*******************************************************************/

void LoadAsync( const char *thread_name, Level *level )
{
pthread_mutex_lock( &level->mutex );
pthread_create( &level->load_thread, NULL, &AsyncMain, level );
if( thread_name )
    {
    pthread_setname_np( level->load_thread, thread_name );
    }

} /* Destroy() */


/*******************************************************************
*
*   AsyncMain()
*
*   DESCRIPTION:
*       Asynchronous thread entry point.
*
*******************************************************************/

static void * AsyncMain( void *args )
{
Core::IMDevice *device = Core::GetDevice();
Level *level = (Level*)args;
Upload::Frame *frame = NULL;
Core::IMResource *upload = NULL;
LinearAllocator scratch_allocator = {};
UnmappedModelBuffers *unmapped = NULL;
ResourceLoader loader;
void *upload_data = NULL;
LinearAllocator upload_allocator = {};
LinearAllocator heap_allocator = {};

if( !level
 || !device )
    {
    hard_assert_always();
    pthread_exit( NULL );
    }

frame = Upload::BeginFrame();
if( !frame)
    {
    goto failed_0;
    }

level->heap = Core::CreateHeap( LEVEL_HEAP_SZ );
if( !level->heap )
    {
    goto failed_1;
    }

NameD3DString( level->heap, level->name.str, L"Level::Heap" );

LinearAllocator_InitDetached( LEVEL_HEAP_SZ, &heap_allocator );

upload = Upload::AllocateUpload( LEVEL_HEAP_SZ, frame );
if( !upload )
    {
    goto failed_2;
    }

if( !LinearAllocator_Init( MAX_MODEL_ALLOCATOR_SZ, &scratch_allocator ) )
    {
    goto failed_2; /* nothing new */
    }

unmapped = (UnmappedModelBuffers*)LinearAllocator_Allocate( sizeof( UnmappedModelBuffers ) * MAX_MODEL_COUNT, &scratch_allocator );
if( !unmapped )
    {
    goto failed_3;
    }

if( !ResourceLoader_Init( &loader ) )
    {
    goto failed_3; /* nothing new */
    }

upload->Map( 0, NULL, &upload_data );
if( !LinearAllocator_InitAttached( upload->GetDesc().Width, upload_data, &upload_allocator ) )
    {
    goto failed_4;
    }

// TODO <MPA> Implement level prefabs and prefab definition loading

for( uint32_t i = 0; i < cnt_of_array( PREFAB_MODELS ); i++ )
    {
    UnmappedModelBuffers temp_unmapped = {};
    if( !LoadModel( PREFAB_MODELS[ i ], &loader, &scratch_allocator, &upload_allocator, &temp_unmapped, level ) )
        {
        goto failed_5;
        }

    unmapped[ level->model_count ] = temp_unmapped;
    level->models[ level->model_count ] = temp_unmapped.model_id;
    level->model_count++;
    }

LinearAllocator_Destroy( &upload_allocator );
upload->Unmap( 0, NULL );

Utilities_ShellSortU32Ascending( level->model_count, level->models );
WriteModelsGeometry( unmapped, upload_data, &heap_allocator, upload, frame, level );
Utilities_ShellSortU32Ascending( level->texture_count, level->textures );
for( uint32_t i = 0; i < level->texture_count; i++ )
    {
    LoadAndWriteTexture( level->textures[ i ] );

    }

LinearAllocator_Destroy( &scratch_allocator );
Upload::EndFrame( frame );

level->is_loaded = true;
pthread_mutex_unlock( &level->mutex );

return( NULL );

/* failures */
failed_5:
LinearAllocator_Destroy( &upload_allocator );

failed_4:
upload->Unmap( 0, NULL );
ResourceLoader_Destroy( &loader );

failed_3:
LinearAllocator_Destroy( &scratch_allocator );

failed_2:
ComSafeRelease( &level->heap );

failed_1:
Upload::CancelFrame( frame );

failed_0:
debug_assert_always();
level->model_count = 0;
pthread_mutex_unlock( &level->mutex );

return( NULL );

} /* AsyncMain() */


/*******************************************************************
*
*   LoadAndWriteTexture()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

static bool LoadAndWriteTexture( const AssetFileAssetId texture_id, ResourceLoader *loader, LinearAllocator *scratch, Core::IMResource *upload, Level *level )
{
LinearAllocatorResetToken reset_scratch = LinearAllocator_GetResetToken( scratch );

GPUAsset *gpu_asset = (GPUAsset*)HashMap_At( texture_id, &level->assets.map );
if( !gpu_asset
 || gpu_asset->kind != ASSET_FILE_ASSET_KIND_TEXTURE )
    {
    debug_assert_always();
    return( false );
    }

Texture::Texture *texture_asset = &gpu_asset->t.texture;

Render::Asset::Texture read_texture = {};
if( !Render::Asset::ReadTexture( texture_id, loader, scratch, &read_texture )
 || read_texture.mip_count >= 1 )
    {
    debug_assert_always();
    return( false );
    }

DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
switch( read_texture.channel_count )
    {
    case 1:
        format = DXGI_FORMAT_R8_UNORM;
        break;

    case 4:
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

    default:
        hard_assert_always();
        break;
    }

format = Texture::GetSRGBFormat( format );

D3D12_RESOURCE_DESC desc = Initializers::GetTexture2DResourceDescriptor( read_texture.mips->width, read_texture.mips->height, Initializers::TEXTURE_USAGE_SHADER_RESOURCE_ONLY, format );
D3D12_RESOURCE_ALLOCATION_INFO placement_info = Texture::GetPlacedTextureInfo( &desc );


} /* LoadAndWriteTexture() */


/*******************************************************************
*
*   LoadModel()
*
*   DESCRIPTION:
*       Load the given model
*
*******************************************************************/

static bool LoadModel( const AssetFileAssetId model_id, ResourceLoader *loader, LinearAllocator *scratch, LinearAllocator *upload, UnmappedModelBuffers *unmapped, Level *level )
{
LinearAllocatorResetToken reset_upload  = LinearAllocator_GetResetToken( upload );
LinearAllocatorResetToken reset_scratch = LinearAllocator_GetResetToken( scratch );

Render::Asset::Model read_model = {};
if( !Render::Asset::ReadModel( model_id, loader, scratch, &read_model ) )
    {
    debug_assert_always();
    return( false );
    }

*unmapped = {};
unmapped->model_id = model_id;
unmapped->vertices = (AssetFileModelVertex*)LinearAllocator_AllocateAligned( read_model.vertex_count * sizeof( *read_model.vertices ), sizeof( float ), upload );
unmapped->indices  = (AssetFileModelIndex*)LinearAllocator_AllocateAligned(  read_model.index_count  * sizeof( *read_model.indices ),  sizeof( float ), upload );

if( !unmapped->vertices
 || !unmapped->indices )
    {
    debug_assert_always();
    LinearAllocator_ResetByToken( reset_upload, upload );
    LinearAllocator_ResetByToken( reset_scratch, scratch );
    return( false );
    }

memcpy( unmapped->vertices, read_model.vertices, read_model.vertex_count * sizeof( *read_model.vertices ) );
memcpy( unmapped->indices,  read_model.indices,  read_model.index_count  * sizeof( *read_model.indices ) );

GPUAsset *new_asset = InsertModelAsset( model_id, level );
if( !new_asset)
    {
    debug_assert_always();
    LinearAllocator_ResetByToken( reset_upload, upload );
    LinearAllocator_ResetByToken( reset_scratch, scratch );
    return( false );
    }

new_asset->id = model_id;
new_asset->kind = ASSET_FILE_ASSET_KIND_MODEL;

Model *write_model = &new_asset->t.model;

write_model->vertex_count = read_model.vertex_count;
write_model->index_count  = read_model.index_count;
for( write_model->mesh_count = 0; write_model->mesh_count < read_model.mesh_count; write_model->mesh_count++ )
    {
    Mesh *write_mesh = &write_model->meshes[ write_model->mesh_count ];
    Render::Asset::Mesh *read_mesh = &read_model.meshes[ write_model->mesh_count ];

    write_mesh->index_count    = read_mesh->index_count;
    write_mesh->index_first    = read_mesh->index_first;
    write_mesh->transform      = read_mesh->transform;
    write_mesh->vertex_offset  = read_mesh->vertex_offset;
    write_mesh->material_index = write_model->material_count;

    AssetFileModelMaterial *read_material = &read_model.materials[ read_mesh->material_index ];
    Material *write_material = &write_model->materials[ write_model->material_count++ ];

    write_mesh->is_transparent = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_TRANSPARENCY );
    write_material->ordinal.t.transparent  = write_mesh->is_transparent ? 1 : 0;
    write_material->ordinal.t.albedo       = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_ALBEDO_MAP );
    write_material->ordinal.t.normal       = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_NORMAL_MAP );
    write_material->ordinal.t.emissive     = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_EMISSIVE_MAP );
    write_material->ordinal.t.metallic     = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_METALLIC_MAP );
    write_material->ordinal.t.roughness    = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_ROUGHNESS_MAP );
    write_material->ordinal.t.displacement = test_bits( read_material->bits, ASSET_FILE_MODEL_MATERIAL_BIT_DISPLACEMENT_MAP );

    for( uint32_t it_texture = 0; it_texture < cnt_of_array( read_material->textures ); it_texture++ )
        {
        AssetFileAssetId read_texture = read_material->textures[ it_texture ];
        write_material->textures[ it_texture ] = read_texture;

        bool has_texture = false;
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_ALBEDO_MAP 
                      && write_material->ordinal.t.albedo );
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_NORMAL_MAP
                      && write_material->ordinal.t.normal);
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_EMISSIVE_MAP
                      && write_material->ordinal.t.emissive);
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_METALLIC_MAP
                      && write_material->ordinal.t.metallic);
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_ROUGHNESS_MAP
                      && write_material->ordinal.t.roughness);
        has_texture |= ( it_texture == ASSET_FILE_MODEL_TEXTURES_DISPLACEMENT_MAP
                      && write_material->ordinal.t.displacement);

        if( has_texture
         && !EnsureTextureAsset( read_texture, level, NULL ) )
            {
            level->textures[ level->texture_count++ ] = read_texture;
            }
        }
    }

LinearAllocator_ResetByToken( reset_scratch, scratch );
return( true );

} /* LoadModel() */


/*******************************************************************
*
*   Release()
*
*   DESCRIPTION:
*       Release the level's resources.
*
*******************************************************************/

static void Release( Level *level )
{
pthread_join( level->load_thread, NULL );

TossTrash( &level->heap );
for( uint32_t i = 0; i < level->texture_count; i++ )
    {
    GPUAsset *texture = (GPUAsset*)HashMap_At( level->textures[ i ], &level->assets.map );
    debug_assert( texture->kind == ASSET_FILE_ASSET_KIND_TEXTURE );
    if( !texture )
        {
        continue;
        }

    TossTrash( &texture->t.texture.resource );
    }

TossTrash( &level->vb );
TossTrash( &level->ib );
TossTrash( &level->heap );

clr_struct( level );

} /* Release() */


/*******************************************************************
*
*   WriteModelsGeometry()
*
*   DESCRIPTION:
*       Upload all the model vertices and indices to the level's
*       GPU memory heap.
*
*******************************************************************/

static bool WriteModelsGeometry( const UnmappedModelBuffers *unmapped, const void *upload_base_address, LinearAllocator *heap, Core::IMResource *upload, Upload::Frame *frame, Level *level )
{
LinearAllocatorResetToken reset_heap = LinearAllocator_GetResetToken( heap );

/* create the vertex and index buffers in our heap */
uint32_t vb_element_count = GetTotalVertexCount( level );
uint32_t ib_element_count = GetTotalIndexCount( level );

if( vb_element_count == 0
 || ib_element_count == 0 )
    {
    return( true );
    }

uint64_t vb_aligned_sz = align_size_round_up( vb_element_count * sizeof( *unmapped->vertices ), D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT );
uint64_t ib_aligned_sz = align_size_round_up( ib_element_count * sizeof( *unmapped->indices ),  D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT );
void *heap_vb = LinearAllocator_AllocateAligned( vb_aligned_sz, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, heap );
void *heap_ib = LinearAllocator_AllocateAligned( ib_aligned_sz, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, heap );
if( !heap_vb
 || !heap_ib )
    {
    debug_assert_always();
    LinearAllocator_ResetByToken( reset_heap, heap );
    return( false );
    }

uint64_t heap_vb_offset = Utilities_PointerToByteOffset( LINEAR_ALLOCATOR_DETACHED_BASE_ADDRESS, heap_vb );
uint64_t heap_ib_offset = Utilities_PointerToByteOffset( LINEAR_ALLOCATOR_DETACHED_BASE_ADDRESS, heap_ib );

level->vb = Core::CreatePlacedBuffer( heap_vb_offset, vb_aligned_sz, D3D12_RESOURCE_STATE_COMMON, level->heap );
level->ib = Core::CreatePlacedBuffer( heap_ib_offset, ib_aligned_sz, D3D12_RESOURCE_STATE_COMMON, level->heap );
if( !level->vb
 || !level->ib )
    {
    ComSafeRelease( &level->vb );
    ComSafeRelease( &level->ib );
    debug_assert_always();
    LinearAllocator_ResetByToken( reset_heap, heap );
    return( false );
    }

NameD3DString( level->vb, level->name.str, L"Level::VertexBuffer" );
NameD3DString( level->ib, level->name.str, L"Level::IndexBuffer" );

D3D12_RESOURCE_BARRIER transition_vb_to_write = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST, level->vb );
frame->command_list->ResourceBarrier( 1, &transition_vb_to_write );
D3D12_RESOURCE_BARRIER transition_ib_to_write = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST, level->ib );
frame->command_list->ResourceBarrier( 1, &transition_ib_to_write );

/* copy vertices from upload buffer to the heap's vertex buffer */
uint32_t written_vertex_count = 0;
for( uint32_t it_model = 0; it_model < level->model_count; it_model++ )
    {
    GPUAsset *asset = (GPUAsset*)HashMap_At( level->models[ it_model ], &level->assets.map );
    Model *model = &asset->t.model;
    const UnmappedModelBuffers *buffers = GetUnmappedBuffersForModel( asset->id, level->model_count, unmapped );

    uint64_t read_offset = Utilities_PointerToByteOffset( upload_base_address, buffers->vertices );
    uint64_t write_offset = written_vertex_count * sizeof( *buffers->vertices );
    uint64_t copy_sz = model->vertex_count * sizeof( *buffers->vertices );

    frame->command_list->CopyBufferRegion( level->vb, write_offset, upload, read_offset, copy_sz );

    for( uint32_t it_mesh = 0; it_mesh < model->mesh_count; it_mesh++ )
        {
        model->meshes[ it_mesh ].vertex_offset = written_vertex_count;
        }

    written_vertex_count += model->vertex_count;
    }

debug_assert( written_vertex_count == vb_element_count );

/* copy indices from upload buffer to the heap's index buffer */
uint32_t written_index_count = 0;
for( uint32_t it_model = 0; it_model < level->model_count; it_model++ )
    {
    GPUAsset *asset = (GPUAsset*)HashMap_At( level->models[ it_model ], &level->assets.map );
    Model *model = &asset->t.model;
    const UnmappedModelBuffers *buffers = GetUnmappedBuffersForModel( asset->id, level->model_count, unmapped );

    uint64_t read_offset = Utilities_PointerToByteOffset( upload_base_address, buffers->indices );
    uint64_t write_offset = written_index_count * sizeof( *buffers->indices );
    uint64_t copy_sz = model->index_count * sizeof( *buffers->indices );

    frame->command_list->CopyBufferRegion( level->ib, write_offset, upload, read_offset, copy_sz );

    for( uint32_t it_mesh = 0; it_mesh < model->mesh_count; it_mesh++ )
        {
        model->meshes[ it_mesh ].index_first += written_index_count;
        }

    written_index_count += model->index_count;
    }

debug_assert( written_index_count == ib_element_count );

/* finalize */
//D3D12_RESOURCE_BARRIER transition_vb_to_read = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, level->vb );
//frame->command_list->ResourceBarrier( 1, &transition_vb_to_read );
//D3D12_RESOURCE_BARRIER transition_ib_to_read = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, level->ib );
//frame->command_list->ResourceBarrier( 1, &transition_ib_to_read );

level->vbv.BufferLocation = level->vb->GetGPUVirtualAddress();
level->vbv.StrideInBytes  = sizeof( *unmapped->vertices );
level->vbv.SizeInBytes    = written_vertex_count * level->vbv.StrideInBytes;

level->ibv.BufferLocation = level->ib->GetGPUVirtualAddress();
level->ibv.Format         = DXGI_FORMAT_R32_UINT;
level->ibv.SizeInBytes    = written_index_count * sizeof( *unmapped->indices );

return( true );

} /* WriteModelsGeometry() */


} }/* namespace D3D12::Level */