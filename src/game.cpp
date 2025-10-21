#include "game.hpp"

#include "entity.hpp"
#include "imgui.h"

#include "common/math.cpp"
#include "geometry.cpp"
#include "input.hpp"
#include "renderer.hpp"
#include "shaders.cpp"
#include "renderer_dx11.cpp"
#include "gui.cpp"
#include "entity.cpp"
#include "input.cpp"

Entity defaultEntity()
{
    Entity e{};
    e.name = "entity";
    e.active = true;
    e.scale = vec3(1.f, 1.f, 1.f);
    e.isWorldMatrixDirty = true;
    e.guiIsLocal = true;
    return e;
}

Entity* pushEntity()
{
    auto entity = defaultEntity();
    return arrayPush(getEntities(), entity);
}

Entity* pushDrawable(RenderState& renderState, MeshType mesh, AssetID customMeshID = AssetID::Max)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Drawable;
    entity.drawCommand = pushDrawCmdEx(renderState, mesh, customMeshID);

    switch (mesh)
    {
        default: LOGIC_ERROR();
        case MeshType::Cube: entity.name = "cube"; break;
        case MeshType::Grid: entity.name = "grid"; break;
        case MeshType::Sphere: entity.name = "sphere"; break;
        case MeshType::Triangle: entity.name = "triangle"; break;
        case MeshType::Quad: entity.name = "quad"; break;
        case MeshType::Custom:
        {
            switch (customMeshID)
            {
                default: LOGIC_ERROR();
                case AssetID::ArrowMesh: entity.name = "arrow";
            }
        }
        break;
    }

    return arrayPush(getEntities(), entity);
}

Entity* pushLight(RenderState& renderState, LightType type)
{
    auto entity = defaultEntity();
    entity.type = EntityType::Light | EntityType::Drawable;
    entity.drawCommand = pushDrawCmd(renderState, MeshType::Sphere, ShaderType::Unlit);
    entity.name = "light";

    setLightType(entity, type);

    if (type == LightType::Directional)
    {
        setLightDirection(entity,
            {Shaders::DEFAULT_VARIABLES.lightDirection[0],
                Shaders::DEFAULT_VARIABLES.lightDirection[1],
                Shaders::DEFAULT_VARIABLES.lightDirection[2]});
    }

    setColor(entity, vec4(1, 1, 1, 1));
    setLocalScale(entity, vec3(0.1f));

    return arrayPush(getEntities(), entity);
}

void onResize(Context& ctx)
{
    calculateCameraProjection(ctx.entityManager.camera, ctx.render.screenSize, getEntities());
}

