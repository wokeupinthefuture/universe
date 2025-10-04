#pragma once

#include <cmath>

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

template <typename T>
int signum(T val)
{
    return (T(0) < val) - (val < T(0));
}
