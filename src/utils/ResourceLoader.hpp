#pragma once

#include "AssetFile.hpp"


typedef struct _ResourceLoader
    {
    AssetFileReader     reader;
    } ResourceLoader;

typedef struct _ResourceLoaderModelStats
    {
    uint32_t            vertex_count;
    uint32_t            vertex_stride;
    uint32_t            index_count;
    uint32_t            index_stride;
    uint32_t            mesh_count;
    uint32_t            node_count;
    } ResourceLoaderModelStats;

bool     ResourceLoader_Destroy( ResourceLoader *loader );
uint32_t ResourceLoader_GetModelMeshIndices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t index_capacity, AssetFileModelIndex *indices, ResourceLoader *loader );
uint32_t ResourceLoader_GetModelMeshNodes( const AssetFileAssetId asset_id, const uint32_t node_capacity, AssetFileModelNode *nodes, ResourceLoader *loader );
uint32_t ResourceLoader_GetModelMeshVertices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t vertex_capacity, AssetFileModelVertex *vertices, ResourceLoader *loader );
bool     ResourceLoader_GetModelStats( const AssetFileAssetId asset_id, ResourceLoaderModelStats *out_stats, ResourceLoader *loader );
bool     ResourceLoader_GetShader( const AssetFileAssetId asset_id, uint32_t *sz, uint8_t *bytes, ResourceLoader *loader );
bool     ResourceLoader_Init( ResourceLoader *loader );