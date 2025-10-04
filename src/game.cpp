#include "game.hpp"
#include "common/math.hpp"
#include "context.hpp"
#include "entity.hpp"
#include "input.hpp"

#include "common/math.cpp"
#include "geometry.cpp"
#include "renderer.hpp"
#include "renderer_dx11.cpp"
#include "entity.cpp"
#include "input.cpp"

void setColor(Entity& entity, vec4 color)
{
    setShaderVariableVec4(*entity.drawCommand, "objectColor", color);

    if (hasType(entity, EntityType::Light))
    {
        entity.lightColor = color;
        for (auto& other : g_entityManager->entities)
        {
            if (hasType(other, EntityType::Drawable) && other.drawCommand->shader == ShaderType::Basic)
                setShaderVariableVec4(*other.drawCommand, "lightColor", color);
        }
    }
}

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

Entity* pushLight(HeapArray<Entity>& entities, HeapArray<DrawCommand>& drawCmds, LightType type)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Light | EntityType::Drawable;
    entity.lightType = type;
    entity.drawCommand = pushDrawCmd(drawCmds, MeshType::Cube, ShaderType::Unlit);

    for (auto& other : entities)
    {
        if (hasType(other, EntityType::Drawable) && other.drawCommand->shader == ShaderType::Basic)
            setShaderVariableInt(*other.drawCommand, "lightType", (i32)type);
    }

    setColor(entity, vec4(1, 1, 1, 1));
    setLocalScale(entity, vec3(0.1f));

    return arrayPush(entities, entity);
}

void setLightDirection(Entity& light, vec3 direction)
{
    ENSURE(hasType(light, EntityType::Light));

    light.lightDirection = direction;

    for (const auto& entity : g_entityManager->entities)
    {
        if (hasType(entity, EntityType::Drawable) && entity.drawCommand->shader == ShaderType::Basic)
        {
            setShaderVariableVec3(*entity.drawCommand, "lightDirection", light.lightDirection);
        }
    }
}

void gameInit(Context& ctx)
{
    logInfo("game init");
    g_input = &ctx.input;
    g_entityManager = &ctx.entityManager;

    renderInit(ctx.window, ctx.windowSize.x, ctx.windowSize.y);

    auto camera = defaultEntity();
    camera.type = EntityType::Camera;
    camera.fov = 45.f;
    camera.aspect = 16.f / 9.f;
    camera.nearZ = 0.001f;
    camera.farZ = 1000.f;

    ctx.gameState.grid = pushDrawable(ctx.entityManager.entities, ctx.render.drawCommands, MeshType::Grid);
    ctx.gameState.sphere = pushDrawable(ctx.entityManager.entities, ctx.render.drawCommands, MeshType::Sphere);
    ctx.gameState.cube = pushDrawable(ctx.entityManager.entities, ctx.render.drawCommands, MeshType::Cube);
    setLocalPosition(*ctx.gameState.cube, vec3(-1.f, 0.f, 0.f));
    setColor(*ctx.gameState.cube, vec4(1.f, 1.f, 0.2f, 1.f));
    setColor(*ctx.gameState.sphere, vec4(0.2f, 1.f, 0.5f, 1.f));

    calculateCameraProjection(camera, ctx.entityManager.entities);
    setLocalPosition(camera, vec3(0, 0, -5));

    g_entityManager->camera = camera;

    ctx.gameState.cameraController = {};
    ctx.gameState.cameraController.camera = &ctx.entityManager.camera;
    ctx.gameState.cameraController.speed = 1.f;
    ctx.gameState.cameraController.sensitivity = 5.f;

    ctx.gameState.light = pushLight(ctx.entityManager.entities, ctx.render.drawCommands, LightType::Point);
    setLocalPosition(*ctx.gameState.light, vec3(0.f, 1.f, 0.f));
    setLightDirection(*ctx.gameState.light, vec3(0.f, -1.f, 0.f));

    for (auto& entity : ctx.entityManager.entities)
        updateTransform(entity, ctx.entityManager.camera);
}

void gamePreHotReload(Context& ctx)
{
    gameExit(ctx);
}

void gamePostHotReload(Context& ctx)
{
    gameInit(ctx);
}

