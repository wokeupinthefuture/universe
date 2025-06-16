#pragma once

#include <cmath>
#include "glm/detail/qualifier.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/common.hpp"
#include "glm/trigonometric.hpp"
#include "glm/geometric.hpp"

using namespace glm;

#define rlv3(v) *reinterpret_cast<const Vector3*>(&v.x)
#define glmv3(v) *reinterpret_cast<const vec3*>(&v.x)

inline vec3 rayPlaneIntersectionResult(
    vec3 rayOrigin, vec3 rayDirection, float distanceToPlane, vec3 planeOrigin, vec3 planeNormal = vec3(0, 0, 1))
{
    // const auto denom = dot(planeNormal, rayDirection);

    // if (std::abs(denom) > 1e-6f)
    //     distanceToPlane = dot(planeNormal, planeOrigin - rayOrigin) / denom;

    return rayOrigin + rayDirection * distanceToPlane;
}
