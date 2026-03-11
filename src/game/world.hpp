#include "../common/common.hpp"
#include "../entity.hpp"
#include "../physics.hpp"

struct World
{
    static constexpr auto ENEMIES_COUNT = 2;
    // static constexpr auto PLAYER_PROJECTILES_COUNT = 100;
    static constexpr auto FOOD_COUNT = 250;
    static constexpr auto SEED = 123;

    Entity* player{};
    // Entity* playerProjectiles[PLAYER_PROJECTILES_COUNT]{};
    Entity* enemies[ENEMIES_COUNT]{};
    Entity* food[FOOD_COUNT]{};
};