void cameraControllerMoveAndRotate(CameraController& controller)
{
    using namespace Platform;

    const auto dt = 0.016f;

    controller.speed = CameraController::DEFAULT_SPEED;
    if (isKeyPressed(KeyboardKey::KEY_SHIFT))
        controller.speed *= 10.f;

    if (isKeyPressed(KeyboardKey::KEY_Q))
    {
        addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * -controller.speed);
    }
    if (isKeyPressed(KeyboardKey::KEY_E))
    {
        addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * controller.speed);
    }
    if (isKeyPressed(KeyboardKey::KEY_D))
    {
        addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * controller.speed);
    }
    if (isKeyPressed(KeyboardKey::KEY_A))
    {
        addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * -controller.speed);
    }
    if (isKeyPressed(KeyboardKey::KEY_W))
    {
        addLocalPosition(*controller.camera, getForwardVector(*controller.camera) * dt * controller.speed);
    }
    if (isKeyPressed(KeyboardKey::KEY_S))
    {
        addLocalPosition(*controller.camera, getForwardVector(*controller.camera) * dt * -controller.speed);
    }

    controller.pitchYawTarget += controller.pitchYawDelta * controller.sensitivity * dt;
    controller.pitchYaw =
        lerp(controller.pitchYaw, controller.pitchYawTarget, min(CameraController::PITCH_YAW_SMOOTHING * dt, 1.f));
    setLocalRotation(*controller.camera, controller.pitchYaw);
    controller.pitchYawDelta = {};
}

void cameraControllerUpdate(InputState& input, CameraController& controller)
{
    const auto mouseDelta = vec3{-input.mouse.delta.y, -input.mouse.delta.x, 0};
    if (wasMousePressed(true))
    {
        if (!controller.isPressed)
        {
            controller.isPressed = true;
            controller.pitchYawDelta += mouseDelta;
            controller.pressedPos = input.mouse.pos;
        }
    }

    if (controller.isPressed)
    {
        controller.pitchYawDelta += mouseDelta;
        cameraControllerMoveAndRotate(controller);

        if (wasMouseReleased(true))
        {
            controller.isPressed = false;
            controller.pitchYawDelta += mouseDelta;
        }
    }
}

void gameUpdateAndRender(Context& ctx)
{
    float time = getElapsedTime();
    for (auto& entity : ctx.entityManager.entities)
    {
        if (hasType(entity, EntityType::Drawable))
            setShaderVariableFloat(*entity.drawCommand, "time", time);
    }

    if (isKeyPressed(KeyboardKey::KEY_ESCAPE))
    {
        ctx.wantsToQuit = true;
        return;
    }

    if (wasKeyPressed(KeyboardKey::KEY_R))
    {
        ctx.wantsToReload = true;
        return;
    }

    cameraControllerUpdate(ctx.input, ctx.gameState.cameraController);

    const auto speed = 1;
    addLocalRotation(*ctx.gameState.cube, vec3(speed));

    if (isKeyPressed(KeyboardKey::KEY_1))
    {
        addLocalPosition(*ctx.gameState.light, vec3(0.f, -0.1f, 0.f));
    }
    if (isKeyPressed(KeyboardKey::KEY_2))
    {
        addLocalPosition(*ctx.gameState.light, vec3(0.f, 0.1f, 0.f));
    }

    if (wasKeyPressed(KeyboardKey::KEY_R))
    {
        ctx.wantsToReload = true;
    }

    static vec4 clearColor{0, 0, 0, 1};
    renderClear(clearColor);
    for (const auto& command : ctx.render.drawCommands)
        renderDraw(command);
    renderPresent();

    for (auto& kb : ctx.input.keyboard)
    {
        if (kb == ButtonState::Pressed)
            kb = ButtonState::Holding;
        if (kb == ButtonState::Released)
            kb = ButtonState::NotPressed;
    }

    if (ctx.input.mouse.leftState == ButtonState::Pressed)
        ctx.input.mouse.leftState = ButtonState::Holding;
    if (ctx.input.mouse.rightState == ButtonState::Pressed)
        ctx.input.mouse.rightState = ButtonState::Holding;
    ctx.input.mouse.delta = {};
}

void gameExit(Context& ctx)
{
    logInfo("game exit");
    renderDeinit();
}
