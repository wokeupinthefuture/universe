#include "entity.hpp"
#include "context.hpp"

static mat4 calculateWorldTransform(Entity& entity)
{
    if (!entity.isWorldMatrixDirty && !entity.parent)
        return entity.worldMatrixCache;

    auto localMatrix = transformToMatrix(entity.position, entity.rotation, entity.scale);

    if (entity.parent)
    {
        auto parentWorld = calculateWorldTransform(*entity.parent);
        entity.worldMatrixCache = parentWorld * localMatrix;
    }
    else
    {
        entity.worldMatrixCache = localMatrix;
    }

    entity.isWorldMatrixDirty = false;

    entity.worldPosition = matrixExtractPosition(entity.worldMatrixCache);
    entity.worldScale = matrixExtractScale(entity.worldMatrixCache);
    ENSURE(entity.worldScale.x > 0.f && entity.worldScale.y > 0.f && entity.worldScale.z > 0.f);
    matrixExtractRotation(entity.worldMatrixCache, entity.worldScale, entity.worldRotation, entity.worldEuler);

    return entity.worldMatrixCache;
}

static void updateShaderMVP(Entity& entity, Entity& camera)
{
    const auto model = entity.worldMatrixCache;
    const auto view = camera.view;
    const auto projection = camera.perspective;
    ENSURE(entity.drawCommand != nullptr);
    if (entity.drawCommand->shader == ShaderType::Basic)
        setShaderVariableMat4(*entity.drawCommand, "world", transpose(model));
    setShaderVariableMat4(*entity.drawCommand, "mvp", transpose(projection * view * model));
}

