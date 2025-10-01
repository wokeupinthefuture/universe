#pragma once

#include "common/common.hpp"

struct Vertex
{
    vec3 pos;
};

static constexpr Vertex TRIANGLE_VERTICES[] = {
    {.pos = vec3{-0.5f, -0.5f, 0.f}}, {.pos = vec3{0.f, 0.5f, 0.f}}, {.pos = vec3{0.5f, -0.5f, 0.f}}};
static constexpr u32 TRIANGLE_INDICES[] = {0, 1, 2};

static constexpr Vertex QUAD_VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.f}},
    {.pos = vec3{-0.5f, 0.5f, 0.f}},
    {.pos = vec3{0.5f, 0.5f, 0.f}},
    {.pos = vec3{0.5f, -0.5f, 0.f}}};
static constexpr u32 QUAD_INDICES[] = {0, 1, 2, 2, 3, 0};

static constexpr auto SPHERE_STACKS = 16;
static constexpr auto SPHERE_SLICES = 16;
static constexpr auto SPHERE_RADIUS = 1.f;
inline Vertex SPHERE_VERTICES[SPHERE_STACKS * SPHERE_SLICES] = {};
inline u32 SPHERE_INDICES[SPHERE_STACKS * SPHERE_SLICES * 6] = {};
void generateSphere(
    float radius, u32 stacks, u32 slices, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount);

static constexpr auto GRID_X = 10;
static constexpr auto GRID_Y = 10;
static constexpr auto GRID_SPACING = 5.f;
inline Vertex GRID_VERTICES[GRID_X * GRID_Y] = {};
inline u32 GRID_INDICES[GRID_X * GRID_Y * 4] = {};
void generateGrid(
    i32 gridX, i32 gridY, float spacing, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount);

enum class MeshType
{
    Triangle,
    Quad,
    Sphere,
    Grid,
    Max
};
