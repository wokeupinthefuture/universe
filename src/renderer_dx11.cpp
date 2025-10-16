#include "common/utils.hpp"
#include "geometry.hpp"
#include "platform.hpp"
#include "renderer.hpp"

#include "shaders.hpp"

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

ConstantBuffer constantBuffer{};

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

static ID3D11Buffer* createVertexBuffer(Vertex const* vertices, size_t vertexCount)
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

static ID3D11Buffer* createIndexBuffer(u32 const* indices, size_t indexCount)
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

static ID3D11Buffer* createConstantBuffer()
{
    ID3D11Buffer* buffer = nullptr;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(Shaders::Variables);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HR_ASSERT(device->CreateBuffer(&cbDesc, nullptr, &buffer));

    return buffer;
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

static void createScreenRenderTarget(vec2 size)
{
    ComPtr<ID3D11Resource> backbuffer{};
    HR_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backbuffer));
    HR_ASSERT(device->CreateRenderTargetView(backbuffer.Get(), nullptr, rtView.GetAddressOf()));

    deviceContext->OMSetRenderTargets(1, rtView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = size.x;
    vp.Height = size.y;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    deviceContext->RSSetViewports(1, &vp);
}

void renderInit(RenderState& state, void* window)
{
    const auto& triangle = state.generatedMeshes[(i32)MeshType::Triangle] = generateMesh(MeshType::Triangle);
    const auto& quad = state.generatedMeshes[(i32)MeshType::Quad] = generateMesh(MeshType::Quad);
    const auto& cube = state.generatedMeshes[(i32)MeshType::Cube] = generateMesh(MeshType::Cube);
    const auto& sphere = state.generatedMeshes[(i32)MeshType::Sphere] = generateMesh(MeshType::Sphere);
    const auto& grid = state.generatedMeshes[(i32)MeshType::Grid] = generateMesh(MeshType::Grid);

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

    createScreenRenderTarget(state.screenSize);

    vertexBuffers[(i32)MeshType::Triangle] = createVertexBuffer(triangle.vertices, triangle.verticesCount);
    indexBuffers[(i32)MeshType::Triangle] = createIndexBuffer(triangle.indices, triangle.indicesCount);

    vertexBuffers[(i32)MeshType::Quad] = createVertexBuffer(quad.vertices, quad.verticesCount);
    indexBuffers[(i32)MeshType::Quad] = createIndexBuffer(quad.indices, quad.indicesCount);

    vertexBuffers[(i32)MeshType::Cube] = createVertexBuffer(cube.vertices, cube.verticesCount);
    indexBuffers[(i32)MeshType::Cube] = createIndexBuffer(cube.indices, cube.indicesCount);

    vertexBuffers[(i32)MeshType::Sphere] = createVertexBuffer(sphere.vertices, sphere.verticesCount);
    indexBuffers[(i32)MeshType::Sphere] = createIndexBuffer(sphere.indices, sphere.indicesCount);

    vertexBuffers[(i32)MeshType::Grid] = createVertexBuffer(grid.vertices, grid.verticesCount);
    indexBuffers[(i32)MeshType::Grid] = createIndexBuffer(grid.indices, grid.indicesCount);

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
    shaders[(i32)ShaderType::Unlit] = createShader(Shaders::Unlit::PATH);

    constantBuffer = {.buffer = createConstantBuffer(),
        .mappings = {
            createFieldMapping(Shaders::Variables, mvp, &Shaders::DEFAULT_VARIABLES.mvp),
            createFieldMapping(Shaders::Variables, time, &Shaders::DEFAULT_VARIABLES.time),
            createFieldMapping(Shaders::Variables, objectColor, &Shaders::DEFAULT_VARIABLES.objectColor),
            createFieldMapping(Shaders::Variables, world, &Shaders::DEFAULT_VARIABLES.world),
            createFieldMapping(Shaders::Variables, lightPosition, &Shaders::DEFAULT_VARIABLES.lightPosition),
            createFieldMapping(Shaders::Variables, lightDirection, &Shaders::DEFAULT_VARIABLES.lightDirection),
            createFieldMapping(Shaders::Variables, lightColor, &Shaders::DEFAULT_VARIABLES.lightColor),
            createFieldMapping(Shaders::Variables, lightType, &Shaders::DEFAULT_VARIABLES.lightType),
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

    constantBuffer.buffer.Reset();

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

    auto buffer = constantBuffer;

    size_t dataSizeBytes = sizeof(Shaders::Variables);
    auto data = (u8*)alloca(dataSizeBytes);

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
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffers[(i32)command.mesh->type].GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffers[(i32)command.mesh->type].Get(), DXGI_FORMAT_R32_UINT, 0);
    if (command.mesh->isIndexed)
        deviceContext->DrawIndexed(command.mesh->indicesCount, 0, 0);
}

void renderClearAndResize(RenderState& state, glm::vec4 color)
{
    if (state.needsToResize)
    {
        deviceContext->OMSetRenderTargets(0, 0, nullptr);
        rtView.Reset();
        HR_ASSERT(swapChain->ResizeBuffers(0, state.screenSize.x, state.screenSize.y, DXGI_FORMAT_UNKNOWN, 0));
        createScreenRenderTarget(state.screenSize);

        state.needsToResize = false;
    }

    deviceContext->ClearRenderTargetView(rtView.Get(), &color.x);
}

void renderPresentAndResize()
{
    swapChain->Present(1, 0);
}

void createShaderVariables(DrawCommand& command)
{
    for (int i = 0; i < MAX_SHADER_VARIABLES; ++i)
    {
        auto& mapping = constantBuffer.mappings[i];
        command.variables[i] = {
            .name = mapping.name,
            .value = {},
        };
        memcpy(&command.variables[i].value, &mapping.defaultValue, mapping.sizeBytes);
    }
}
