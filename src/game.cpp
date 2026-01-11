#include "game.hpp"

#include "context.hpp"
#include "common/string.cpp"
#include "common/math.cpp"
#include "entity.hpp"
#include "geometry.cpp"
#include "renderer.hpp"
#include "shaders.cpp"
#include "renderer_dx11.cpp"
#include "gui.cpp"
#include "entity.cpp"
#include "input.cpp"
#include "celestial.hpp"

#include "imgui.h"

struct CameraController
{
    static constexpr auto PITCH_YAW_SMOOTHING = 30.f;
    static constexpr auto DEFAULT_SPEED = 5.f;
    Entity* camera;
    bool isPressed;
    vec2 pressedPos;
    float speed;
    float sensitivity;
    vec3 pitchYaw;
    vec3 pitchYawTarget;
    vec3 pitchYawDelta;
};

struct GameState
{
    static constexpr auto DELTA_TIME = 0.016f;

    bool pause;
    float timeScale;

    Entity* grid;
    CameraController cameraController;
    Celestial celestials[(size_t)CelestialType::Max];
};

Entity defaultEntity()
{
    Entity e{};
    e.name = strL("entity");
    e.flags = EntityFlag::Active;
    e.scale = vec3(1.f, 1.f, 1.f);
    e.isWorldMatrixDirty = true;
    e.guiIsLocal = true;
    return e;
}

Entity* pushEntity()
{
    auto entity = defaultEntity();
    return arrayPush(g_context->entityManager.entities, entity);
}

Entity* pushDrawable(RenderState& renderState, GeneratedMesh meshType, ShaderType shader = ShaderType::Unlit)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Drawable;

    entity.drawCommand = pushDrawCmd(renderState, renderState.generatedMeshes[(i32)meshType], shader);
    entity.name = entity.drawCommand->mesh->name;

    return arrayPush(g_context->entityManager.entities, entity);
}

Entity* pushDrawable(RenderState& renderState, const char* meshName, ShaderType shader = ShaderType::Unlit)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Drawable;

    entity.drawCommand = pushDrawCmd(renderState, renderGetMesh(renderState, strSz(meshName)), shader);
    entity.name = entity.drawCommand->mesh->name;

    return arrayPush(g_context->entityManager.entities, entity);
}

void setLightSource(Entity& entity, LightType type)
{
    entity.type |= EntityType::Light;

    setLightType(entity, type);
    setColor(entity, vec4(1.f));

    if (type == LightType::Directional)
    {
        setLightDirection(entity,
            {Shaders::DEFAULT_VARIABLES.lightDirection[0],
                Shaders::DEFAULT_VARIABLES.lightDirection[1],
                Shaders::DEFAULT_VARIABLES.lightDirection[2]});
    }
}

Entity* pushSkybox(RenderState& render, String cubemapTextureName)
{
    const auto box = pushDrawable(render, GeneratedMesh::Cube, ShaderType::Skybox);
    box->type |= EntityType::Skybox;
    setTexture(*box, 0, renderGetCubemap(render, cubemapTextureName));
    setColor(*box, vec4(1.f));
    return box;
}

Celestial pushCelestial(RenderState& render, CelestialType type)
{
    const auto& data = CELESTIAL_DATA[(size_t)type];
    const auto entity =
        // pushDrawable(render, GeneratedMesh::Sphere, type == CelestialType::Sun ? ShaderType::Unlit : ShaderType::Basic);
        pushDrawable(render, GeneratedMesh::Sphere, ShaderType::Unlit);
    setTexture(*entity, 0, renderGetTexture(render, data.name));

    entity->name = data.name;

    setLocalScale(*entity, data.scale);

    if (type == CelestialType::Sun)
        setLightSource(*entity, LightType::Point);

    Celestial celestial;

    celestial.e = entity;
    celestial.initialVelocity = vec3(0.f, 0.f, -0.5f);
    celestial.currentVelocity = celestial.initialVelocity;
    celestial.mass = data.scale.x;

    return celestial;
}