void gameInit(Context& ctx)
{
    logInfo("game init");
    setInternalPointer(ctx.input);
    setInternalPointer(ctx.entityManager, ctx.tempMemory);

    renderInitGeometry(ctx.render, ctx.platform.assets, ctx.gameMemory, ctx.tempMemory);
    renderInit(ctx.render, ctx.platform.window);
    guiInit(&ctx.platform.guiWindowEventCallback, ctx.platform.window, ctx.platform.dpi);

    auto camera = defaultEntity();
    camera.name = "camera";
    camera.type = EntityType::Camera;
    camera.defaultFov = 75.f;
    camera.nearZ = 0.001f;
    camera.farZ = 1000.f;

    setLocalPosition(camera, vec3(0, 0, -5));
    ctx.entityManager.camera = camera;

    onResize(ctx);

    ctx.gameState.grid = pushDrawable(ctx.render, MeshType::Grid);
    ctx.gameState.sphere = pushDrawable(ctx.render, MeshType::Sphere);
    setLocalPosition(*ctx.gameState.sphere, vec3(1.f, 0.f, 0.f));
    ctx.gameState.cube = pushDrawable(ctx.render, MeshType::Cube);
    setLocalPosition(*ctx.gameState.cube, vec3(-1.f, 0.f, 0.f));
    setColor(*ctx.gameState.cube, vec4(1.f, 1.f, 0.2f, 1.f));
    setColor(*ctx.gameState.sphere, vec4(0.2f, 1.f, 0.5f, 1.f));

    ctx.gameState.cameraController = {};
    ctx.gameState.cameraController.camera = &ctx.entityManager.camera;
    ctx.gameState.cameraController.speed = 1.f;
    ctx.gameState.cameraController.sensitivity = 5.f;

    ctx.gameState.lightOrigin = pushEntity();
    ctx.gameState.lightOrigin->name = "light origin";
    setLocalPosition(*ctx.gameState.lightOrigin, ctx.gameState.cube->position);
    ctx.gameState.light = pushLight(ctx.render, LightType::Point);
    setParent(*ctx.gameState.light, ctx.gameState.lightOrigin, true);

    setLocalPosition(ctx.entityManager.camera, vec3(0, 2.5, -7));

    ctx.gameState.arrow = pushDrawable(ctx.render, MeshType::Custom, AssetID::ArrowMesh);
    setColor(*ctx.gameState.arrow, vec4(0.5f, 0.5f, 0.5f, 1.f));
    setParent(*ctx.gameState.arrow, ctx.gameState.sphere);
    setLocalPosition(*ctx.gameState.arrow, vec3(0, 2, 0));

    for (auto& entity : ctx.entityManager.entities)
        if (hasType(entity, EntityType::Drawable))
            updateTransform(entity);
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
    if (guiIsCapturingMouse() || guiIsCapturingKeyboard())
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
    ImGui::Text("%s", entity.name);

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
            calculateCameraProjection(entity, ctx.render.screenSize, getEntities());
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

        auto color = entity.lightColor;
        if (ImGui::ColorEdit4("color", &color.x))
        {
            setColor(entity, color);
        }
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

    const auto hasChildren =
        find(entity.children, MAX_ENTITY_CHILDREN, [](Entity* child) { return child != nullptr; }) != nullptr;

    ImGui::TableNextColumn();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (ctx.gui.selectedEntity == &entity)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    if (hasChildren)
    {
        const auto open = ImGui::TreeNodeEx(entity.name, nodeFlags);

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
        ImGui::TreeNodeEx(entity.name, nodeFlags);

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
    ImGui::Begin("universe");

    ImGui::Text("pause: %s", ctx.pause ? "true" : "false");
    ImGui::SetNextItemWidth(GUI_SLIDER_WIDTH);
    ImGui::DragFloat("time scale", &ctx.timeScale, 0.1f);

    const auto flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable |
                       ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("table", 2, flags))
    {

        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 40);
        ImGui::TableHeadersRow();

        for (auto& entity : getEntities())
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
        ImGui::Text("selected: ");
        guiEntityContents(ctx, *(Entity*)ctx.gui.selectedEntity);
    }

    ImGui::Separator();

    ImGui::End();
}

void gameUpdateAndRender(Context& ctx)
{
    const auto timeScale = ctx.timeScale * (float)!ctx.pause;
    const auto dt = ctx.dt * timeScale;

    if (wasKeyPressed(KeyboardKey::KEY_SPACE))
        ctx.pause = !ctx.pause;
    if (wasKeyPressed(KeyboardKey::KEY_Z))
        ctx.timeScale = std::max(0.f, ctx.timeScale += -1);
    if (wasKeyPressed(KeyboardKey::KEY_X))
        ctx.timeScale += 1;

    float time = getElapsedTime();
    for (auto& entity : ctx.entityManager.entities)
    {
        if (hasType(entity, EntityType::Drawable))
            setShaderVariableFloat(*entity.drawCommand, "time", time);
    }

    if (isKeyPressed(KeyboardKey::KEY_ESCAPE))
        ctx.wantsToQuit = true;

    if (wasKeyPressed(KeyboardKey::KEY_R))
        ctx.wantsToReload = true;

    cameraControllerUpdate(ctx.dt, ctx.input, ctx.gameState.cameraController);

    const auto speed = 75.f * dt;
    const auto sine = std::sin(time) * dt;
    addLocalRotation(*ctx.gameState.cube, vec3(0, speed, 0));
    addLocalRotation(*ctx.gameState.sphere, vec3(speed * 0.7, 0, 0));
    addLocalPosition(*ctx.gameState.lightOrigin, vec3(0, sine, 0));

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
    renderPresentAndResize();

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
