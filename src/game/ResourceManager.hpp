#pragma once

#include "AssetFile.hpp"


typedef struct _ResourceManager
    {
    AssetFileReader     reader;
    } ResourceManager;

typedef struct _ResourceManagerModelStats
    {
    uint32_t            vertex_count;
    uint32_t            vertex_stride;
    uint32_t            index_count;
    uint32_t            index_stride;
    uint32_t            mesh_count;
    uint32_t            node_count;
    } ResourceManagerModelStats;

bool     ResourceManager_Destroy( ResourceManager *manager );
uint32_t ResourceManager_GetModelMeshIndices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t index_capacity, AssetFileModelIndex *indices, ResourceManager *manager );
uint32_t ResourceManager_GetModelMeshVertices( const AssetFileAssetId asset_id, const uint32_t mesh_index, const uint32_t vertex_capacity, AssetFileModelVertex *vertices, ResourceManager *manager );
bool     ResourceManager_GetModelStats( const AssetFileAssetId asset_id, ResourceManagerModelStats *out_stats, ResourceManager *manager );
bool     ResourceManager_GetShader( const AssetFileAssetId asset_id, uint32_t *sz, uint8_t *bytes, ResourceManager *manager );
bool     ResourceManager_Init( ResourceManager *manager );