void generateNormalArrows(RenderState& render, Entity& entity)
{
    auto& mesh = *entity.drawCommand->mesh;
    for (size_t i = 0; i < mesh.vertices.size; ++i)
    {
        auto vertex = mesh.vertices[i];
        const auto arrow = pushDrawable(render, "arrow");
        setParent(*arrow, &entity);
        const auto worldArrowPos = entity.worldMatrixCache * vec4(vertex.pos, 1.f);
        setWorldPosition(*arrow, worldArrowPos);
        setLocalScale(*arrow, 0.05f);
        setLocalRotation(*arrow, directionToEuler(vertex.normal));
        setColor(*arrow, vec4(0.7, 0.7, 0.7, 1));
    }
}

void onResize(Context& ctx)
{
    calculateCameraProjection(ctx.entityManager.camera, ctx.render.screenSize, ctx.entityManager.entities);
}

void cameraControllerInit(CameraController& controller, Entity& camera)
{
    controller = {};
    controller.camera = &camera;
    controller.speed = 1.f;
    controller.sensitivity = 5.f;
    controller.pitchYawTarget = controller.camera->euler;
    controller.pitchYaw = controller.camera->euler;
}

void finalizeEntities(Context& ctx)
{
    const auto lightSource = find(ctx.entityManager.entities.data,
        ctx.entityManager.entities.size,
        [](Entity& entity) { return hasType(entity, EntityType::Light); });
    const auto lightColor = getShaderVariableVec4(*lightSource->drawCommand, "lightColor");

    for (auto& entity : ctx.entityManager.entities)
    {
        if (hasType(entity, EntityType::Drawable))
        {
            if (entity.drawCommand->shader == ShaderType::Basic)
            {
                setShaderVariableVec4(*entity.drawCommand, "lightColor", lightColor);
                setShaderVariableVec3(*entity.drawCommand, "lightDirection", lightSource->lightDirection);
                setShaderVariableInt(*entity.drawCommand, "lightType", (i32)lightSource->lightType);
            }

            updateTransform(entity);
        }
    }
}

void gameInit(Context& ctx)
{
    logInfo("game init");

    if (!ctx.isGameLoaded)
    {
        ctx.gameState = arenaAlloc<GameState>(ctx.platformMemory);
    }
    const auto gameState = (GameState*)ctx.gameState;

    if (!ctx.isGameLoaded)
        gameState->timeScale = 1.f;

    g_context = &ctx;

    renderInitResources(ctx.render, ctx.platform.assets);
    renderInit(ctx.render, ctx.platform.window);
    guiInit(&ctx.platform.guiWindowEventCallback, ctx.platform.window, ctx.platform.dpi);

    defer({
        if (!ctx.isGameLoaded)
            cameraControllerInit(gameState->cameraController, ctx.entityManager.camera);

        pushSkybox(ctx.render, strL("skybox"));

        finalizeEntities(ctx);

        ctx.isGameLoaded = true;
    });

    if (!ctx.isGameLoaded)
    {
        auto camera = defaultEntity();
        camera.name = strClone("camera", ctx.platformMemory);
        camera.type = EntityType::Camera;
        camera.defaultFov = 75.f;
        camera.nearZ = 0.001f;
        camera.farZ = 1000.f;

        ctx.entityManager.camera = camera;
    }

    onResize(ctx);

    gameState->grid = pushDrawable(ctx.render, GeneratedMesh::Grid, ShaderType::Unlit);
    setColor(*gameState->grid, vec4(0.5, 0.5, 0.5, 1));

    if (!ctx.isGameLoaded)
    {
        setLocalPosition(ctx.entityManager.camera, vec3(-45, 34, 23));
        setLocalRotation(ctx.entityManager.camera, vec3(30, 115, 0));
    }

    vec3 offset = {0, 0, 0};
    for (size_t type = 0; type < (size_t)CelestialType::Max; ++type)
    {
        vec3 offsetByType{};

        const auto celestial = pushCelestial(ctx.render, (CelestialType)type);

        if ((CelestialType)type == CelestialType::Moon)
        {
            offsetByType.x = 2.f;
            offsetByType.y = 1.f;
        }
        else
        {
            offset += vec3(10, 0, 0);
        }

        setLocalPosition(*celestial.e, offset + offsetByType);
        gameState->celestials[type] = celestial;

        if ((CelestialType)type == CelestialType::Sun)
        {
            offset.x += 5.f;
        }
    }

    for (auto& c : gameState->celestials)
        setActive(*c.e, false);

    auto& earth = gameState->celestials[(size_t)CelestialType::Earth];
    auto& moon = gameState->celestials[(size_t)CelestialType::Moon];
    setActive(*moon.e, true);
    setActive(*earth.e, true);

    earth.initialVelocity = {};
    earth.currentVelocity = {};
}

