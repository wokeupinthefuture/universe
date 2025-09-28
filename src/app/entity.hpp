#pragma once

#include "lib/common.hpp"
#include "renderer.hpp"

static constexpr auto MAX_ENTITIES = 10;

struct Entity
{
    const char* name = "entity";
    Entity* parent = nullptr;
    bool active = true;

    vec3 position{};
    quat rotation{};
    vec3 euler{};
    vec3 scale = {1, 1, 1};

    mat4 worldMatrixCache = mat4(1.f);
    bool isWorldMatrixDirty = true;
};

struct ECamera : Entity
{
    float fov = 45.f;
    float aspect = 1.67f;
    float nearZ = 0.1f;
    float farZ = 100.f;
    mat4 view;
    mat4 perspective;
};

struct EDrawable : Entity
{
    Renderer::ShaderType shader;
};

static inline size_t realDrawables = 0;
static inline ECamera screenCamera;
static inline EDrawable drawables[MAX_ENTITIES];

void setLocalPosition(EDrawable& entity, vec3 pos);
void addLocalPosition(EDrawable& entity, vec3 pos);
void setLocalRotation(EDrawable& entity, vec3 euler);
void setLocalScale(EDrawable& entity, vec3 scale);

void setLocalPosition(ECamera& camera, vec3 pos);
void addLocalPosition(ECamera& camera, vec3 pos);
void setLocalRotation(ECamera& camera, vec3 euler);

void updateTransform(EDrawable& entity);
