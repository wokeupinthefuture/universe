#pragma once

#include "entity.hpp"

bool checkAABBOverlap(Entity& a, Entity& b);
bool checkCircleAABBOverlap(Entity& circle, Entity& aabb);
bool isColliding(Entity& a, Entity& b);
