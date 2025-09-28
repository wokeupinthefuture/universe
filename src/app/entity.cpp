#include "entity.hpp"
#include "app/renderer.hpp"
#include "lib/log.hpp"
#include "lib/math.hpp"

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

static void matrixToTransform(mat4 mat, vec3& outPos, vec3& outScale, quat& outRot, vec3& outEuler)
{
    outPos = transformMatrixGetPos(mat);
    outScale = transformMatrixGetScale(mat);
    transformMatrixGetRotation(mat, outScale, outRot, outEuler);
}

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

    return entity.worldMatrixCache;
}

static void calculateCameraView(ECamera& camera)
{
    const auto rotation = glm::toMat4(camera.rotation);
    const auto target = camera.position + getForwardVector(rotation);
    const auto up = getUpVector(rotation);
    camera.view = lookAtLH(camera.position, target, up);
}

static void calculateCameraTransform(ECamera& camera)
{
    calculateCameraView(camera);
    for (size_t i = 0; i < realDrawables; ++i)
        updateTransform(drawables[i]);
}

static void calculateCameraProjection(ECamera& camera)
{
    camera.perspective = perspectiveLH(radians(camera.fov), camera.aspect, camera.nearZ, camera.farZ);
    calculateCameraTransform(camera);
}

void updateTransform(EDrawable& entity)
{
    const auto model = getWorldMatrix(entity);
    const auto view = screenCamera.view;
    const auto projection = screenCamera.perspective;
    Renderer::setShaderVariableMat4(entity.shader, "mvp", transpose(projection * view * model));
}

void addLocalPosition(EDrawable& entity, vec3 pos)
{
    setLocalPosition(entity, entity.position + pos);
}

void setLocalPosition(EDrawable& entity, vec3 pos)
{
    entity.position = pos;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void setLocalScale(EDrawable& entity, vec3 scale)
{
    entity.scale = scale;
    entity.isWorldMatrixDirty = true;
    updateTransform(entity);
}

void addLocalPosition(ECamera& camera, vec3 pos)
{
    setLocalPosition(camera, camera.position + pos);
}

void setLocalPosition(ECamera& camera, vec3 pos)
{
    camera.position = pos;
    camera.isWorldMatrixDirty = true;
    calculateCameraTransform(camera);
}
