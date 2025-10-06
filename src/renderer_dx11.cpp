#include "common/utils.hpp"
#include "geometry.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <wrl/client.h>

using namespace Microsoft::WRL;

ComPtr<ID3D11Device> device;
ComPtr<IDXGISwapChain> swapChain;
ComPtr<ID3D11DeviceContext> deviceContext;
ComPtr<ID3D11RenderTargetView> rtView;

ComPtr<ID3D11Buffer> vertexBuffers[(i32)MeshType::Max];
ComPtr<ID3D11Buffer> indexBuffers[(i32)MeshType::Max];

struct ConstantBufferFieldMapping
{
    const char* name;
    size_t offsetBytes;
    size_t sizeBytes;
    ShaderVariableValue defaultValue;
};

struct ConstantBuffer
{
    ComPtr<ID3D11Buffer> buffer;
    ConstantBufferFieldMapping mappings[MAX_SHADER_VARIABLES];
};

static ConstantBufferFieldMapping _createFieldMapping(const char* name, size_t offsetBytes, size_t sizeBytes, const void* value)
{
    ConstantBufferFieldMapping mapping{};
    mapping.name = name;
    mapping.offsetBytes = offsetBytes;
    mapping.sizeBytes = sizeBytes;
    memcpy(&mapping.defaultValue, value, sizeof(ConstantBufferFieldMapping::defaultValue));
    return mapping;
}
#define createFieldMapping(bufferStruct, bufferField, value) \
    _createFieldMapping((#bufferField), offsetof(bufferStruct, bufferField), sizeof(bufferStruct::bufferField), (value))

ConstantBuffer constantBuffers[(i32)ShaderType::Max] = {};

struct Shader
{
    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11PixelShader> ps;
    ComPtr<ID3D11InputLayout> layout;
};

Shader shaders[(i32)ShaderType::Max] = {};

ComPtr<ID3D11RasterizerState> rasterizerStates[(i32)RasterizerState::Max] = {};

static ID3DBlob* compileShader(const wchar_t* srcPath, const char* entryPoint, const char* target)
{
    ID3DBlob* resultBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    const auto hr = D3DCompileFromFile(srcPath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        target,
        D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        &resultBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        wprintf(L"shader %hs source path: %ls", target, srcPath);
        logError("shader %s compile error: %s", target, errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    else
    {
        logInfo("shader %s compile success", target);
    }

    return resultBlob;
}

static Shader createShader(const wchar_t* path)
{
    Shader shader;

    const auto vsBlob = compileShader(path, "VS_Main", "vs_5_0");
    defer({ vsBlob->Release(); });
    const auto psBlob = compileShader(path, "PS_Main", "ps_5_0");
    defer({ psBlob->Release(); });

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    HR_ASSERT(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs));
    HR_ASSERT(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps));
    shader.vs = vs;
    shader.ps = ps;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(Vertex::pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    ID3D11InputLayout* layout = nullptr;
    HR_ASSERT(device->CreateInputLayout(
        layoutDesc, ARR_LENGTH(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &layout));
    shader.layout = layout;

    return shader;
}

static ID3D11Buffer* createVertexBuffer(Vertex const* vertices, int vertexCount)
{
    ID3D11Buffer* buffer;

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = sizeof(Vertex);
    bd.ByteWidth = sizeof(Vertex) * vertexCount;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    HR_ASSERT(device->CreateBuffer(&bd, &sd, &buffer));

    return buffer;
}

static ID3D11Buffer* createIndexBuffer(u32 const* indices, int indexCount)
{
    ID3D11Buffer* buffer;

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.ByteWidth = sizeof(u32) * indexCount;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = indices;

    HR_ASSERT(device->CreateBuffer(&bd, &sd, &buffer));

    return buffer;
}

static ID3D11Buffer* createConstantBuffer(ShaderType shaderType, D3D11_USAGE usage)
{
    ID3D11Buffer* constantBuffer = nullptr;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = usage;
    switch (shaderType)
    {
        case ShaderType::Basic: cbDesc.ByteWidth = sizeof(Shaders::Basic::Variables); break;
        case ShaderType::Unlit: cbDesc.ByteWidth = sizeof(Shaders::Unlit::Variables); break;
        default: LOGIC_ERROR();
    }
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (usage == D3D11_USAGE_DYNAMIC)
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HR_ASSERT(device->CreateBuffer(&cbDesc, nullptr, &constantBuffer));

    return constantBuffer;
}

static ID3D11RasterizerState* createRasterizerState(RasterizerState state)
{
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = state == RasterizerState::Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;  // state == RasterizerState::Wireframe ? D3D11_CULL_NONE : D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;

    ID3D11RasterizerState* rasterizerState = nullptr;
    HR_ASSERT(device->CreateRasterizerState(&rasterizerDesc, &rasterizerState));

    return rasterizerState;
}

void renderInit(void* window, float windowWidth, float windowHeight)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)window;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HR_ASSERT(D3D11CreateDeviceAndSwapChain(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &device,
        nullptr,
        &deviceContext));

    ComPtr<ID3D11Resource> backbuffer{};
    HR_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backbuffer));
    HR_ASSERT(device->CreateRenderTargetView(backbuffer.Get(), nullptr, &rtView));

    deviceContext->OMSetRenderTargets(1, rtView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    deviceContext->RSSetViewports(1, &vp);

    vertexBuffers[(i32)MeshType::Triangle] = createVertexBuffer(TRIANGLE_VERTICES, ARR_LENGTH(TRIANGLE_VERTICES));
    indexBuffers[(i32)MeshType::Triangle] = createIndexBuffer(TRIANGLE_INDICES, ARR_LENGTH(TRIANGLE_INDICES));

    vertexBuffers[(i32)MeshType::Quad] = createVertexBuffer(QUAD_VERTICES, ARR_LENGTH(QUAD_VERTICES));
    indexBuffers[(i32)MeshType::Quad] = createIndexBuffer(QUAD_INDICES, ARR_LENGTH(QUAD_INDICES));

    vertexBuffers[(i32)MeshType::Cube] = createVertexBuffer(CUBE_VERTICES, ARR_LENGTH(CUBE_VERTICES));
    indexBuffers[(i32)MeshType::Cube] = createIndexBuffer(CUBE_INDICES, ARR_LENGTH(CUBE_INDICES));

    generateSphere(SPHERE_RADIUS,
        SPHERE_STACKS,
        SPHERE_SLICES,
        SPHERE_VERTICES,
        ARR_LENGTH(SPHERE_VERTICES),
        SPHERE_INDICES,
        ARR_LENGTH(SPHERE_INDICES));
    vertexBuffers[(i32)MeshType::Sphere] = createVertexBuffer(SPHERE_VERTICES, ARR_LENGTH(SPHERE_VERTICES));
    indexBuffers[(i32)MeshType::Sphere] = createIndexBuffer(SPHERE_INDICES, ARR_LENGTH(SPHERE_INDICES));

    generateGrid(GRID_X, GRID_Y, GRID_SPACING, GRID_VERTICES, ARR_LENGTH(GRID_VERTICES), GRID_INDICES, ARR_LENGTH(GRID_INDICES));
    vertexBuffers[(i32)MeshType::Grid] = createVertexBuffer(GRID_VERTICES, ARR_LENGTH(GRID_VERTICES));
    indexBuffers[(i32)MeshType::Grid] = createIndexBuffer(GRID_INDICES, ARR_LENGTH(GRID_INDICES));

    u32* strides = (u32*)alloca(sizeof(u32) * (i32)MeshType::Max);
    u32* offsets = (u32*)alloca(sizeof(u32) * (i32)MeshType::Max);
    for (i32 i = 0; i < (i32)MeshType::Max; ++i)
    {
        strides[i] = sizeof(Vertex);
        offsets[i] = 0;
    }

    ID3D11Buffer* vertexBufferPointers[(i32)MeshType::Max];
    for (i32 i = 0; i < (i32)MeshType::Max; ++i)
        vertexBufferPointers[i] = vertexBuffers[i].Get();
    deviceContext->IASetVertexBuffers(0, (i32)MeshType::Max, vertexBufferPointers, strides, offsets);

    shaders[(i32)ShaderType::Basic] = createShader(Shaders::Basic::PATH);
    constantBuffers[(i32)ShaderType::Basic] = {.buffer = createConstantBuffer(ShaderType::Basic, D3D11_USAGE_DYNAMIC),
        .mappings = {
            createFieldMapping(Shaders::Basic::Variables, mvp, &Shaders::Basic::DEFAULT_VARIABLES.mvp),
            createFieldMapping(Shaders::Basic::Variables, time, &Shaders::Basic::DEFAULT_VARIABLES.time),
            createFieldMapping(Shaders::Basic::Variables, objectColor, &Shaders::Basic::DEFAULT_VARIABLES.objectColor),
            createFieldMapping(Shaders::Basic::Variables, world, &Shaders::Basic::DEFAULT_VARIABLES.world),
            createFieldMapping(Shaders::Basic::Variables, lightPosition, &Shaders::Basic::DEFAULT_VARIABLES.lightPosition),
            createFieldMapping(Shaders::Basic::Variables, lightDirection, &Shaders::Basic::DEFAULT_VARIABLES.lightDirection),
            createFieldMapping(Shaders::Basic::Variables, lightColor, &Shaders::Basic::DEFAULT_VARIABLES.lightColor),
            createFieldMapping(Shaders::Basic::Variables, lightType, &Shaders::Basic::DEFAULT_VARIABLES.lightType),
        }};

    shaders[(i32)ShaderType::Unlit] = createShader(Shaders::Unlit::PATH);
    constantBuffers[(i32)ShaderType::Unlit] = {.buffer = createConstantBuffer(ShaderType::Unlit, D3D11_USAGE_DYNAMIC),
        .mappings = {
            createFieldMapping(Shaders::Unlit::Variables, mvp, &Shaders::Unlit::DEFAULT_VARIABLES.mvp),
            createFieldMapping(Shaders::Unlit::Variables, time, &Shaders::Unlit::DEFAULT_VARIABLES.time),
            createFieldMapping(Shaders::Unlit::Variables, objectColor, &Shaders::Unlit::DEFAULT_VARIABLES.objectColor),
        }};

    rasterizerStates[(i32)RasterizerState::Default] = createRasterizerState(RasterizerState::Default);
    rasterizerStates[(i32)RasterizerState::Wireframe] = createRasterizerState(RasterizerState::Wireframe);
}

void renderDeinit()
{
    if (deviceContext)
        deviceContext.Reset();
    if (rtView)
        rtView.Reset();
    if (swapChain)
        swapChain.Reset();
    if (device)
        device.Reset();

    for (auto& buffer : vertexBuffers)
        buffer.Reset();
    for (auto& buffer : indexBuffers)
        buffer.Reset();
    for (auto& cb : constantBuffers)
        cb.buffer.Reset();
    for (auto& shader : shaders)
    {
        shader.vs.Reset();
        shader.ps.Reset();
        shader.layout.Reset();
    }
    for (auto& state : rasterizerStates)
        state.Reset();

    if (deviceContext)
    {
        deviceContext->ClearState();
        deviceContext->Flush();
    }
}

static void writeShaderVariables(ShaderType shader, const ShaderVariable* variables, size_t variablesCount)
{
    if (shader >= ShaderType::Max)
        LOGIC_ERROR();

    auto buffer = constantBuffers[(i32)shader];

    size_t dataSizeBytes = 0;
    u8* data = nullptr;
    if (shader == ShaderType::Basic)
    {
        dataSizeBytes = sizeof(Shaders::Basic::Variables);
        data = (u8*)alloca(dataSizeBytes);
    }
    else if (shader == ShaderType::Unlit)
    {
        dataSizeBytes = sizeof(Shaders::Unlit::Variables);
        data = (u8*)alloca(dataSizeBytes);
    }

    for (size_t i = 0; i < variablesCount; ++i)
    {
        const auto& variable = variables[i];
        if (!variable.name)
            continue;

        const auto mapping = find(buffer.mappings,
            MAX_SHADER_VARIABLES,
            [variable](auto const& mapping) { return strncmp(mapping.name, variable.name, 256) == 0; });
        if (mapping)
        {
            memcpy(data + mapping->offsetBytes, &variable.value, mapping->sizeBytes);
        }
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR_ASSERT(deviceContext->Map(buffer.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    memcpy((uint8_t*)mappedResource.pData, data, dataSizeBytes);

    deviceContext->Unmap(buffer.buffer.Get(), 0);

    deviceContext->VSSetConstantBuffers(0, 1, buffer.buffer.GetAddressOf());
    deviceContext->PSSetConstantBuffers(0, 1, buffer.buffer.GetAddressOf());
}

void renderDraw(DrawCommand const& command)
{
    writeShaderVariables(command.shader, command.variables, MAX_SHADER_VARIABLES);

    deviceContext->IASetPrimitiveTopology(command.rasterizerState == RasterizerState::Wireframe
                                              ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST
                                              : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto& shader = shaders[(i32)command.shader];
    deviceContext->IASetInputLayout(shader.layout.Get());
    deviceContext->VSSetShader(shader.vs.Get(), nullptr, 0);
    deviceContext->PSSetShader(shader.ps.Get(), nullptr, 0);

    deviceContext->RSSetState(rasterizerStates[(i32)command.rasterizerState].Get());

    u32 stride = sizeof(Vertex), offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffers[(i32)command.mesh].GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffers[(i32)command.mesh].Get(), DXGI_FORMAT_R32_UINT, 0);
    switch (command.mesh)
    {
        case MeshType::Triangle: deviceContext->DrawIndexed(ARR_LENGTH(TRIANGLE_INDICES), 0, 0); break;
        case MeshType::Quad: deviceContext->DrawIndexed(ARR_LENGTH(QUAD_INDICES), 0, 0); break;
        case MeshType::Cube: deviceContext->DrawIndexed(ARR_LENGTH(CUBE_INDICES), 0, 0); break;
        case MeshType::Sphere: deviceContext->DrawIndexed(ARR_LENGTH(SPHERE_INDICES), 0, 0); break;
        case MeshType::Grid: deviceContext->DrawIndexed(ARR_LENGTH(GRID_INDICES), 0, 0); break;
        default: LOGIC_ERROR();
    }
}

void renderClear(glm::vec4 color)
{
    deviceContext->ClearRenderTargetView(rtView.Get(), &color.x);
}

void renderPresent()
{
    swapChain->Present(1, 0);
}

void createShaderVariables(DrawCommand& command)
{
    for (int i = 0; i < MAX_SHADER_VARIABLES; ++i)
    {
        auto& mapping = constantBuffers[(i32)command.shader].mappings[i];
        command.variables[i] = {
            .name = mapping.name,
            .value = {},
        };
        memcpy(&command.variables[i].value, &mapping.defaultValue, mapping.sizeBytes);
    }
}

static ShaderVariable* getVariableByName(ShaderVariable* variables, const char* name)
{
    auto var = find(variables,
        MAX_SHADER_VARIABLES,
        [name](ShaderVariable& var)
        {
            if (!var.name)
                return false;
            return strncmp(var.name, name, 256) == 0;
        });
    ENSURE(var != nullptr);
    return var;
}

void setShaderVariableInt(DrawCommand& command, const char* variableName, int value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.i = value;
    var->value = newValue;
}

void setShaderVariableFloat(DrawCommand& command, const char* variableName, float value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.f = value;
    var->value = newValue;
}

void setShaderVariableVec2(DrawCommand& command, const char* variableName, vec2 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v2 = value;
    var->value = newValue;
}

void setShaderVariableVec3(DrawCommand& command, const char* variableName, vec3 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v3 = value;
    var->value = newValue;
}

void setShaderVariableVec4(DrawCommand& command, const char* variableName, vec4 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.v4 = value;
    var->value = newValue;
}

void setShaderVariableMat4(DrawCommand& command, const char* variableName, mat4 value)
{
    auto var = getVariableByName(command.variables, variableName);
    ShaderVariableValue newValue;
    newValue.m4 = value;
    var->value = newValue;
}

mat4 getShaderVariableMat4(DrawCommand& command, const char* variableName)
{
    if (command.shader >= ShaderType::Max)
        LOGIC_ERROR();

    auto buffers = constantBuffers[(i32)command.shader];

    const auto mapping = find(buffers.mappings,
        ARR_LENGTH(buffers.mappings),
        [variableName](auto const& mapping) { return strncmp(mapping.name, variableName, 256) == 0; });
    ENSURE(mapping != nullptr);

    const auto buffer = buffers.buffer;

    // Create a staging buffer to read the constant buffer
    D3D11_BUFFER_DESC cbDesc;
    buffer->GetDesc(&cbDesc);

    D3D11_BUFFER_DESC stagingDesc = cbDesc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    ID3D11Buffer* stagingBuffer = nullptr;
    HR_ASSERT(device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer));

    // Copy constant buffer to staging buffer
    deviceContext->CopyResource(stagingBuffer, buffer.Get());

    // Map the staging buffer to read data
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR_ASSERT(deviceContext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource));

    // Read the matrix data (assuming mat4)
    glm::mat4 matrix;
    memcpy(&matrix, mappedResource.pData, sizeof(glm::mat4));

    // Unmap and release staging buffer
    deviceContext->Unmap(stagingBuffer, 0);
    stagingBuffer->Release();

    return matrix;
}
