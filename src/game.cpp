#include "game.hpp"

#include "context.hpp"
#include "common/string.cpp"
#include "common/math.cpp"
#include "entity.hpp"
#include "geometry.cpp"
#include "geometry.hpp"
#include "renderer.hpp"
#include "shaders.cpp"
#include "renderer_dx11.cpp"
#include "physics.cpp"
#include "gui.cpp"
#include "entity.cpp"
#include "input.cpp"

#include "game/world.hpp"

#include "imgui.h"
#include <random>

struct CameraController
{
    static constexpr auto PITCH_YAW_SMOOTHING = 30.f;
    static constexpr auto DEFAULT_SPEED = 5.f;
    static constexpr auto USE_PRESSED_MODE = false;

    Entity* camera;

    bool isPressed;
    vec2 pressedPos;

    bool isMoving;

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

    World world;
};

void simulateCreatures(World& world, float dt);

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

Entity* createEntity()
{
    auto entity = defaultEntity();
    return arrayPush(g_context->entityManager.entities, entity);
}

Entity* createDrawable(RenderState& renderState, GeneratedMesh mesh)
{
    auto entity = defaultEntity();
    entity.traits = EntityTrait::Drawable;

    entity.drawCommand = pushDrawCmd(renderState, renderState.generatedMeshes[(i32)mesh]);
    entity.name = entity.drawCommand->mesh->name;

    return arrayPush(g_context->entityManager.entities, entity);
}

Entity* createDrawable(RenderState& renderState, const char* meshName)
{
    auto entity = defaultEntity();
    entity.traits = EntityTrait::Drawable;

    entity.drawCommand = pushDrawCmd(renderState, renderGetMesh(renderState, strSz(meshName)));
    entity.name = entity.drawCommand->mesh->name;

    return arrayPush(g_context->entityManager.entities, entity);
}

Entity* createLight(RenderState& renderState, LightType type)
{
    auto entity = createDrawable(renderState, GeneratedMesh::Cube);
    entity->traits = EntityTrait::Drawable | EntityTrait::Light;
    entity->name = strL("light");

    setLocalScale(*entity, vec3(0.5f));

    setLightType(*entity, type);
    setColor(*entity, vec4(1.f));

    if (type == LightType::Directional)
    {
        setLightDirection(*entity,
            {Shaders::DEFAULT_VARIABLES.lightDirection[0],
                Shaders::DEFAULT_VARIABLES.lightDirection[1],
                Shaders::DEFAULT_VARIABLES.lightDirection[2]});
    }

    return entity;
}

Entity* createGrid(RenderState& render)
{
    const auto grid = createDrawable(render, GeneratedMesh::Grid);

    auto& cmd = *grid->drawCommand;
    cmd.drawFlags |= DrawFlag::Wireframe;
    cmd.culling = Culling::None;

    setColor(*grid, vec4(0.5, 0.5, 0.5, 1));

    return grid;
}

Entity* createSkybox(RenderState& render, String cubemapTextureName)
{
    const auto box = createDrawable(render, GeneratedMesh::Cube);

    auto& cmd = *box->drawCommand;
    cmd.shaderType = ShaderType::Skybox;
    cmd.shaderTraits &= ShaderTrait::Textured;
    cmd.culling = Culling::Front;
    cmd.drawFlags &= ~(DrawFlag::DepthWrite);

    box->name = strL("skybox");
    box->traits |= EntityTrait::Skybox;
    setTexture(*box, 0, renderGetCubemap(render, cubemapTextureName));
    setColor(*box, vec4(1.f));

    return box;
}

