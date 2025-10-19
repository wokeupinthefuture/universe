#pragma once

#include "common/common.hpp"
#include "platform.hpp"

struct Vertex
{
    vec3 pos;
    vec3 normal;
};

enum class MeshType
{
    Triangle,
    Quad,
    Cube,
    Sphere,
    Grid,
    Custom,
    Max
};

struct Mesh
{
    Vertex* vertices;
    size_t verticesCount;
    u32* indices;
    size_t indicesCount;
    MeshType type;
    AssetID customMeshID;
    bool isIndexed;
};

Mesh generateMesh(MeshType type);
Mesh loadMesh(struct Asset const& asset, Arena& permanentMemory, Arena& tempMemory);
