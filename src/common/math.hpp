#pragma once

// dx11 defines
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

float remap(float source, float sourceFrom, float sourceTo, float targetFrom, float targetTo);

const char* vec3ToString(struct Arena& arena, vec3 v);