void generateNormalArrows(RenderState& render, Entity& entity)
{
    auto& mesh = *entity.drawCommand->mesh;
    for (size_t i = 0; i < mesh.vertices.size; ++i)
    {
        auto vertex = mesh.vertices[i];
        const auto arrow = createDrawable(render, "arrow");
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

void focusCameraOnEntity(Entity& camera, Entity& target)
{
    vec3 targetPos = target.worldPosition;
    float distance = glm::length(target.worldScale) * 3.f + 2.f;

    vec3 camForward = getForwardVector(camera);
    vec3 newCamPos = targetPos - camForward * distance;
    setLocalPosition(camera, newCamPos);

    vec3 direction = glm::normalize(newCamPos - targetPos);
    vec3 euler = directionToEuler(direction);
    if (glm::any(glm::isnan(euler)))
        return;
    setLocalRotation(camera, euler);
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
        [](Entity& entity) { return entityHasTrait(entity, EntityTrait::Light); });

    vec4 lightColor{};
    if (lightSource)
        lightColor = getShaderVariableVec4(*lightSource->drawCommand, "lightColor");

    for (auto& entity : ctx.entityManager.entities)
    {
        if (entityHasTrait(entity, EntityTrait::Drawable))
        {
            if (getShaderVariableInt(*entity.drawCommand, "shaderTraits") & (i32)ShaderTrait::Shaded)
            {
                if (lightSource)
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
        ctx.gameState = arenaAlloc<GameState>(ctx.platformMemory);

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

        createSkybox(ctx.render, strL("skybox"));

        finalizeEntities(ctx);

        ctx.isGameLoaded = true;
    });

    if (!ctx.isGameLoaded)
    {
        auto camera = defaultEntity();
        camera.name = strClone("camera", ctx.platformMemory);
        camera.traits = EntityTrait::Camera;
        camera.cameraProjection = CameraProjection::Orthographic;
        camera.defaultFov = 75.f;
        camera.nearZ = 0.001f;
        camera.farZ = 1000.f;
        camera.orthoSize = 10.f;

        ctx.entityManager.camera = camera;
    }

    onResize(ctx);

    // createGrid(ctx.render);

    // light
    auto light = createLight(ctx.render, LightType::Point);
    setLocalPosition(*light, vec3(10, 20, 10));

    std::mt19937 rng(World::SEED);
    std::uniform_real_distribution<float> foodPosX(-10.f, 10.f);
    std::uniform_real_distribution<float> foodPosY(-10.f, 10.f);
    std::uniform_real_distribution<float> creatureColor(0.f, 1.f);

    for (int i = 0; i < World::CREATURES_COUNT; ++i)
    {
        const auto entity = createDrawable(ctx.render, GeneratedMesh::Quad);

        auto& cmd = *entity->drawCommand;
        cmd.shaderTraits |= ShaderTrait::Circle;

        setLocalPosition(*entity, {i + i * 0.2, 0, 0});
        setColor(*entity, {creatureColor(rng), creatureColor(rng), creatureColor(rng), 1});
        setLocalScale(*entity, {0.3f, 0.3f, 0.3f});

        entity->name = strL("creature");
        entity->creatureSpeed = 0.1f;
        entity->traits |= EntityTrait::Creature | EntityTrait::AABB;

        gameState->world.creatures[i] = entity;
    }

    for (int i = 0; i < World::FOOD_COUNT; ++i)
    {
        const auto entity = createDrawable(ctx.render, GeneratedMesh::Triangle);

        setLocalPosition(*entity, {foodPosX(rng), foodPosY(rng), 0});

        setColor(*entity, {1, 0, 0, 1});
        setLocalScale(*entity, {0.15f, 0.15f, 0.16f});

        char name[256]{};
        sprintf(name, "food_%i", i);
        entity->name = strClone(name, ctx.gameMemory);
        entity->traits |= EntityTrait::Food | EntityTrait::AABB;

        gameState->world.food[i] = entity;
    }

    if (!ctx.isGameLoaded)
    {
        setLocalPosition(ctx.entityManager.camera, vec3(0, 0, -15));
    }
}

void gamePreHotReload(Context& ctx)
{
    gameExit(ctx);
}

void gamePostHotReload(Context& ctx)
{
    gameInit(ctx);
}

void cameraControllerMoveAndRotate(float dt, CameraController& controller, vec2 screenSize, Array<Entity> entities)
{
    controller.isMoving = false;

    controller.speed = CameraController::DEFAULT_SPEED;
    if (isKeyPressed(KeyboardKey::KEY_SHIFT))
        controller.speed *= 10.f;

    if (controller.camera->cameraProjection == CameraProjection::Perspective)
    {
        if (isKeyPressed(KeyboardKey::KEY_Q))
        {
            addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * -controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_E))
        {
            addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_D))
        {
            addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_A))
        {
            addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * -controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_W))
        {
            addLocalPosition(*controller.camera, getForwardVector(*controller.camera) * dt * controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_S))
        {
            addLocalPosition(*controller.camera, getForwardVector(*controller.camera) * dt * -controller.speed);
            controller.isMoving = true;
        }
    }
    else
    {
        if (isKeyPressed(KeyboardKey::KEY_Q))
        {
            addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * -controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_E))
        {
            addLocalPosition(*controller.camera, getUpVector(*controller.camera) * dt * controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_D))
        {
            addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_A))
        {
            addLocalPosition(*controller.camera, getRightVector(*controller.camera) * dt * -controller.speed);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_W))
        {
            controller.camera->orthoSize -= dt * controller.speed;
            controller.camera->orthoSize = std::max(controller.camera->orthoSize, 0.001f);
            calculateCameraProjection(*controller.camera, screenSize, entities);
            controller.isMoving = true;
        }
        if (isKeyPressed(KeyboardKey::KEY_S))
        {
            controller.camera->orthoSize += dt * controller.speed;
            calculateCameraProjection(*controller.camera, screenSize, entities);
            controller.isMoving = true;
        }

        return;
    }

    controller.pitchYawTarget += controller.pitchYawDelta * controller.sensitivity * dt;
    controller.pitchYaw =
        lerp(controller.pitchYaw, controller.pitchYawTarget, min(CameraController::PITCH_YAW_SMOOTHING * dt, 1.f));
    setLocalRotation(*controller.camera, controller.pitchYaw);
    controller.pitchYawDelta = {};
}

