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

using namespace glm;

#define PI 3.14159265358979323846f

inline vec3 getForwardVector(mat4 rotation)
{
    return {rotation[2][0], rotation[2][1], rotation[2][2]};
}

inline vec3 getUpVector(mat4 rotation)
{
    return {rotation[1][0], rotation[1][1], rotation[1][2]};
}

inline vec3 getRightVector(mat4 rotation)
{
    return {rotation[0][0], rotation[0][1], rotation[0][2]};
}

inline quat eulerToQuat(vec3 const& eulerDegrees)
{
    const float halfToRad = 0.5f * PI / 180.0f;

    vec3 eulerRadians = eulerDegrees * halfToRad;

    float sx = std::sin(eulerRadians.x);
    float cx = std::cos(eulerRadians.x);
    float sy = std::sin(eulerRadians.y);
    float cy = std::cos(eulerRadians.y);
    float sz = std::sin(eulerRadians.z);
    float cz = std::cos(eulerRadians.z);

    quat q;
    q.x = sx * cy * cz + cx * sy * sz;
    q.y = cx * sy * cz + sx * cy * sz;
    q.z = cx * cy * sz - sx * sy * cz;
    q.w = cx * cy * cz - sx * sy * sz;

    return q;
}

inline vec3 quatToEuler(quat const& quat)
{
    float x = quat.x;
    float y = quat.y;
    float z = quat.z;
    float w = quat.w;

    float roll = 0.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;

    float test = x * y + z * w;

    if (test > 0.499999f)
    {
        roll = 0.0f;
        yaw = degrees(2.0f * std::atan2(x, w));
        pitch = 90.0f;
    }
    else if (test < -0.499999f)
    {
        roll = 0.0f;
        yaw = -degrees(2.0f * std::atan2(x, w));
        pitch = -90.0f;
    }
    else
    {
        float sqx = x * x;
        float sqy = y * y;
        float sqz = z * z;
        roll = degrees(std::atan2(2.0f * x * w - 2.0f * y * z, 1.0f - 2.0f * sqx - 2.0f * sqz));
        yaw = degrees(std::atan2(2.0f * y * w - 2.0f * x * z, 1.0f - 2.0f * sqy - 2.0f * sqz));
        pitch = degrees(std::asin(2.0f * test));
    }

    return vec3(roll, yaw, pitch);
}

template <typename T>
int signum(T val)
{
    return (T(0) < val) - (val < T(0));
}
