#pragma once

// dx11 defines
#include "glm/ext/scalar_constants.hpp"
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/detail/qualifier.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/common.hpp"
#include "glm/trigonometric.hpp"
#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/vec2.hpp"
#include "glm/vec2.hpp"
#include "glm/vec2.hpp"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/compatibility.hpp"

using namespace glm;

#define PI 3.14159265358979323846f

vec3 getForwardVector(mat4 rotation);
vec3 getUpVector(mat4 rotation);
vec3 getRightVector(mat4 rotation);

quat eulerToQuat(vec3 const& eulerDegrees);
vec3 quatToEuler(quat const& quat);
vec3 quatToDirection(quat const& quat, vec3 viewUp = vec3(0, 1, 0));
quat directionToQuat(vec3 direction, vec3 viewUp = vec3(0, 1, 0));
vec3 directionToEuler(vec3 direction, vec3 viewUp = vec3(0, 1, 0));

vec3 matrixExtractPosition(mat4 mat);
vec3 matrixExtractScale(mat4 mat);
void matrixExtractRotation(mat4 mat, vec3 scale, quat& outRot, vec3& outEuler);
mat4 transformToMatrix(vec3 translation, quat rotation, vec3 scale);

vec3 worldToLocal(vec3 worldPoint, mat4 parentMatrix);
quat worldToLocal(quat worldRot, quat parentWorldRot);

template <typename T>
int signum(T val)
{
    return (T(0) < val) - (val < T(0));
}

inline bool isZero(vec3 v)
{
    return v.x == 0.f && v.y == 0.f && v.z == 0.f;
}

inline vec3 direction(vec3 from, vec3 to)
{
    vec3 diff = to - from;
    if (isZero(diff))
        return vec3(0.f);
    return glm::normalize(diff);
}

float remap(float source, float sourceFrom, float sourceTo, float targetFrom, float targetTo);

const char* vec2ToString(struct Arena& arena, vec2 v);
const char* vec3ToString(struct Arena& arena, vec3 v);

// screen/world coordinate conversions
vec2 screenToNdc(vec2 screenPos, vec2 screenSize);
vec4 ndcToView(vec2 ndc, float depth, mat4 invProjection);
vec4 viewToWorld(vec4 viewPos, mat4 invView);
vec3 screenToWorldRay(vec2 screenPos, vec2 screenSize, mat4 invView, mat4 invProjection);
vec3 screenToWorldPoint(vec2 screenPos, vec2 screenSize, float depth, mat4 invView, mat4 invProjection);
