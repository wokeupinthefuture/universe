#include "game.hpp"
#include "context.hpp"
#include "entity.hpp"
#include "input.hpp"

#include "geometry.cpp"
#include "renderer.hpp"
#include "renderer_dx11.cpp"
#include "entity.cpp"
#include "input.cpp"

Entity defaultEntity()
{
    Entity e{};
    e.name = "entity";
    e.active = true;
    e.scale = vec3(1.f, 1.f, 1.f);
    e.isWorldMatrixDirty = true;
    return e;
}

Entity* pushDrawable(HeapArray<Entity>& entities, HeapArray<DrawCommand>& drawCmds, MeshType mesh)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Drawable;
    entity.drawCommand = pushDrawCmd(drawCmds, mesh);
    return arrayPush(entities, entity);
}

static void gameStart(Context& ctx)
{
    g_entityManager = &ctx.entityManager;
    g_input = &ctx.input;

    renderInit(ctx.window, ctx.windowSize.x, ctx.windowSize.y);

    auto camera = defaultEntity();
    camera.type = EntityType::Camera;
    camera.fov = 45.f;
    camera.aspect = 16.f / 9.f;
    camera.nearZ = 0.001f;
    camera.farZ = 1000.f;

    ctx.gameState.grid = pushDrawable(ctx.entityManager.entities, ctx.render.drawCommands, MeshType::Grid);
    ctx.gameState.sphere = pushDrawable(ctx.entityManager.entities, ctx.render.drawCommands, MeshType::Sphere);
    setShaderVariableVec4(*ctx.gameState.sphere->drawCommand, "color", vec4(0.5f, 1.f, 0.5f, 1.f));

    calculateCameraProjection(camera, ctx.entityManager.entities);
    setLocalPosition(camera, vec3(0, 5, -30));

    g_entityManager->camera = camera;
}

void gameInit(Context& ctx)
{
    gameStart(ctx);
}

void gamePreHotReload(Context& ctx)
{
    renderDeinit();
}

void gamePostHotReload(Context& ctx)
{
    gameStart(ctx);
}

void cameraController(Entity& camera)
{
    using namespace Platform;

    float cameraSpeed = 0.1f;

    if (isKeyPressed(KeyboardKey::KEY_SHIFT))
    {
        cameraSpeed *= 10.f;
    }

    if (isKeyPressed(KeyboardKey::KEY_Q))
    {
        addLocalPosition(camera, vec3(0, -cameraSpeed, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_E))
    {
        addLocalPosition(camera, vec3(0, cameraSpeed, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_D))
    {
        addLocalPosition(camera, vec3(cameraSpeed, 0, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_A))
    {
        addLocalPosition(camera, vec3(-cameraSpeed, 0, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_W))
    {
        addLocalPosition(camera, vec3(0, 0, cameraSpeed));
    }
    if (isKeyPressed(KeyboardKey::KEY_S))
    {
        addLocalPosition(camera, vec3(0, 0, -cameraSpeed));
    }
}

void gameUpdateAndRender(Context& ctx)
{
    if (isKeyPressed(KeyboardKey::KEY_ESCAPE))
    {
        ctx.wantsToQuit = true;
        return;
    }

    cameraController(g_entityManager->camera);

    static vec4 clearColor{0, 0, 0, 1};
    renderClear(clearColor);
    for (const auto& command : ctx.render.drawCommands)
        renderDraw(command);
    renderPresent();
}

void gameExit(Context& ctx)
{
    renderDeinit();
}
