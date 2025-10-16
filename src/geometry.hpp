#pragma once

#include "common/common.hpp"

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
    Max
};

struct Mesh
{
    Vertex* vertices;
    size_t verticesCount;
    u32* indices;
    size_t indicesCount;
    MeshType type;
    bool isIndexed;
};

Mesh generateMesh(MeshType type);
