#include "game.hpp"
#include "lib/common.hpp"
#include "lib/log.hpp"
#include "app/context.hpp"

#include <cstring>
#include <functional>
#include <raylib.h>
#include <raymath.h>
#include "math.hpp"
#include "rcommon.hpp"
#include "rlgl.h"

#include "rlImGui.h"
#include "imgui.h"

#include "shaders.hpp"

#define internal static

struct Light
{
    vec3 pos;
};

struct ModelEx
{
    Model base;

    char name[128];

    vec3 pos{};
    vec3 eulerAngles{};
    float scale = 1.f;

    ShaderEx shader;
    int lightPosLoc = -1;
};

ModelEx createSphere()
{
    ModelEx result{};

    strcpy_s(result.name, "sphere");
    result.base = LoadModelFromMesh(GenMeshSphere(1.f, 32, 32));

    return result;
}

ModelEx createCube()
{
    ModelEx result{};

    strcpy_s(result.name, "cube");
    result.base = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));

    return result;
}

void setModelShader(ModelEx& model, ShaderEx& shader)
{
    model.base.materials[0].shader = shader.base;
    model.shader = shader;
    model.lightPosLoc = findUniformLocation(model.shader.uniforms, "lightPos");
}

void updateModel(ModelEx& model, Light& light)
{
    setShaderUniform(model.base.materials[0].shader, model.lightPosLoc, light.pos);

    ImGui::Text("%s", model.name);
    debugDrawShader(model.shader, model.name);

    model.base.transform = MatrixRotateXYZ(rlv3(model.eulerAngles));
    DrawModel(model.base, rlv3(model.pos), model.scale, BLACK);
}

struct GameState
{
    bool isInitialized;

    Camera3D camera;

    Light light;

    ModelEx sphere;
    ModelEx cube;
    ModelEx visualLight;

    ShaderEx basicShader;
    ShaderEx lightShader;
};

internal void restart(GameState& gameState, Arena& tempArena)
{
    static constexpr auto cameraOffset = 10.f;

    gameState.camera.fovy = 60.f;
    gameState.camera.position = {0, cameraOffset * 0.5f, -cameraOffset};
    gameState.camera.target = {0, 0, 0.f};
    gameState.camera.up = {0, 1.f, 0};
    gameState.camera.projection = CAMERA_PERSPECTIVE;

    if (gameState.isInitialized)
    {
        deinitShader(gameState.basicShader);
        deinitShader(gameState.lightShader);
    }

    gameState.basicShader = initShader("basic", BasicShader::vs, BasicShader::fs, tempArena);
    gameState.lightShader = initShader("light", LightReceiverShader::vs, LightReceiverShader::fs, tempArena);

    setModelShader(gameState.sphere, gameState.lightShader);
    setModelShader(gameState.cube, gameState.lightShader);
    setModelShader(gameState.visualLight, gameState.basicShader);
}

void gameInit(void* contextPtr)
{
    auto& context = CTX(contextPtr);
    GameState& gameState = *(GameState*)context.memory.buffer;

    gameState.isInitialized = true;

    gameState.sphere = createSphere();
    gameState.cube = createCube();
    gameState.visualLight = createCube();

    restart(gameState, context.tempMemory);
}

void gamePreHotReload(void* contextPtr) {}

void gamePostHotReload(void* contextPtr)
{
    auto& context = CTX(contextPtr);
    restart(*(GameState*)context.memory.buffer, context.tempMemory);
}

void gameUpdate(void* contextPtr)
{
    auto& context = CTX(contextPtr);
    auto& gameState = *(GameState*)context.memory.buffer;

    UpdateCamera(&gameState.camera, CameraMode::CAMERA_ORBITAL);

    const auto mouseRay = GetScreenToWorldRay(GetMousePosition(), gameState.camera);
    gameState.light.pos = rayPlaneIntersectionResult(
        glmv3(mouseRay.position), glmv3(mouseRay.direction), 2.f, {gameState.camera.position.x, gameState.camera.position.y, 0});

    BeginDrawing();
    ClearBackground(BLACK);
    rlImGuiBegin();
    BeginMode3D(gameState.camera);

    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("hello");

    updateModel(gameState.sphere, gameState.light);
    updateModel(gameState.cube, gameState.light);

    DrawModel(gameState.visualLight.base, rlv3(gameState.light.pos), 0.2f, BLACK);

    ImGui::End();

    DrawGrid(100, 1);
    EndMode3D();

    DrawFPS(10, 10);
    rlImGuiEnd();
    EndDrawing();

    gameState.sphere.eulerAngles.z += GetFrameTime();
    gameState.cube.eulerAngles.z += GetFrameTime();
}