void gamePreHotReload(Context& ctx)
{
    gameExit(ctx);
}

void gamePostHotReload(Context& ctx)
{
    gameInit(ctx);
}

void cameraControllerMoveAndRotate(float dt, CameraController& controller)
{
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

void cameraControllerUpdate(float dt, InputState& input, CameraController& controller)
{
    if (guiIsCapturingKeyboard() || guiIsCapturingMouse())
        return;

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
        cameraControllerMoveAndRotate(dt, controller);

        if (wasMouseReleased(true))
        {
            controller.isPressed = false;
            controller.pitchYawDelta += mouseDelta;
        }
    }
}

static constexpr auto GUI_SLIDER_WIDTH = 50.f;
void guiEntityContents(Context& ctx, Entity& entity)
{
    ImGui::PushID(&entity);
    ImGui::Text("%s", entity.name.data);

    // transform
    if (entity.guiIsLocal)
    {
        auto pos = entity.position;
        if (ImGui::DragFloat3("pos", &pos.x, 0.1f))
        {
            setLocalPosition(entity, pos);
        }

        auto rot = entity.euler;
        if (ImGui::DragFloat3("rot", &rot.x, 0.1f))
        {
            setLocalRotation(entity, rot);
        }

        if (!hasType(entity, EntityType::Camera))
        {
            auto scale = entity.scale;
            if (ImGui::DragFloat3("scale", &scale.x, 0.1f))
            {
                setLocalScale(entity, scale);
            }
        }
    }
    else
    {
        auto pos = entity.worldPosition;
        if (ImGui::DragFloat3("pos", &pos.x, 0.1f))
        {
            setWorldPosition(entity, pos);
        }

        auto rot = entity.worldEuler;
        if (ImGui::DragFloat3("rot", &rot.x, 0.1f))
        {
            setWorldRotation(entity, rot);
        }

        if (!hasType(entity, EntityType::Camera))
        {
            auto scale = entity.worldScale;
            if (ImGui::DragFloat3("scale", &scale.x, 0.1f))
            {
                setWorldScale(entity, scale);
            }
        }
    }

    if (ImGui::RadioButton("local", entity.guiIsLocal))
        entity.guiIsLocal = true;
    ImGui::SameLine();
    if (ImGui::RadioButton("world", !entity.guiIsLocal))
        entity.guiIsLocal = false;

    // camera
    if (hasType(entity, EntityType::Camera))
    {
        ImGui::SetNextItemWidth(GUI_SLIDER_WIDTH);
        auto fov = entity.defaultFov;
        if (ImGui::DragFloat("fov", &fov))
        {
            entity.defaultFov = fov;
            calculateCameraProjection(entity, ctx.render.screenSize, ctx.entityManager.entities);
        }
    }

    // light
    if (hasType(entity, EntityType::Light))
    {
        static const char* lightTypes[] = {"directional", "point"};
        if (ImGui::BeginCombo("type", lightTypes[(i32)entity.lightType]))
        {
            for (int n = 0; n < ARR_LENGTH(lightTypes); n++)
            {
                const auto isSelected = n == (i32)entity.lightType;
                if (ImGui::Selectable(lightTypes[n], isSelected))
                    setLightType(entity, (LightType)n);
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (entity.lightType == LightType::Directional)
        {
            auto direction = entity.lightDirection;
            if (ImGui::DragFloat3("direction", &direction.x, 0.1f))
            {
                setLightDirection(entity, direction);
            }
        }
    }

    if (hasType(entity, EntityType::Drawable))
    {
        ImGui::Text("shader: %s", (entity.drawCommand->shader == ShaderType::Basic) ? "basic" : "unlit");
        auto color = getShaderVariableVec4(*entity.drawCommand, "objectColor");
        if (ImGui::ColorEdit4("color", &color.x))
        {
            setColor(entity, color);
        }
    }

    bool active = isActive(entity);
    if (ImGui::Checkbox("active", &active))
    {
        setActive(entity, active);
    }

    ImGui::PopID();
}

void guiCheckEntityReparentSource(Entity& toReparent)
{
    if (ImGui::BeginDragDropSource())
    {
        const auto entityPtr = &toReparent;
        ImGui::SetDragDropPayload("reparent", &entityPtr, sizeof(void*));
        ImGui::EndDragDropSource();
    }
}

void guiCheckEntityReparentTarget(Entity& newParent)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const auto entityPayload = ImGui::AcceptDragDropPayload("reparent"); entityPayload && entityPayload->Data)
        {
            const auto toReparent = (Entity**)entityPayload->Data;
            if (isKeyPressed(KeyboardKey::KEY_ALT))
            {
                setParent(**toReparent, &newParent, false);
            }
            else
            {
                setParent(**toReparent, &newParent, true);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void guiEntityContextMenu(Entity& entity)
{
    char label[256]{};
    sprintf(label, "%llu", (u64)&entity);
    if (ImGui::BeginPopupContextItem(label))
    {
        if (ImGui::MenuItem("unparent"))
            setParent(entity, nullptr);

        ImGui::EndPopup();
    }
}

void guiEntityHierarchy(Context& ctx, Entity& entity, ImGuiTableFlags flags, u32 hierarchyLevel)
{
    hierarchyLevel++;

    if (hierarchyLevel == 1 && entity.parent != nullptr)
        return;

    const auto hasChildren = (bool)entity.children;

    ImGui::TableNextColumn();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (ctx.gui.selectedEntity == &entity)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    if (hasChildren)
    {
        ImGui::PushID(&entity);
        const auto open = ImGui::TreeNodeEx(entity.name.data, nodeFlags);
        ImGui::PopID();

        guiEntityContextMenu(entity);

        guiCheckEntityReparentSource(entity);
        guiCheckEntityReparentTarget(entity);

        if (ImGui::IsItemClicked())
        {
            ctx.gui.selectedEntity = &entity;
        }

        ImGui::TableNextColumn();

        if (open)
        {
            for (auto child : entity.children)
            {
                if (!child)
                    continue;
                guiEntityHierarchy(ctx, *child, nodeFlags, hierarchyLevel);
            }

            ImGui::TreePop();
        }
    }
    else
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
        nodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::PushID(&entity);
        ImGui::TreeNodeEx(entity.name.data, nodeFlags);
        ImGui::PopID();

        guiEntityContextMenu(entity);

        guiCheckEntityReparentSource(entity);
        guiCheckEntityReparentTarget(entity);

        if (ImGui::IsItemClicked())
        {
            ctx.gui.selectedEntity = &entity;
        }

        ImGui::TableNextColumn();
    }
}

void onGui(Context& ctx)
{
    const auto gameState = (GameState*)ctx.gameState;

    ImGui::SetNextWindowPos({});
    ImGui::Begin("universe");

    ImGui::Text("pause: %s", gameState->pause ? "true" : "false");
    ImGui::SetNextItemWidth(GUI_SLIDER_WIDTH);
    ImGui::DragFloat("time scale", &gameState->timeScale, 0.1f);

    const auto flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable |
                       ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("table", 2, flags))
    {

        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40);
        ImGui::TableHeadersRow();

        for (auto& entity : ctx.entityManager.entities)
        {
            if (entity.parent)
                continue;
            guiEntityHierarchy(ctx, entity, flags, 0);
        }

        if (!ctx.entityManager.camera.parent)
            guiEntityHierarchy(ctx, ctx.entityManager.camera, flags, 0);

        ImGui::EndTable();
    }

    if (ctx.gui.selectedEntity)
    {
        guiEntityContents(ctx, *(Entity*)ctx.gui.selectedEntity);
    }

    ImGui::Separator();

    for (auto& c : gameState->celestials)
    {
        ImGui::PushID(&c);
        ImGui::Text("%s", c.e->name.data);

        float& mass = c.mass;
        vec3& velocity = c.currentVelocity;
        ImGui::DragFloat("mass", &mass);
        ImGui::DragFloat3("velocity", &velocity.x);

        ImGui::PopID();
        ImGui::Separator();
    }

    ImGui::End();
}

