#include "game.hpp"
#include "lib/log.hpp"
#include "app/context.hpp"

#include <GLFW/glfw3.h>
#include <raylib.h>
#include <raymath.h>
#include "math.hpp"
#include "rcommon.hpp"
#include "rlgl.h"

#include "lib/heap_array.hpp"

#include <imgui.h>

struct Light
{
    vec3 pos;
    Color color;
    Shader shader;
    int lightPosLoc;
    int colorLoc;
};

struct ShaderUniform
{
    const char* name;
    int location;
    ShaderUniformDataType type;
};

using ShaderUniforms = HeapArray<ShaderUniform>;

void fillShaderUniforms(ShaderUniforms& cache, Shader shader)
{
    int uniformsCount = 0;
    rlGetProgram(shader.id, RL_ACTIVE_UNIFORMS, &uniformsCount);

    int maxNameLength = 0;
    rlGetProgram(shader.id, RL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

    char* uniformName = (char*)_alloca(maxNameLength);

    for (int i = 0; i < uniformsCount; i++)
    {
        i32 length;
        i32 size;
        i32 type;
        rlGetActiveUniform(shader.id, i, maxNameLength, &length, &size, &type, uniformName);

        logInfo("Uniform #%d: Name: %s, Type: %x, Size: %d\n", i, uniformName, type, size);
    }

    _freea(uniformName);

    //  for (const auto uniform : allUniforms)
    //  {
    //      arrayPush(cache, {uniform.name, GetShaderLocation(shader, uniform.name), uniform.type});
    //  }
}

struct ModelEx
{
    Model model;

    vec3 pos{};
    vec3 eulerAngles{};
    float scale = 1.f;

    ShaderUniforms shaderUniforms;
};

ModelEx createSphere(Shader shader)
{
    ModelEx result{};

    const auto mesh = GenMeshSphere(1.f, 32, 32);
    result.model = LoadModelFromMesh(mesh);
    result.model.materials[0].shader = shader;

    fillShaderUniforms(result.shaderUniforms, shader);

    return result;
}

ModelEx createCube(Shader shader)
{
    ModelEx result{};

    const auto mesh = GenMeshCube(1.f, 1.f, 1.f);
    result.model = LoadModelFromMesh(mesh);
    result.model.materials[0].shader = shader;

    fillShaderUniforms(result.shaderUniforms, shader);

    return result;
}

Light createLight(Color color, Shader shader)
{
    const auto colorLoc = GetShaderLocation(shader, "lightColor");
    float colorIn[3] = {color.r / 255.f, color.g / 255.f, color.b / 255.f};
    SetShaderValue(shader, colorLoc, colorIn, SHADER_UNIFORM_VEC3);

    const auto lightPosLoc = GetShaderLocation(shader, "lightPos");
    float posIn[3]{};
    SetShaderValue(shader, lightPosLoc, posIn, SHADER_UNIFORM_VEC3);

    return {.pos = {}, .color = color, .shader = shader, .lightPosLoc = lightPosLoc, .colorLoc = colorLoc};
}

void imguiUniform(Shader shader, const char* name, int location, ShaderUniformDataType type)
{
    if (type == SHADER_UNIFORM_FLOAT)
    {
        float value{};
        if (ImGui::DragFloat(name, &value, 0, 100))
            SetShaderValue(shader, location, &value, type);
    }
    else if (type == SHADER_UNIFORM_VEC2)
    {
        vec2 value{};
        if (ImGui::DragFloat2(name, &value.x, 0, 100))
            SetShaderValue(shader, location, &value, type);
    }
    else if (type == SHADER_UNIFORM_VEC3)
    {
        vec3 value{};
        if (ImGui::DragFloat3(name, &value.x, 0, 100))
            SetShaderValue(shader, location, &value, type);
    }
    else if (type == SHADER_UNIFORM_VEC4)
    {
        vec4 value{};
        if (ImGui::DragFloat4(name, &value.x, 0, 100))
            SetShaderValue(shader, location, &value, type);
    }
    else
    {
        logInfo("[imguiUniform] type not implemented");
    }
}

void updateModel(ModelEx& model)
{
    for (const auto& uniform : model.shaderUniforms)
        imguiUniform(model.model.materials[0].shader, uniform.name, uniform.location, uniform.type);

    model.model.transform = MatrixRotateXYZ(rlv3(model.eulerAngles));
    DrawModel(model.model, rlv3(model.pos), model.scale, BLACK);
}

struct GameState
{
    bool isInitialized;

    Camera3D camera;

    Light light;

    ModelEx sphere;
    ModelEx cube;
    ModelEx visualLight;

    Shader basicShader;
    ShaderUniforms basicShaderUniforms;
    Shader lightShader;
    ShaderUniforms lightShaderUniforms;
};

void gameInit(void* contextPtr)
{
    GameState& gameState = *(GameState*)CTX(contextPtr).memory.buffer;

    static constexpr auto cameraOffset = 10.f;

    gameState.camera.fovy = 60.f;
    gameState.camera.position = {0, 0, -cameraOffset};
    gameState.camera.target = {0, 0, 0.f};
    gameState.camera.up = {0, 1.f, 0};
    gameState.camera.projection = CAMERA_PERSPECTIVE;

    gameState.basicShader = LoadShader("resources/shaders/basic.vs", "resources/shaders/basic.fs");
    const auto lightReceiverShader = LoadShader("resources/shaders/light.vs", "resources/shaders/light.fs");

    lightReceiverShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lightReceiverShader, "viewPos");

    gameState.sphere = createSphere(lightReceiverShader);
    gameState.cube = createCube(lightReceiverShader);
    gameState.visualLight = createCube(gameState.basicShader);

    const auto lightColor = GREEN;
    gameState.light = createLight(lightColor, lightReceiverShader);
}

void gameUpdate(void* contextPtr)
{
    auto& context = CTX(contextPtr);
    auto& gameState = *(GameState*)context.memory.buffer;

    UpdateCamera(&gameState.camera, CameraMode::CAMERA_FREE);

    setShaderUniform(gameState.basicShader, GetShaderLocation(gameState.basicShader, "color"), WHITE);

    setShaderUniform(gameState.light.shader, gameState.light.shader.locs[SHADER_LOC_VECTOR_VIEW], gameState.camera.position);
    setShaderUniform(gameState.light.shader, gameState.light.colorLoc, gameState.light.color);
    setShaderUniform(gameState.light.shader, gameState.light.lightPosLoc, rlv3(gameState.light.pos));

    const auto mouseRay = GetScreenToWorldRay(GetMousePosition(), gameState.camera);
    gameState.light.pos = rayPlaneIntersectionResult(
        glmv3(mouseRay.position), glmv3(mouseRay.direction), 2.f, {gameState.camera.position.x, gameState.camera.position.y, 0});

    BeginDrawing();
    ClearBackground(BLACK);
    reinterpret_cast<void (*)()>(context.rlImgui.rlImGuiBegin)();
    ImGui::SetCurrentContext(context.rlImgui.context);
    BeginMode3D(gameState.camera);

    ImGui::Begin("hello");

    updateModel(gameState.sphere);
    updateModel(gameState.cube);

    DrawModel(gameState.visualLight.model, rlv3(gameState.light.pos), 0.2f, BLACK);

    ImGui::End();

    DrawGrid(100, 1);
    EndMode3D();

    DrawFPS(10, 10);
    reinterpret_cast<void (*)()>(context.rlImgui.rlImGuiEnd)();
    EndDrawing();

    gameState.sphere.eulerAngles.z += GetFrameTime();
    gameState.cube.eulerAngles.z += GetFrameTime();
}
