#include "context.hpp"
#include "common/array.hpp"

const i32 MAX_ASSETS = 25;
const i32 MAX_ENTITIES = 3000;
const i32 MAX_DRAW_COMMANDS = 3000;
static void initGameArrays(Context& context)
{
    arrayInit(context.render.drawCommands, MAX_DRAW_COMMANDS, context.gameMemory);
    arrayInit(context.entityManager.entities, MAX_ENTITIES, context.gameMemory);
    arrayInit(context.render.meshes, MAX_ASSETS, context.gameMemory);
    arrayInit(context.render.allTextures, MAX_ASSETS, context.gameMemory);
}

void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    context.render.needsToResize = true;

    arenaInit(context.platformMemory, memorySize);
    arenaInit(context.gameMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);

    initGameArrays(context);

    arrayInit(context.platform.assets[(i32)AssetType::ObjMesh], MAX_ASSETS, context.platformMemory);
    arrayInit(context.platform.assets[(i32)AssetType::Texture], MAX_ASSETS, context.platformMemory);
    arrayInit(context.platform.assets[(i32)AssetType::CubemapTexture], MAX_ASSETS, context.platformMemory);
}

void contextHotReload(Context& context)
{
    arenaClear(context.gameMemory);
    arenaClear(context.tempMemory);

    initGameArrays(context);
}

void contextDeinit(Context& context)
{
    arrayClear(context.entityManager.entities);
    arrayClear(context.render.drawCommands);
    arrayClear(context.render.meshes);
    arrayClear(context.render.allTextures);
    for (size_t i = 0; i < (i32)AssetType::Max; ++i)
        arrayClear(context.platform.assets[i]);

    arenaDeinit(context.gameMemory);
    arenaDeinit(context.tempMemory);
    arenaDeinit(context.platformMemory);
}
