#include "common/utils.hpp"
#include "geometry.hpp"
#include "input.hpp"
#include "platform.hpp"
#include "renderer.hpp"

#include "shaders.hpp"

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <wrl/client.h>

using namespace Microsoft::WRL;

static ComPtr<ID3D11Device> device;
static ComPtr<IDXGISwapChain> swapChain;
static ComPtr<ID3D11DeviceContext> deviceContext;
static ComPtr<ID3D11RenderTargetView> rtView;
static ComPtr<ID3D11DepthStencilView> dsView;
static ComPtr<ID3D11DepthStencilState> dss;

static ComPtr<ID3D11Buffer> vertexBuffers[(i32)MeshType::Max + (i32)AssetID::Max];
static ComPtr<ID3D11Buffer> indexBuffers[(i32)MeshType::Max + (i32)AssetID::Max];

static i32 getMeshBufferIndex(Mesh const& mesh)
{
    return (i32)mesh.type + (i32)mesh.customMeshID;
}

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

static void createRenderTargetAndDepthStencil(vec2 size)
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = size.x;
    vp.Height = size.y;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    deviceContext->RSSetViewports(1, &vp);

    ComPtr<ID3D11Resource> backbuffer{};
    HR_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backbuffer));
    HR_ASSERT(device->CreateRenderTargetView(backbuffer.Get(), nullptr, rtView.GetAddressOf()));

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = size.x;
    textureDesc.Height = size.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* texture;
    HR_ASSERT(device->CreateTexture2D(&textureDesc, nullptr, &texture));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = textureDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    HR_ASSERT(device->CreateDepthStencilView(texture, &dsvDesc, dsView.GetAddressOf()));

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};

    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HR_ASSERT(device->CreateDepthStencilState(&dsDesc, &dss));

    deviceContext->OMSetRenderTargets(1, rtView.GetAddressOf(), dsView.Get());
}

void renderInitGeometry(RenderState& state, const Asset* assets, Arena& permanentMemory, Arena& tempMemory)
{
    state.generatedMeshes[(i32)MeshType::Triangle] = generateMesh(MeshType::Triangle);
    state.generatedMeshes[(i32)MeshType::Quad] = generateMesh(MeshType::Quad);
    state.generatedMeshes[(i32)MeshType::Cube] = generateMesh(MeshType::Cube);
    state.generatedMeshes[(i32)MeshType::Sphere] = generateMesh(MeshType::Sphere);
    state.generatedMeshes[(i32)MeshType::Grid] = generateMesh(MeshType::Grid);

    state.loadedMeshes[(i32)AssetID::ArrowMesh] = loadMesh(assets[(i32)AssetID::ArrowMesh], permanentMemory, tempMemory);
}

void renderInit(RenderState& state, void* window)
{
    const auto& triangle = state.generatedMeshes[(i32)MeshType::Triangle];
    const auto& quad = state.generatedMeshes[(i32)MeshType::Quad];
    const auto& cube = state.generatedMeshes[(i32)MeshType::Cube];
    const auto& sphere = state.generatedMeshes[(i32)MeshType::Sphere];
    const auto& grid = state.generatedMeshes[(i32)MeshType::Grid];
    const auto& arrow = state.loadedMeshes[(i32)AssetID::ArrowMesh];

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

    createRenderTargetAndDepthStencil(state.screenSize);

    vertexBuffers[getMeshBufferIndex(triangle)] = createVertexBuffer(triangle.vertices, triangle.verticesCount);
    indexBuffers[getMeshBufferIndex(triangle)] = createIndexBuffer(triangle.indices, triangle.indicesCount);

    vertexBuffers[getMeshBufferIndex(quad)] = createVertexBuffer(quad.vertices, quad.verticesCount);
    indexBuffers[getMeshBufferIndex(quad)] = createIndexBuffer(quad.indices, quad.indicesCount);

    vertexBuffers[getMeshBufferIndex(cube)] = createVertexBuffer(cube.vertices, cube.verticesCount);
    indexBuffers[getMeshBufferIndex(cube)] = createIndexBuffer(cube.indices, cube.indicesCount);

    vertexBuffers[getMeshBufferIndex(sphere)] = createVertexBuffer(sphere.vertices, sphere.verticesCount);
    indexBuffers[getMeshBufferIndex(sphere)] = createIndexBuffer(sphere.indices, sphere.indicesCount);

    vertexBuffers[getMeshBufferIndex(grid)] = createVertexBuffer(grid.vertices, grid.verticesCount);
    indexBuffers[getMeshBufferIndex(grid)] = createIndexBuffer(grid.indices, grid.indicesCount);

    vertexBuffers[getMeshBufferIndex(arrow)] = createVertexBuffer(arrow.vertices, arrow.verticesCount);
    // indexBuffers[getMeshBufferIndex(arrow)] = createIndexBuffer(arrow.indices, arrow.indicesCount);

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
    if (dss)
        dss.Reset();
    if (dsView)
        dsView.Reset();
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
    ENSURE(command.mesh != nullptr);

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

    deviceContext->IASetVertexBuffers(0, 1, vertexBuffers[getMeshBufferIndex(*command.mesh)].GetAddressOf(), &stride, &offset);
    if (command.mesh->isIndexed)
    {
        deviceContext->IASetIndexBuffer(indexBuffers[getMeshBufferIndex(*command.mesh)].Get(), DXGI_FORMAT_R32_UINT, 0);
        deviceContext->DrawIndexed(command.mesh->indicesCount, 0, 0);
    }
    else
    {
        deviceContext->Draw(command.mesh->verticesCount, 0);
    }
}

void renderClearAndResize(RenderState& state, glm::vec4 color)
{
    if (state.needsToResize)
    {
        deviceContext->OMSetRenderTargets(0, 0, nullptr);
        rtView.Reset();
        dsView.Reset();
        dss.Reset();
        HR_ASSERT(swapChain->ResizeBuffers(0, (UINT)state.screenSize.x, (UINT)state.screenSize.y, DXGI_FORMAT_UNKNOWN, 0));
        createRenderTargetAndDepthStencil(state.screenSize);
        state.needsToResize = false;
    }

    deviceContext->ClearRenderTargetView(rtView.Get(), &color.x);
    deviceContext->ClearDepthStencilView(dsView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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
