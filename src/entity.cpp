#include "entity.hpp"
#include "common/math.hpp"
#include "renderer.hpp"

static EntityManager* manager;
void setInternalPointer(EntityManager* _manager)
{
    manager = _manager;
}

HeapArray<Entity>& getEntities()
{
    ENSURE(manager);
    return manager->entities;
}

static mat4 getWorldMatrix(Entity& entity)
{
    if (!entity.isWorldMatrixDirty && !entity.parent)
        return entity.worldMatrixCache;

    auto localMatrix = transformToMatrix(entity.position, entity.rotation, entity.scale);

    if (entity.parent)
    {
        auto parentWorld = getWorldMatrix(*entity.parent);
        entity.worldMatrixCache = parentWorld * localMatrix;
    }
    else
    {
        entity.worldMatrixCache = localMatrix;
    }

    entity.isWorldMatrixDirty = false;
    return entity.worldMatrixCache;
}

static void updateShaderMVP(Entity& entity, Entity& camera)
{
    const auto model = getWorldMatrix(entity);  // aka world matrix
    const auto view = camera.view;
    const auto projection = camera.perspective;
    ENSURE(entity.drawCommand != nullptr);
    if (entity.drawCommand->shader == ShaderType::Basic)
        setShaderVariableMat4(*entity.drawCommand, "world", transpose(model));
    setShaderVariableMat4(*entity.drawCommand, "mvp", transpose(projection * view * model));
}

static void calculateCameraView(Entity& camera)
{
    const auto rotation = glm::toMat4(camera.rotation);
    const auto target = camera.position + getForwardVector(rotation);
    const auto up = getUpVector(rotation);
    camera.view = lookAtLH(camera.position, target, up);
}

void calculateCameraTransform(Entity& camera, HeapArray<Entity> entities)
{
    calculateCameraView(camera);
    for (size_t i = 0; i < entities.size; ++i)
    {
        auto& entity = entities[i];
        if (hasType(entity, EntityType::Drawable))
        {
            updateShaderMVP(entity, camera);
        }
    }
}

void calculateCameraProjection(Entity& camera, vec2 screenSize, HeapArray<Entity> entities)
{
    camera.aspect = screenSize.x / screenSize.y;
    auto fov = camera.defaultFov;
    if (camera.aspect < 1.f)
        fov *= 1 / camera.aspect;
    camera.fov = fov;
    camera.perspective = perspectiveLH(radians(camera.fov), camera.aspect, camera.nearZ, camera.farZ);
    calculateCameraTransform(camera, entities);
}

void updateTransform(Entity& entity)
{
    ENSURE(manager != nullptr);
    if (hasType(entity, EntityType::Camera))
    {
        calculateCameraTransform(entity, manager->entities);
    }
    else if (hasType(entity, EntityType::Drawable))
    {
        updateShaderMVP(entity, manager->camera);
    }

    const auto worldMatrix = entity.worldMatrixCache;
    entity.worldPosition = matrixExtractPosition(worldMatrix);
    entity.worldScale = matrixExtractScale(worldMatrix);
    matrixExtractRotation(worldMatrix, entity.worldScale, entity.worldRotation, entity.worldEuler);

    if (hasType(entity, EntityType::Light))
    {
        if (entity.lightType == LightType::Directional)
            return;

        for (const auto& drawable : manager->entities)
        {
            if (hasType(drawable, EntityType::Drawable))
            {
                ENSURE(drawable.drawCommand != nullptr);
                if (drawable.drawCommand->shader == ShaderType::Basic)
                    setShaderVariableVec3(*drawable.drawCommand, "lightPosition", entity.worldPosition);
            }
        }
    }

    for (auto& child : entity.children)
        if (child)
            updateTransform(*child);
}

bool hasType(Entity const& entity, EntityType type)
{
    return bool(entity.type & type);
}

void addLocalPosition(Entity& entity, vec3 pos)
{
    setLocalPosition(entity, entity.position + pos);
}

void setLocalPosition(Entity& entity, vec3 pos)
{
    entity.position = pos;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void setLocalScale(Entity& entity, vec3 scale)
{
    entity.scale = scale;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void setLocalRotation(Entity& entity, vec3 rot)
{
    entity.rotation = eulerToQuat(rot);
    entity.euler = rot;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addLocalRotation(Entity& entity, vec3 euler)
{
    setLocalRotation(entity, entity.euler + euler);
}

void setParent(Entity& entity, Entity& parent)
{
    const auto childSlot = findIndex(parent.children, MAX_ENTITY_CHILDREN, [](auto& child) { return child == nullptr; });
    if (childSlot == -1)
        return;

    entity.parent = &parent;
    parent.children[childSlot] = &entity;

    updateTransform(entity);
}

vec3 getForwardVector(Entity& entity)
{
    return getForwardVector(toMat4(entity.rotation));
}
vec3 getRightVector(Entity& entity)
{
    return getRightVector(toMat4(entity.rotation));
}
vec3 getUpVector(Entity& entity)
{
    return getUpVector(toMat4(entity.rotation));
}

constexpr EntityType operator|(EntityType lhs, EntityType rhs)
{
    return static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) | static_cast<std::underlying_type_t<EntityType>>(rhs));
}

constexpr EntityType& operator|=(EntityType& lhs, EntityType rhs)
{
    lhs = static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) | static_cast<std::underlying_type_t<EntityType>>(rhs));
    return lhs;
}

constexpr EntityType operator&(EntityType lhs, EntityType rhs)
{
    return static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) & static_cast<std::underlying_type_t<EntityType>>(rhs));
}

constexpr EntityType& operator&=(EntityType& lhs, EntityType rhs)
{
    lhs = static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) & static_cast<std::underlying_type_t<EntityType>>(rhs));
    return lhs;
}

constexpr EntityType operator^(EntityType lhs, EntityType rhs)
{
    return static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) ^ static_cast<std::underlying_type_t<EntityType>>(rhs));
}

constexpr EntityType& operator^=(EntityType& lhs, EntityType rhs)
{
    lhs = static_cast<EntityType>(
        static_cast<std::underlying_type_t<EntityType>>(lhs) ^ static_cast<std::underlying_type_t<EntityType>>(rhs));
    return lhs;
}

constexpr EntityType operator~(EntityType rhs)
{
    return static_cast<EntityType>(~static_cast<std::underlying_type_t<EntityType>>(rhs));
}
