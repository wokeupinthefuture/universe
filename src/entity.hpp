#pragma once

#include "common/common.hpp"
#include "common/array.hpp"

struct DrawCommand;

enum class EntityType
{
    Default = BIT(0),
    Drawable = BIT(1),
    Camera = BIT(2),
    Light = BIT(3),
    Skybox = BIT(4),
};

DEFINE_ENUM_BITWISE_OPERATORS(EntityType)

enum class EntityFlag
{
    Active = BIT(0)
};

DEFINE_ENUM_BITWISE_OPERATORS(EntityFlag)

enum class LightType
{
    Directional,
    Point
};

struct Entity
{
    String name;
    Entity* parent;
    Array<Entity*> children;
    EntityFlag flags;
    EntityType type;

    vec3 position;
    quat rotation;
    vec3 euler;
    vec3 scale;

    vec3 worldPosition;
    quat worldRotation;
    vec3 worldEuler;
    vec3 worldScale;

    mat4 worldMatrixCache;
    bool isWorldMatrixDirty;

    // camera
    float defaultFov;
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
    LightType lightType;

    bool guiIsLocal;
};

struct EntityManager
{
    Entity camera;
    Array<Entity> entities;
};

void updateTransform(Entity& entity);

void setLocalPosition(Entity& entity, vec3 pos);
void addLocalPosition(Entity& entity, vec3 pos);
void setLocalRotation(Entity& entity, vec3 euler);
void addLocalRotation(Entity& entity, vec3 euler);
void setLocalScale(Entity& entity, vec3 scale);
void setLocalScale(Entity& entity, float scale);

void setWorldPosition(Entity& entity, vec3 pos);
void addWorldPosition(Entity& entity, vec3 pos);
void setWorldRotation(Entity& entity, vec3 euler);
void addLocalRotation(Entity& entity, vec3 euler);
void setWorldScale(Entity& entity, vec3 scale);

void setEntityFlag(Entity& entity, EntityFlag flag);
void setParent(Entity& entity, Entity* newParent, bool keepWorldTransform = false);

void setLightDirection(Entity& light, vec3 direction);
void setLightType(Entity& light, LightType type);
void setColor(Entity& entity, vec4 color);

vec3 getRightVector(Entity& entity);
vec3 getUpVector(Entity& entity);
vec3 getForwardVector(Entity& entity);
vec3 getWorldRightVector(Entity& entity);
vec3 getWorldUpVector(Entity& entity);
vec3 getWorldForwardVector(Entity& entity);

bool hasType(Entity const& entity, EntityType type);
