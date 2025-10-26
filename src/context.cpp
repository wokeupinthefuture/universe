#include "context.hpp"
#include "common/heap_array.hpp"

void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    context.timeScale = 1.f;
    context.dt = 0.016f;
    context.render.needsToResize = true;

    arenaInit(context.platformMemory, memorySize);
    arenaInit(context.gameMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);

    static constexpr auto MAX_ASSETS = 25;

    arrayInit(context.render.drawCommands, 2000, context.gameMemory, "draw commands");
    arrayInit(context.entityManager.entities, 3000, context.gameMemory, "entities");
    arrayInit(context.render.loadedMeshes, MAX_ASSETS, context.gameMemory, "loaded meshes");
    arrayInit(context.render.loadedTextures, MAX_ASSETS, context.gameMemory, "loaded textures");
    for (size_t i = 0; i < (i32)AssetType::Max; ++i)
        arrayInit(context.platform.assets[i], MAX_ASSETS, context.platformMemory, ASSETS_PATH[i]);
}

void contextHotReload(Context& context)
{
    arenaClear(context.gameMemory);
    arenaClear(context.tempMemory);

    arrayInit(context.render.drawCommands, context.render.drawCommands.capacity, context.gameMemory, "draw commands");
    arrayInit(context.entityManager.entities, context.entityManager.entities.capacity, context.gameMemory, "entities");
    arrayInit(context.render.loadedMeshes, context.render.loadedMeshes.capacity, context.gameMemory, "loaded meshes");
    arrayInit(context.render.loadedTextures, context.render.loadedTextures.capacity, context.gameMemory, "loaded textures");
}

void contextDeinit(Context& context)
{
    arrayClear(context.entityManager.entities);
    arrayClear(context.render.drawCommands);
    arrayClear(context.render.loadedMeshes);
    arrayClear(context.render.loadedTextures);
    for (size_t i = 0; i < (i32)AssetType::Max; ++i)
        arrayClear(context.platform.assets[i]);

    arenaDeinit(context.gameMemory);
    arenaDeinit(context.tempMemory);
    arenaDeinit(context.platformMemory);
}
