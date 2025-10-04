#pragma once

#include "common/common.hpp"
#include "common/heap_array.hpp"

enum class EntityType
{
    Default = 1 << 0,
    Drawable = 1 << 1,
    Camera = 1 << 2,
    Light = 1 << 3
};

enum class LightType
{
    Directional,
    Point
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

    // light
    vec3 lightDirection;
    vec3 lightColor;
    LightType lightType;
};

struct EntityManager
{
    Entity camera;
    HeapArray<Entity> entities;
};

inline EntityManager* g_entityManager;

void updateTransform(Entity& entity, Entity& camera);

void setLocalPosition(Entity& entity, vec3 pos);
void addLocalPosition(Entity& entity, vec3 pos);
void setLocalRotation(Entity& entity, vec3 euler);
void addLocalRotation(Entity& entity, vec3 euler);
void setLocalScale(Entity& entity, vec3 scale);

vec3 getRightVector(Entity& entity);
vec3 getUpVector(Entity& entity);
vec3 getForwardVector(Entity& entity);

bool hasType(Entity const& entity, EntityType type);

constexpr EntityType operator|(EntityType lhs, EntityType rhs);
constexpr EntityType& operator|=(EntityType& lhs, EntityType rhs);

constexpr EntityType operator&(EntityType lhs, EntityType rhs);
constexpr EntityType& operator&=(EntityType& lhs, EntityType rhs);

constexpr EntityType operator^(EntityType lhs, EntityType rhs);
constexpr EntityType& operator^=(EntityType& lhs, EntityType rhs);

constexpr EntityType operator~(EntityType rhs);
