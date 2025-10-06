#include "entity.hpp"
#include "common/math.hpp"
#include "renderer.hpp"

static vec3 transformMatrixGetPos(mat4 mat)
{
    return {mat[3][0], mat[3][1], mat[3][2]};
}

static vec3 transformMatrixGetScale(mat4 mat)
{
    vec3 scale{};
    scale.x = std::sqrt(mat[1][0] * mat[0][0] + mat[1][0] * mat[1][0] + mat[2][0] * mat[2][0]) * signum(mat[0][0]);
    scale.y = std::sqrt(mat[0][1] * mat[0][1] + mat[1][1] * mat[1][1] + mat[2][1] * mat[2][1]) * signum(mat[1][1]);
    scale.z = std::sqrt(mat[0][2] * mat[0][2] + mat[1][2] * mat[1][2] + mat[2][2] * mat[2][2]) * signum(mat[2][2]);
    return scale;
}

static void transformMatrixGetRotation(mat4 mat, vec3 scale, quat& outRot, vec3& outEuler)
{
    auto rotationMatrix = mat;
    rotationMatrix[3][0] = 0;
    rotationMatrix[3][1] = 0;
    rotationMatrix[3][2] = 0;
    rotationMatrix[0] /= vec4{scale.x, scale.y, scale.z, 1.0};
    rotationMatrix[1] /= vec4{scale.x, scale.y, scale.z, 1.0};
    rotationMatrix[2] /= vec4{scale.x, scale.y, scale.z, 1.0};
    outRot = toQuat(rotationMatrix);
    outEuler = quatToEuler(outRot);
}

// static void matrixToTransform(mat4 mat, vec3& outPos, vec3& outScale, quat& outRot, vec3& outEuler)
// {
//     outPos = transformMatrixGetPos(mat);
//     outScale = transformMatrixGetScale(mat);
//     transformMatrixGetRotation(mat, outScale, outRot, outEuler);
// }

static mat4 transformToMatrix(vec3 pos, quat rot, vec3 scale_)
{
    const auto scaleMatrix = scale(mat4(1.f), scale_);
    const auto rotMatrix = toMat4(rot);
    const auto posMatrix = translate(mat4(1.f), pos);
    return posMatrix * rotMatrix * scaleMatrix;
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

static void calculateCameraView(Entity& camera)
{
    const auto rotation = glm::toMat4(camera.rotation);
    const auto target = camera.position + getForwardVector(rotation);
    const auto up = getUpVector(rotation);
    camera.view = lookAtLH(camera.position, target, up);
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

static void calculateCameraTransform(Entity& camera, HeapArray<Entity> entities)
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

void calculateCameraProjection(Entity& camera, HeapArray<Entity> entities)
{
    camera.perspective = perspectiveLH(radians(camera.fov), camera.aspect, camera.nearZ, camera.farZ);
    calculateCameraTransform(camera, entities);
}

void updateTransform(Entity& entity)
{
    ENSURE(g_entityManager != nullptr);
    if (hasType(entity, EntityType::Camera))
    {
        calculateCameraTransform(entity, g_entityManager->entities);
    }
    else if (hasType(entity, EntityType::Drawable))
    {
        updateShaderMVP(entity, g_entityManager->camera);
    }

    const auto worldMatrix = entity.worldMatrixCache;
    entity.worldPosition = transformMatrixGetPos(worldMatrix);
    entity.worldScale = transformMatrixGetScale(worldMatrix);
    transformMatrixGetRotation(worldMatrix, entity.worldScale, entity.worldRotation, entity.worldEuler);

    if (hasType(entity, EntityType::Light))
    {
        if (entity.lightType == LightType::Directional)
            return;

        for (const auto& drawable : g_entityManager->entities)
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
