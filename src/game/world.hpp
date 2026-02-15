#include "../common/common.hpp"
#include "../entity.hpp"
#include "../physics.hpp"

struct World
{
    static constexpr auto CREATURES_COUNT = 2;
    static constexpr auto FOOD_COUNT = 250;
    static constexpr auto SEED = 123;

    Entity* creatures[CREATURES_COUNT]{};
    Entity* food[FOOD_COUNT]{};
};
