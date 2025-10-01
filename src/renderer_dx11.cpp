#include "geometry.hpp"
#include "renderer.hpp"
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include "shaders/dx11/basic.hpp"

ID3D11Device* device;
IDXGISwapChain* swapChain;
ID3D11DeviceContext* deviceContext;
ID3D11RenderTargetView* rtView;

ID3D11Buffer* vertexBuffers[(i32)MeshType::Max] = {nullptr, nullptr};
ID3D11Buffer* indexBuffers[(i32)MeshType::Max] = {nullptr, nullptr};

struct BasicVSConstantBuffer
{
    mat4 mvp;
};
struct BasicPSConstantBuffer
{
    vec4 color;
};

enum class ConstantBufferType
{
    VertexShader,
    PixelShader
};

struct ConstantBufferFieldMapping
{
    const char* name;
    ConstantBufferType bufferType;
    size_t offsetBytes;
    size_t sizeBytes;
};

struct ConstantBuffer
{
    ID3D11Buffer* vsBuffer;
    ID3D11Buffer* psBuffer;
    ConstantBufferFieldMapping mappings[MAX_SHADER_VARIABLES];
};

ConstantBuffer constantBuffers[(i32)ShaderType::Max] = {};

struct Shader
{
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* layout;
};

Shader shaders[(i32)ShaderType::Max] = {};

ID3D11RasterizerState* rasterizerStates[(i32)RasterizerState::Max] = {};

static ID3DBlob* compileShader(const char* src, const char* target)
{
    ID3DBlob* resultBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    const auto hr = D3DCompile(src,
        strlen(src),
        nullptr,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        target,
        D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        &resultBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        logError("shader %s source: %s", target, src);
        logError("shader %s compile error: %s", target, errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    else
    {
        logInfo("shader %s compile success", target);
    }

    return resultBlob;
}

static Shader createShader(const char* vsSrc, const char* psSrc)
{
    Shader shader;

    const auto vsBlob = compileShader(vsSrc, "vs_5_0");
    defer({ vsBlob->Release(); });
    const auto psBlob = compileShader(psSrc, "ps_5_0");
    defer({ psBlob->Release(); });

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    HR_ASSERT(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs));
    HR_ASSERT(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps));
    shader.vs = vs;
    shader.ps = ps;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}};
    ID3D11InputLayout* layout = nullptr;
    HR_ASSERT(device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &layout));
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
        case ShaderType::Basic: cbDesc.ByteWidth = sizeof(BasicVSConstantBuffer); break;
        default: LOGIC_ERROR();
    }
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (usage == D3D11_USAGE_DYNAMIC)
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);

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
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &device,
        nullptr,
        &deviceContext));

    ID3D11Resource* backbuffer{};
    defer({ backbuffer->Release(); });
    HR_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backbuffer));
    HR_ASSERT(device->CreateRenderTargetView(backbuffer, nullptr, &rtView));

    deviceContext->OMSetRenderTargets(1, &rtView, nullptr);

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
        memset(strides + i, sizeof(Vertex), sizeof(u32));
    for (i32 i = 0; i < (i32)MeshType::Max; ++i)
        memset(offsets + i, 0, sizeof(u32));
    deviceContext->IASetVertexBuffers(0, 4, vertexBuffers, strides, offsets);

    shaders[(i32)ShaderType::Basic] = createShader(Shaders::Basic::vs, Shaders::Basic::fs);
    constantBuffers[(i32)ShaderType::Basic] = {.vsBuffer = createConstantBuffer(ShaderType::Basic, D3D11_USAGE_DYNAMIC),
        .psBuffer = createConstantBuffer(ShaderType::Basic, D3D11_USAGE_DYNAMIC),
        .mappings = {
            {.name = "mvp",
                .bufferType = ConstantBufferType::VertexShader,
                .offsetBytes = offsetof(BasicVSConstantBuffer, mvp),
                .sizeBytes = sizeof(mat4)},
            {.name = "color",
                .bufferType = ConstantBufferType::PixelShader,
                .offsetBytes = offsetof(BasicPSConstantBuffer, color),
                .sizeBytes = sizeof(vec4)},
        }};

    rasterizerStates[(i32)RasterizerState::Default] = createRasterizerState(RasterizerState::Default);
    rasterizerStates[(i32)RasterizerState::Wireframe] = createRasterizerState(RasterizerState::Wireframe);
}

