#pragma once

#include "common/common.hpp"

struct Vertex
{
    vec3 pos;
    vec3 normal;
};

static constexpr Vertex TRIANGLE_VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.0f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}}};
static constexpr u32 TRIANGLE_INDICES[] = {0, 1, 2};

static constexpr Vertex QUAD_VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{-0.5f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.5f, 0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.5f, -0.5f, 0.0f}, .normal = vec3{0.0f, 0.0f, -1.0f}}};
static constexpr u32 QUAD_INDICES[] = {0, 1, 2, 2, 3, 0};

static constexpr auto CUBE_SIZE = 1.f;
inline Vertex CUBE_VERTICES[] = {
    // Front face (z = 0.5)
    {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
    {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
    {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
    {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 0.0f, 1.0f}},
    // Back face (z = -0.5)
    {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, 0.0f, -1.0f}},
    // Left face (x = -0.5)
    {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
    {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
    {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
    {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{-1.0f, 0.0f, 0.0f}},
    // Right face (x = 0.5)
    {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
    {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
    {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
    {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{1.0f, 0.0f, 0.0f}},
    // Top face (y = 0.5)
    {.pos = vec3{-0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
    {.pos = vec3{-0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
    {.pos = vec3{0.5f, 0.5f, 0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
    {.pos = vec3{0.5f, 0.5f, -0.5f}, .normal = vec3{0.0f, 1.0f, 0.0f}},
    // Bottom face (y = -0.5)
    {.pos = vec3{-0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
    {.pos = vec3{0.5f, -0.5f, -0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
    {.pos = vec3{0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}},
    {.pos = vec3{-0.5f, -0.5f, 0.5f}, .normal = vec3{0.0f, -1.0f, 0.0f}}};

// Indices for 6 faces (12 triangles)
static constexpr uint32_t CUBE_INDICES[] = {
    0,
    1,
    2,
    0,
    2,
    3,  // Front
    4,
    5,
    6,
    4,
    6,
    7,  // Back
    8,
    9,
    10,
    8,
    10,
    11,  // Left
    12,
    13,
    14,
    12,
    14,
    15,  // Right
    16,
    17,
    18,
    16,
    18,
    19,  // Top
    20,
    21,
    22,
    20,
    22,
    23  // Bottom
};

static constexpr auto SPHERE_STACKS = 16;
static constexpr auto SPHERE_SLICES = 16;
static constexpr auto SPHERE_RADIUS = 1.f;
inline Vertex SPHERE_VERTICES[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1)] = {};
inline u32 SPHERE_INDICES[(SPHERE_STACKS + 1) * (SPHERE_SLICES + 1) * 6] = {};
void generateSphere(
    float radius, u32 stacks, u32 slices, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount);

static constexpr auto GRID_X = 100;
static constexpr auto GRID_Y = 100;
static constexpr auto GRID_SPACING = 5.f;
inline Vertex GRID_VERTICES[GRID_X * GRID_Y] = {};
inline u32 GRID_INDICES[GRID_X * GRID_Y * 4] = {};
void generateGrid(
    i32 gridX, i32 gridY, float spacing, Vertex* outVertices, size_t verticesCount, u32* outIndices, size_t indicesCount);

enum class MeshType
{
    Triangle,
    Quad,
    Cube,
    Sphere,
    Grid,
    Max
};