void celestialsUpdate(float dt, GameState* gameState)
{
    static constexpr auto ROTATION_SPEED = 1.f;
    for (const auto& celestial : gameState->celestials)
    {
        if (!isActive(*celestial.e))
            continue;

        addLocalRotation(*celestial.e, vec3(0, ROTATION_SPEED * dt * 50.f, 0));
    }

    for (auto& celestial : gameState->celestials)
    {
        if (!isActive(*celestial.e) || (isZero(celestial.initialVelocity)))
            continue;

        for (const auto& other : gameState->celestials)
        {
            if (&other == &celestial || !isActive(*other.e))
                continue;

            const auto direction = glm::normalize(celestial.e->worldPosition - other.e->worldPosition);
            auto distance = glm::distance(celestial.e->worldPosition, other.e->worldPosition);
            distance = std::max(distance, 3.f);
            const auto force = G * other.mass / (distance * distance);
            const auto acceleration = -direction * force;
            celestial.currentVelocity += acceleration * dt;
        }
    }

    for (auto& celestial : gameState->celestials)
    {
        addWorldPosition(*celestial.e, celestial.currentVelocity * dt);
    }
}

void gameUpdateAndRender(Context& ctx)
{
    const auto gameState = (GameState*)ctx.gameState;

    const auto timeScale = gameState->timeScale;
    const auto dt = GameState::DELTA_TIME * timeScale;
    const auto unscaledDt = GameState::DELTA_TIME;

    if (wasKeyPressed(KeyboardKey::KEY_SPACE))
        gameState->pause = !gameState->pause;
    if (wasKeyPressed(KeyboardKey::KEY_Z))
        gameState->timeScale *= 0.5;
    if (wasKeyPressed(KeyboardKey::KEY_X))
        gameState->timeScale *= 2;

    float time = getElapsedTime();
    for (auto& entity : ctx.entityManager.entities)
    {
        if (hasType(entity, EntityType::Drawable))
            setShaderVariableFloat(*entity.drawCommand, "time", time);
    }

    if (wasKeyPressed(KeyboardKey::KEY_R))
        ctx.wantsToReload = true;

    cameraControllerUpdate(unscaledDt, ctx.input, gameState->cameraController);

    if (!gameState->pause)
        celestialsUpdate(dt, gameState);

    if (ctx.render.needsToResize)
    {
        onResize(ctx);
    }

    guiBegin();
    onGui(ctx);

    static vec4 clearColor{0, 0, 0, 1};
    renderClearAndResize(ctx.render, clearColor);
    for (const auto& command : ctx.render.drawCommands)
        renderDraw(command);
    guiDraw();
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
    guiDeinit();
    renderDeinit();
}