void renderDeinit()
{
    if (device)
        device->Release();
    if (swapChain)
        swapChain->Release();
    if (deviceContext)
        deviceContext->Release();
    if (rtView)
        rtView->Release();
}

static void writeShaderVariable(ShaderType shader, const char* variableName, void* value)
{
    if (shader >= ShaderType::Max)
        LOGIC_ERROR();

    auto buffers = constantBuffers[(i32)shader];

    const auto mapping = find(buffers.mappings,
        ARR_LENGTH(buffers.mappings),
        [variableName](auto const& mapping) { return strncmp(mapping.name, variableName, 256) == 0; });
    ENSURE(mapping != buffers.mappings + ARR_LENGTH(buffers.mappings));

    const auto buffer = mapping->bufferType == ConstantBufferType::VertexShader ? buffers.vsBuffer : buffers.psBuffer;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HR_ASSERT(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    const auto data = (uint8_t*)mappedResource.pData;
    memcpy(data + mapping->offsetBytes, value, mapping->sizeBytes);

    deviceContext->Unmap(buffer, 0);
    if (mapping->bufferType == ConstantBufferType::VertexShader)
        deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    else
        deviceContext->PSSetConstantBuffers(1, 1, &buffer);
}

void renderDraw(DrawCommand const& command)
{
    for (const auto& variable : command.variables)
    {
        if (!variable.name)
            continue;
        writeShaderVariable(command.shader, variable.name, (void*)&variable.value);
    }

    deviceContext->IASetPrimitiveTopology(command.rasterizerState == RasterizerState::Wireframe
                                              ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST
                                              : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto& shader = shaders[(i32)command.shader];
    deviceContext->IASetInputLayout(shader.layout);
    deviceContext->VSSetShader(shader.vs, nullptr, 0);
    deviceContext->PSSetShader(shader.ps, nullptr, 0);

    deviceContext->RSSetState(rasterizerStates[(i32)command.rasterizerState]);

    u32 stride = sizeof(Vertex), offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffers[(i32)command.mesh], &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffers[(i32)command.mesh], DXGI_FORMAT_R32_UINT, 0);
    switch (command.mesh)
    {
        case MeshType::Triangle: deviceContext->DrawIndexed(ARR_LENGTH(TRIANGLE_INDICES), 0, 0); break;
        case MeshType::Quad: deviceContext->DrawIndexed(ARR_LENGTH(QUAD_INDICES), 0, 0); break;
        case MeshType::Sphere: deviceContext->DrawIndexed(ARR_LENGTH(SPHERE_INDICES), 0, 0); break;
        case MeshType::Grid: deviceContext->DrawIndexed(ARR_LENGTH(GRID_INDICES), 0, 0); break;
        default: LOGIC_ERROR();
    }
}

void renderClear(glm::vec4 color)
{
    deviceContext->ClearRenderTargetView(rtView, &color.x);
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
    }
}

static ShaderVariable* getVariableByName(ShaderVariable* variables, const char* name)
{
    auto var = find(variables, MAX_SHADER_VARIABLES, [name](ShaderVariable& var) { return strncmp(var.name, name, 256) == 0; });
    ENSURE(var != variables + MAX_SHADER_VARIABLES, "shader variable %s not found!", name);
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
    ENSURE(mapping != buffers.mappings + ARR_LENGTH(buffers.mappings));

    const auto buffer = mapping->bufferType == ConstantBufferType::VertexShader ? buffers.vsBuffer : buffers.psBuffer;

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
    deviceContext->CopyResource(stagingBuffer, buffer);

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
