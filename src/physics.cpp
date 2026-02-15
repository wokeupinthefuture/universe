#include "physics.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "platform.hpp"

static bool isCircle(Entity& e)
{
    return e.drawCommand && (bool)(e.drawCommand->shaderTraits & ShaderTrait::Circle);
}

bool checkAABBOverlap(Entity& a, Entity& b)
{
    vec2 aPos = vec2(a.worldPosition.x, a.worldPosition.y);
    vec2 bPos = vec2(b.worldPosition.x, b.worldPosition.y);
    vec2 aHalf = vec2(a.worldScale.x, a.worldScale.y) * 0.5f;
    vec2 bHalf = vec2(b.worldScale.x, b.worldScale.y) * 0.5f;

    return (aPos.x - aHalf.x) < (bPos.x + bHalf.x) && (aPos.x + aHalf.x) > (bPos.x - bHalf.x) &&
           (aPos.y - aHalf.y) < (bPos.y + bHalf.y) && (aPos.y + aHalf.y) > (bPos.y - bHalf.y);
}

bool checkCircleAABBOverlap(Entity& circle, Entity& aabb)
{
    vec2 circleCenter = vec2(circle.worldPosition.x, circle.worldPosition.y);
    float radius = glm::min(circle.worldScale.x, circle.worldScale.y) * 0.5f;

    vec2 aabbCenter = vec2(aabb.worldPosition.x, aabb.worldPosition.y);
    vec2 aabbHalf = vec2(aabb.worldScale.x, aabb.worldScale.y) * 0.5f;
    vec2 aabbMin = aabbCenter - aabbHalf;
    vec2 aabbMax = aabbCenter + aabbHalf;

    vec2 closest = glm::clamp(circleCenter, aabbMin, aabbMax);
    vec2 diff = circleCenter - closest;
    float dist2 = glm::dot(diff, diff);

    return dist2 < radius * radius;
}

bool isColliding(Entity& a, Entity& b)
{
    ENSURE(entityHasTrait(a, EntityTrait::AABB) && entityHasTrait(b, EntityTrait::AABB));

    if (!isActive(a) || !isActive(b))
        return false;

    bool aCircle = isCircle(a);
    bool bCircle = isCircle(b);

    if (aCircle && !bCircle)
        return checkCircleAABBOverlap(a, b);
    if (bCircle && !aCircle)
        return checkCircleAABBOverlap(b, a);

    return checkAABBOverlap(a, b);
}