void cameraControllerUpdate(float dt, InputState& input, CameraController& controller, vec2 screenSize, Array<Entity> entities)
{
    if (guiIsCapturingKeyboard() || guiIsCapturingMouse())
        return;

    const auto mouseDelta = vec3{-input.mouse.delta.y, -input.mouse.delta.x, 0};

    if constexpr (CameraController::USE_PRESSED_MODE)
    {
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
            cameraControllerMoveAndRotate(dt, controller, screenSize, entities);

            if (wasMouseReleased(true))
            {
                controller.isPressed = false;
                controller.pitchYawDelta += mouseDelta;
            }
        }
    }
    else
    {
        cameraControllerMoveAndRotate(dt, controller, screenSize, entities);
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

        if (!entityHasTrait(entity, EntityTrait::Camera))
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

        if (!entityHasTrait(entity, EntityTrait::Camera))
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
    if (entityHasTrait(entity, EntityTrait::Camera))
    {
        static const char* projectionTypes[] = {"perspective", "orthographic"};
        if (ImGui::BeginCombo("projection", projectionTypes[(i32)entity.cameraProjection]))
        {
            for (int n = 0; n < ARR_LENGTH(projectionTypes); n++)
            {
                const auto isSelected = n == (i32)entity.cameraProjection;
                if (ImGui::Selectable(projectionTypes[n], isSelected))
                {
                    entity.cameraProjection = (CameraProjection)n;
                    calculateCameraProjection(entity, ctx.render.screenSize, ctx.entityManager.entities);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (entity.cameraProjection == CameraProjection::Perspective)
        {
            ImGui::SetNextItemWidth(GUI_SLIDER_WIDTH);
            auto fov = entity.defaultFov;
            if (ImGui::DragFloat("fov", &fov))
            {
                entity.defaultFov = fov;
                calculateCameraProjection(entity, ctx.render.screenSize, ctx.entityManager.entities);
            }
        }
        else
        {
            ImGui::SetNextItemWidth(GUI_SLIDER_WIDTH);
            if (ImGui::DragFloat("ortho size", &entity.orthoSize, 0.1f, 0.1f, 1000.f))
            {
                calculateCameraProjection(entity, ctx.render.screenSize, ctx.entityManager.entities);
            }
        }
    }

    // light
    if (entityHasTrait(entity, EntityTrait::Light))
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

    if (entityHasTrait(entity, EntityTrait::Drawable))
    {
        if (ImGui::CheckboxFlags("shaded", (int*)&entity.drawCommand->shaderTraits, (int)ShaderTrait::Shaded))
        {
        }  // value updated by CheckboxFlags
        ImGui::SameLine();
        if (ImGui::CheckboxFlags("textured", (int*)&entity.drawCommand->shaderTraits, (int)ShaderTrait::Textured))
        {
        }
        ImGui::SameLine();
        if (ImGui::CheckboxFlags("circle", (int*)&entity.drawCommand->shaderTraits, (int)ShaderTrait::Circle))
        {
        }

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
    ImGui::Begin("hierarchy");

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

    ImGui::End();

    if (ctx.gui.selectedEntity)
    {
        ImGui::Begin("inspector");
        guiEntityContents(ctx, *(Entity*)ctx.gui.selectedEntity);
        ImGui::End();
    }
}

void simulateCreatures(World& world, float dt)
{
    for (auto& creature : world.creatures)
    {
        Entity* closestFood = nullptr;
        for (auto& thisFood : world.food)
        {
            const auto isClosestFoodValid = closestFood && isActive(*closestFood);
            const auto isThisFoodValid = thisFood && isActive(*thisFood);

            if (!isThisFoodValid)
                continue;

            if (isClosestFoodValid)
            {
                const auto isThisFoodCloser = distance(thisFood->worldPosition, creature->worldPosition) <
                                              distance(closestFood->worldPosition, creature->worldPosition);
                if (isThisFoodCloser)
                {
                    closestFood = thisFood;
                }
            }
            else
            {
                closestFood = thisFood;
            }
        }

        if (closestFood)
        {
            const auto dir = direction(creature->worldPosition, closestFood->worldPosition);

            if (creature->creatureSpeed > 30.f)
            {
                for (auto speed = creature->creatureSpeed; speed > 0.f; speed *= 0.5f)
                {
                    addWorldPosition(*creature, dir * speed * dt);
                    if (isColliding(*creature, *closestFood))
                    {
                        setActive(*closestFood, false);
                        creature->creatureSpeed += 0.05f;
                        creature->creatureSpeed = std::min(15.f, creature->creatureSpeed);
                        break;
                    }
                }
            }
            else
            {
                addWorldPosition(*creature, dir * creature->creatureSpeed * dt);
                if (isColliding(*creature, *closestFood))
                {
                    setActive(*closestFood, false);
                    creature->creatureSpeed += 0.05f;
                    creature->creatureSpeed = std::min(15.f, creature->creatureSpeed);
                }
            }
        }
    }
}

void gameUpdateAndRender(Context& ctx)
{
    const auto gameState = (GameState*)ctx.gameState;

    const auto timeScale = gameState->timeScale;
    const auto dt = GameState::DELTA_TIME * timeScale * (float)!gameState->pause;
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
        if (entityHasTrait(entity, EntityTrait::Drawable))
            setShaderVariableFloat(*entity.drawCommand, "time", time);
    }

    if (wasKeyPressed(KeyboardKey::KEY_R))
        ctx.wantsToReload = true;

    if (wasKeyPressed(KeyboardKey::KEY_F) && ctx.gui.selectedEntity)
    {
        auto& camera = *gameState->cameraController.camera;
        focusCameraOnEntity(camera, *(Entity*)ctx.gui.selectedEntity);
        gameState->cameraController.pitchYaw = camera.euler;
        gameState->cameraController.pitchYawTarget = camera.euler;
    }

    simulateCreatures(gameState->world, dt);

    cameraControllerUpdate(unscaledDt, ctx.input, gameState->cameraController, ctx.render.screenSize, ctx.entityManager.entities);

    if (ctx.render.needsToResize)
        onResize(ctx);

    guiBegin();
    onGui(ctx);

    static vec4 clearColor{1, 1, 1, 1};
    renderClearAndResize(ctx.render, clearColor);
    for (auto& command : ctx.render.drawCommands)
        renderDraw(command);
    guiDraw(gameState->cameraController.isMoving ? 0.1f : 1.f, unscaledDt);
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
