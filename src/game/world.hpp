#include "../common/common.hpp"
#include "../entity.hpp"
#include "../physics.hpp"

const i32 WORLD_CREATURES_COUNT = 1;
const i32 WORLD_FOOD_COUNT = 100;

struct World
{
    Entity* creatures[WORLD_CREATURES_COUNT]{};
    Entity* food[WORLD_FOOD_COUNT]{};
};
