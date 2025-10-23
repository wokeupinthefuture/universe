#pragma once

#include "platform.hpp"

struct Vertex
{
    vec3 pos;
    vec3 normal;
};

static constexpr String MeshTypeName[] = {strFromLiteral("Triangle"),
    strFromLiteral("Quad"),
    strFromLiteral("Cube"),
    strFromLiteral("Sphere"),
    strFromLiteral("Grid"),
    strFromLiteral("Custom"),
    strFromLiteral("Max")};

enum class GeneratedMesh
{
    Triangle,
    Quad,
    Cube,
    Sphere,
    Grid,
    Max
};

enum class MeshFlags
{
    Indexed = BIT(0),
    Generated = BIT(1),
};

constexpr MeshFlags operator|(MeshFlags lhs, MeshFlags rhs);
constexpr MeshFlags& operator|=(MeshFlags& lhs, MeshFlags rhs);
constexpr MeshFlags operator&(MeshFlags lhs, MeshFlags rhs);
constexpr MeshFlags& operator&=(MeshFlags& lhs, MeshFlags rhs);
constexpr MeshFlags operator^(MeshFlags lhs, MeshFlags rhs);
constexpr MeshFlags& operator^=(MeshFlags& lhs, MeshFlags rhs);
constexpr MeshFlags operator~(MeshFlags rhs);

struct Mesh
{
    Vertex* vertices;
    size_t verticesCount;
    u32* indices;
    size_t indicesCount;
    MeshFlags flags;
    size_t id;
    String name;
};

Mesh generateMesh(GeneratedMesh type, Arena& tempMemory);
Mesh loadMesh(struct Asset const& asset, Arena& permanentMemory, Arena& tempMemory);