static void calculateCameraView(Entity& camera)
{
    const auto rotation = glm::toMat4(camera.worldRotation);
    const auto target = camera.worldPosition + getForwardVector(rotation);
    const auto up = getUpVector(rotation);
    camera.view = lookAtLH(camera.worldPosition, target, up);
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
    calculateWorldTransform(entity);

    ENSURE(g_context != nullptr);
    if (hasType(entity, EntityType::Camera))
    {
        calculateCameraTransform(entity, g_context->entityManager.entities);
    }
    else if (hasType(entity, EntityType::Drawable))
    {
        updateShaderMVP(entity, g_context->entityManager.camera);
    }

    if (hasType(entity, EntityType::Light))
    {
        if (entity.lightType == LightType::Directional)
            return;

        for (const auto& drawable : g_context->entityManager.entities)
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

void setLocalPosition(Entity& entity, vec3 pos)
{
    entity.position = pos;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addLocalPosition(Entity& entity, vec3 pos)
{
    setLocalPosition(entity, entity.position + pos);
}

void setLocalScale(Entity& entity, vec3 scale)
{
    scale.x = std::max(scale.x, 0.00001f);
    scale.y = std::max(scale.y, 0.00001f);
    scale.z = std::max(scale.z, 0.00001f);

    entity.scale = scale;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void setLocalRotation(Entity& entity, vec3 rot)
{
    if (rot.x > 360.f)
        rot.x -= 360.f;
    if (rot.y > 360.f)
        rot.y -= 360.f;
    if (rot.z > 360.f)
        rot.z -= 360.f;
    if (rot.x < -360.f)
        rot.x += 360.f;
    if (rot.y < -360.f)
        rot.y += 360.f;
    if (rot.z < -360.f)
        rot.z += 360.f;

    entity.rotation = eulerToQuat(rot);
    entity.euler = rot;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addLocalRotation(Entity& entity, vec3 euler)
{
    setLocalRotation(entity, entity.euler + euler);
}

void setWorldPosition(Entity& entity, vec3 pos)
{
    if (!entity.parent)
    {
        entity.position = pos;
    }
    else
    {
        entity.position = worldToLocal(pos, entity.parent->worldMatrixCache);
    }

    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addWorldPosition(Entity& entity, vec3 pos)
{
    setWorldPosition(entity, entity.worldPosition + pos);
}

void setWorldRotation(Entity& entity, vec3 rot)
{
    if (rot.x > 360.f)
        rot.x -= 360.f;
    if (rot.y > 360.f)
        rot.y -= 360.f;
    if (rot.z > 360.f)
        rot.z -= 360.f;
    if (rot.x < -360.f)
        rot.x += 360.f;
    if (rot.y < -360.f)
        rot.y += 360.f;
    if (rot.z < -360.f)
        rot.z += 360.f;

    if (!entity.parent)
    {
        entity.rotation = eulerToQuat(rot);
        entity.euler = rot;
    }
    else
    {
        const auto worldRotation = eulerToQuat(rot);
        entity.rotation = worldToLocal(worldRotation, entity.parent->worldMatrixCache);
        entity.euler = quatToEuler(entity.rotation);
    }

    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addWorldRotation(Entity& entity, vec3 euler)
{
    setWorldRotation(entity, entity.worldEuler + euler);
}

void setWorldScale(Entity& entity, vec3 scale)
{
    scale.x = std::max(scale.x, 0.00001f);
    scale.y = std::max(scale.y, 0.00001f);
    scale.z = std::max(scale.z, 0.00001f);

    if (!entity.parent)
    {
        entity.scale = scale;
    }
    else
    {
        entity.scale = scale / entity.parent->worldScale;
    }

    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

static void findEraseChild(Entity& parent, Entity& child)
{
    const auto oldParentChildSlot =
        findIndex(parent.children, MAX_ENTITY_CHILDREN, [&](auto& otherChild) { return otherChild == &child; });
    ENSURE(oldParentChildSlot != -1);
    parent.children[oldParentChildSlot] = nullptr;
}

void setParent(Entity& entity, Entity* newParent, bool keepWorldTransform)
{
    if (!newParent)
    {
        if (entity.parent)
            findEraseChild(*entity.parent, entity);
        entity.parent = nullptr;

        entity.isWorldMatrixDirty = true;
        updateTransform(entity);

        return;
    }

    if (&entity == newParent)
        return;

    if (entity.parent && newParent == entity.parent)
        return;

    const auto newChildSlot = findIndex(newParent->children, MAX_ENTITY_CHILDREN, [](auto& child) { return child == nullptr; });
    if (newChildSlot == -1)
        return;

    if (entity.parent)
        findEraseChild(*entity.parent, entity);

    entity.parent = &*newParent;
    newParent->children[newChildSlot] = &entity;

    if (!keepWorldTransform)
    {
        entity.position = {};
        entity.rotation = {};
        entity.euler = {};
        entity.scale = vec3(1.f, 1.f, 1.f);
    }

    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void setColor(Entity& entity, vec4 color)
{
    ENSURE(g_context);
    setShaderVariableVec4(*entity.drawCommand, "objectColor", color);
    if (hasType(entity, EntityType::Light))
    {
        for (auto& other : g_context->entityManager.entities)
        {
            if (hasType(other, EntityType::Drawable) && other.drawCommand->shader == ShaderType::Basic)
                setShaderVariableVec4(*other.drawCommand, "lightColor", color);
        }
    }
}

void setLightType(Entity& light, LightType type)
{
    ENSURE(g_context);
    ENSURE(hasType(light, EntityType::Light));

    light.lightType = type;

    for (auto& other : g_context->entityManager.entities)
    {
        if (hasType(other, EntityType::Drawable) && other.drawCommand->shader == ShaderType::Basic)
            setShaderVariableInt(*other.drawCommand, "lightType", (i32)type);
    }
}

void setLightDirection(Entity& light, vec3 direction)
{
    ENSURE(g_context);
    ENSURE(hasType(light, EntityType::Light));

    light.lightDirection = direction;

    for (const auto& entity : g_context->entityManager.entities)
    {
        if (hasType(entity, EntityType::Drawable) && entity.drawCommand->shader == ShaderType::Basic)
        {
            setShaderVariableVec3(*entity.drawCommand, "lightDirection", light.lightDirection);
        }
    }
}

void setEntityFlag(Entity& entity, EntityFlag flag)
{
    const auto processSingleFlag = [&entity, flag](EntityFlag single, void (*onChanged)(Entity& entity, bool isSet))
    {
        bool wasSet = bool(entity.flags & single);
        bool isSet = bool(flag & single);

        if (isSet)
            entity.flags |= flag;
        else
            entity.flags &= flag;

        if (isSet != wasSet)
        {
            onChanged(entity, isSet);
        }
    };

    processSingleFlag(EntityFlag::Active,
        [](Entity& entity, bool isSet)
        {
            if (entity.drawCommand)
            {
                if (isSet)
                    entity.drawCommand->flags |= DrawFlag::Active;
                else
                    entity.drawCommand->flags &= ~(DrawFlag::Active);
            }
        });

    for (const auto& child : entity.children)
    {
        if (!child)
            continue;
        setEntityFlag(*child, flag);
    }
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

vec3 getWorldForwardVector(Entity& entity)
{
    return getForwardVector(toMat4(entity.worldRotation));
}

vec3 getWorldRightVector(Entity& entity)
{
    return getRightVector(toMat4(entity.worldRotation));
}

vec3 getWorldUpVector(Entity& entity)
{
    return getUpVector(toMat4(entity.worldRotation));
}
