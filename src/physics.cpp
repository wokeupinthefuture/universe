#include "physics.hpp"
#include "entity.hpp"
#include "platform.hpp"

bool checkAABBOverlap(Entity& a, Entity& b)
{
    vec2 aPos = vec2(a.worldPosition.x, a.worldPosition.y);
    vec2 bPos = vec2(b.worldPosition.x, b.worldPosition.y);
    vec2 aHalf = vec2(a.worldScale.x, a.worldScale.y) * 0.5f;
    vec2 bHalf = vec2(b.worldScale.x, b.worldScale.y) * 0.5f;

    return (aPos.x - aHalf.x) < (bPos.x + bHalf.x) && (aPos.x + aHalf.x) > (bPos.x - bHalf.x) &&
           (aPos.y - aHalf.y) < (bPos.y + bHalf.y) && (aPos.y + aHalf.y) > (bPos.y - bHalf.y);
}

bool isColliding(Entity& a, Entity& b)
{
    ENSURE(entityHasTrait(a, EntityTrait::AABB) && entityHasTrait(b, EntityTrait::AABB));
    return isActive(a) && isActive(b) && checkAABBOverlap(a, b);
}
