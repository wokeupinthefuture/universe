#pragma once

#include "common/common.hpp"
#include "common/heap_array.hpp"

enum class EntityType
{
    Default = 1 << 0,
    Drawable = 1 << 1,
    Camera = 1 << 2
};

struct Entity
{
    const char* name;
    Entity* parent;
    bool active;
    EntityType type;

    vec3 position;
    quat rotation;
    vec3 euler;
    vec3 scale;

    mat4 worldMatrixCache;
    bool isWorldMatrixDirty;

    // camera
    float fov;
    float aspect;
    float nearZ;
    float farZ;
    mat4 view;
    mat4 perspective;

    // drawable
    struct DrawCommand* drawCommand;
};

bool sameType(Entity& entity, EntityType otherType);

struct EntityManager
{
    Entity camera;
    HeapArray<Entity> entities;
};

inline EntityManager* g_entityManager;

void setLocalPosition(Entity& entity, vec3 pos);
void addLocalPosition(Entity& entity, vec3 pos);
void setLocalRotation(Entity& entity, vec3 euler);
void setLocalScale(Entity& entity, vec3 scale);

void setLocalPosition(Entity& camera, vec3 pos);
void addLocalPosition(Entity& camera, vec3 pos);
void setLocalRotation(Entity& camera, vec3 euler);
