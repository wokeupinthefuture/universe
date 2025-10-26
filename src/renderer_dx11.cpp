#include "geometry.hpp"
#include "renderer.hpp"
#include "context.hpp"

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <wrl/client.h>

using namespace Microsoft::WRL;

static ComPtr<ID3D11Device> s_device;
static ComPtr<IDXGISwapChain> s_swapChain;
static ComPtr<ID3D11DeviceContext> s_deviceContext;
static ComPtr<ID3D11RenderTargetView> s_rtView;
static ComPtr<ID3D11DepthStencilView> s_dsView;
static ID3D11DepthStencilState* s_dss;
static ID3D11DepthStencilState* s_dssNoWrite;

static HeapArray<ID3D11Buffer*> s_vertexBuffers;
static HeapArray<ID3D11Buffer*> s_indexBuffers;

static HeapArray<ID3D11ShaderResourceView*> s_textureViews;
static ID3D11SamplerState* s_textureSampler;
static ID3D11SamplerState* s_cubeMapTextureSampler;

size_t getMeshBufferIndex(Mesh const& mesh)
{
    return !size_t(mesh.flags & MeshFlag::Generated) * (size_t)GeneratedMesh::Max + mesh.id;
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
    HR_ASSERT(s_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs));
    HR_ASSERT(s_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps));
    shader.vs = vs;
    shader.ps = ps;

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    ID3D11InputLayout* layout = nullptr;
    HR_ASSERT(s_device->CreateInputLayout(
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

    HR_ASSERT(s_device->CreateBuffer(&bd, &sd, &buffer));

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

    HR_ASSERT(s_device->CreateBuffer(&bd, &sd, &buffer));

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

    HR_ASSERT(s_device->CreateBuffer(&cbDesc, nullptr, &buffer));

    return buffer;
}

static ID3D11RasterizerState* createRasterizerState(RasterizerState state)
{
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = state == RasterizerState::Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = state == RasterizerState::Wireframe ? D3D11_CULL_NONE : D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;

    ID3D11RasterizerState* rasterizerState = nullptr;
    HR_ASSERT(s_device->CreateRasterizerState(&rasterizerDesc, &rasterizerState));

    return rasterizerState;
}

static ID3D11RenderTargetView* createRenderTarget(vec2 size)
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = size.x;
    vp.Height = size.y;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    s_deviceContext->RSSetViewports(1, &vp);

    ComPtr<ID3D11Resource> backbuffer{};
    HR_ASSERT(s_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backbuffer));
    ID3D11RenderTargetView* view;
    HR_ASSERT(s_device->CreateRenderTargetView(backbuffer.Get(), nullptr, &view));
    return view;
}

static ID3D11DepthStencilView* createDepthStencilView(vec2 size)
{
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
    HR_ASSERT(s_device->CreateTexture2D(&textureDesc, nullptr, &texture));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = textureDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    ID3D11DepthStencilView* dsView;
    HR_ASSERT(s_device->CreateDepthStencilView(texture, &dsvDesc, &dsView));

    return dsView;
}

static ID3D11DepthStencilState* createDepthStencilState(bool noWrite)
{
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};

    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = noWrite ? D3D11_DEPTH_WRITE_MASK_ZERO : D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = noWrite ? D3D11_COMPARISON_LESS_EQUAL : D3D11_COMPARISON_LESS;

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

    ID3D11DepthStencilState* state;
    HR_ASSERT(s_device->CreateDepthStencilState(&dsDesc, &state));
    return state;
}

static ID3D11ShaderResourceView* createTexture(Texture texture)
{
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = texture.width;
    texDesc.Height = texture.height;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    ComPtr<ID3D11Texture2D> texture2d;
    HR_ASSERT(s_device->CreateTexture2D(&texDesc, nullptr, &texture2d));

    s_deviceContext->UpdateSubresource(
        texture2d.Get(), 0, nullptr, texture.data, texture.width * 4, texture.width * texture.height * 4);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = -1;

    ID3D11ShaderResourceView* srv = nullptr;
    HR_ASSERT(s_device->CreateShaderResourceView(texture2d.Get(), &srvDesc, &srv));
    s_deviceContext->GenerateMips(srv);

    return srv;
}

static ID3D11ShaderResourceView* createSkyboxTexture(Texture* textures)
{
    ENSURE(textures);

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

    auto& sample = textures[0];

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = sample.width;
    texDesc.Height = sample.height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    ComPtr<ID3D11Texture2D> texture;
    HR_ASSERT(s_device->CreateTexture2D(&texDesc, nullptr, &texture));

    s_deviceContext->UpdateSubresource(texture.Get(), 0, nullptr, imageData, width * 4, width * height * 4);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2D.MipLevels = -1;

    ID3D11ShaderResourceView* srv = nullptr;
    HR_ASSERT(s_device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv));
    s_deviceContext->GenerateMips(srv);

    return srv;
}

ID3D11SamplerState* createTextureSampler(bool isCubemap)
{
    D3D11_SAMPLER_DESC samplerDesc = {};

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    if (!isCubemap)
    {
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    }
    else
    {

        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    }

    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    samplerDesc.MaxAnisotropy = 1;

    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 1.0f;

    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    ID3D11SamplerState* sampler = nullptr;
    HR_ASSERT(s_device->CreateSamplerState(&samplerDesc, &sampler));
    return sampler;
}

void renderInitResources(RenderState& state, HeapArray<Asset>* assets)
{
    ENSURE(g_context);

    state.generatedMeshes[(i32)GeneratedMesh::Triangle] = generateMesh(GeneratedMesh::Triangle, g_context->tempMemory);
    state.generatedMeshes[(i32)GeneratedMesh::Quad] = generateMesh(GeneratedMesh::Quad, g_context->tempMemory);
    state.generatedMeshes[(i32)GeneratedMesh::Cube] = generateMesh(GeneratedMesh::Cube, g_context->tempMemory);
    state.generatedMeshes[(i32)GeneratedMesh::Sphere] = generateMesh(GeneratedMesh::Sphere, g_context->tempMemory);
    state.generatedMeshes[(i32)GeneratedMesh::Grid] = generateMesh(GeneratedMesh::Grid, g_context->tempMemory);

    const auto& meshes = assets[(i32)AssetType::ObjMesh];
    for (size_t i = 0; i < meshes.size; ++i)
    {
        Asset const& asset = meshes[i];
        auto mesh = loadMesh(asset, g_context->gameMemory, g_context->tempMemory);
        mesh.id = i;
        arrayPush(state.loadedMeshes, mesh);
    }

    const auto textures = assets[(i32)AssetType::Texture];
    for (size_t i = 0; i < textures.size; ++i)
    {
        const auto& asset = textures[i];
        arrayPush(state.loadedTextures,
            {.data = (u8*)asset.data,
                .size = asset.size,
                .name = asset.name,
                .width = asset.textureWidth,
                .height = asset.textureHeight,
                .channels = asset.textureChannels,
                .gpuTextureId = i,
                .isCubemap = false});
    }

    const auto skybox = assets[(i32)AssetType::SkyboxTexture];
    for (size_t i = 0; i < 6; ++i)
    {
        const auto& asset = skybox[i];
        arrayPush(state.loadedTextures,
            {.data = (u8*)asset.data,
                .size = asset.size,
                .name = asset.name,
                .width = asset.textureWidth,
                .height = asset.textureHeight,
                .channels = asset.textureChannels,
                .gpuTextureId = i,
                .isCubemap = false});
    }
}

void renderInit(RenderState& state, void* window)
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
        &s_swapChain,
        &s_device,
        nullptr,
        &s_deviceContext));

    s_rtView = createRenderTarget(state.screenSize);
    s_dsView = createDepthStencilView(state.screenSize);
    s_dss = createDepthStencilState(false);
    s_dssNoWrite = createDepthStencilState(true);
    s_deviceContext->OMSetRenderTargets(1, s_rtView.GetAddressOf(), s_dsView.Get());

    ENSURE(g_context);
    arrayInit(s_vertexBuffers, (size_t)GeneratedMesh::Max + state.loadedMeshes.size, g_context->gameMemory, "s_vertexBuffers");
    arrayInit(s_indexBuffers, (size_t)GeneratedMesh::Max, g_context->gameMemory, "s_indexBuffers");
    arrayInit(s_textureViews, state.loadedTextures.size, g_context->gameMemory, "s_textureViews");
    for (int i = 0; i < (i32)GeneratedMesh::Max; ++i)
    {
        arrayPush(s_vertexBuffers, createVertexBuffer(state.generatedMeshes[i].vertices, state.generatedMeshes[i].verticesCount));
        arrayPush(s_indexBuffers, createIndexBuffer(state.generatedMeshes[i].indices, state.generatedMeshes[i].indicesCount));
    }

    for (const auto& mesh : state.loadedMeshes)
    {
        arrayPush(s_vertexBuffers, createVertexBuffer(mesh.vertices, mesh.verticesCount));
    }

    for (auto& texture : state.loadedTextures)
    {
        arrayPush(s_textureViews, createTexture(texture));
    }

    s_textureSampler = createTextureSampler(false);
    s_cubeMapTextureSampler = createTextureSampler(true);

    for (i32 i = 0; i < (i32)ShaderType::Max; ++i)
    {
        shaders[i] = createShader(SHADER_PATH[i]);
    }

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
    if (s_deviceContext)
        s_deviceContext.Reset();
    if (s_rtView)
        s_rtView.Reset();
    if (s_dss)
        s_dss->Release();
    if (s_dssNoWrite)
        s_dssNoWrite->Release();
    if (s_dsView)
        s_dsView.Reset();
    if (s_swapChain)
        s_swapChain.Reset();
    if (s_device)
        s_device.Reset();

    for (auto& buffer : s_vertexBuffers)
        buffer->Release();
    for (auto& buffer : s_indexBuffers)
        buffer->Release();
    arrayClear(s_vertexBuffers, "s_vertexBuffers");
    arrayClear(s_indexBuffers, "s_indexBuffers");
    arrayClear(s_textureViews, "s_textureViews");

    constantBuffer.buffer.Reset();

    for (auto& shader : shaders)
    {
        shader.vs.Reset();
        shader.ps.Reset();
        shader.layout.Reset();
    }
    for (auto& state : rasterizerStates)
        state.Reset();

    if (s_deviceContext)
    {
        s_deviceContext->ClearState();
        s_deviceContext->Flush();
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
    HR_ASSERT(s_deviceContext->Map(buffer.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

    memcpy((uint8_t*)mappedResource.pData, data, dataSizeBytes);

    s_deviceContext->Unmap(buffer.buffer.Get(), 0);

    s_deviceContext->VSSetConstantBuffers(0, 1, buffer.buffer.GetAddressOf());
    s_deviceContext->PSSetConstantBuffers(0, 1, buffer.buffer.GetAddressOf());
}

void renderDraw(DrawCommand const& command)
{
    if (!bool(command.flags & DrawFlag::Active))
        return;

    ENSURE(command.mesh != nullptr);

    writeShaderVariables(command.shader, command.variables, MAX_SHADER_VARIABLES);

    s_deviceContext->IASetPrimitiveTopology(command.rasterizerState == RasterizerState::Wireframe
                                                ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST
                                                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (size_t i = 0; i < MAX_TEXTURE_SLOTS; ++i)
    {
        const auto& texture = command.textures[i];
        if (!texture)
            continue;

        if (texture->isCubemap)
            s_deviceContext->PSSetSamplers(0, 1, &s_cubeMapTextureSampler);
        else
            s_deviceContext->PSSetSamplers(0, 1, &s_textureSampler);

        s_deviceContext->PSSetShaderResources(i, 1, &s_textureViews[texture->gpuTextureId]);
    }

    auto& shader = shaders[(i32)command.shader];
    s_deviceContext->IASetInputLayout(shader.layout.Get());
    s_deviceContext->VSSetShader(shader.vs.Get(), nullptr, 0);
    s_deviceContext->PSSetShader(shader.ps.Get(), nullptr, 0);

    s_deviceContext->RSSetState(rasterizerStates[(i32)command.rasterizerState].Get());

    if (bool(command.flags & DrawFlag::DepthWrite))
    {
        s_deviceContext->OMSetDepthStencilState(s_dss, 0);
    }
    else
    {
        s_deviceContext->OMSetDepthStencilState(s_dssNoWrite, 0);
    }

    u32 stride = sizeof(Vertex), offset = 0;
    s_deviceContext->IASetVertexBuffers(0, 1, &s_vertexBuffers[getMeshBufferIndex(*command.mesh)], &stride, &offset);

    if (bool(command.mesh->flags & MeshFlag::Indexed))
    {
        s_deviceContext->IASetIndexBuffer(s_indexBuffers[getMeshBufferIndex(*command.mesh)], DXGI_FORMAT_R32_UINT, 0);
        s_deviceContext->DrawIndexed(command.mesh->indicesCount, 0, 0);
    }
    else
    {
        s_deviceContext->Draw(command.mesh->verticesCount, 0);
    }

    ID3D11ShaderResourceView* textureSlots[]{nullptr, nullptr, nullptr};
    s_deviceContext->PSSetShaderResources(0, 3, textureSlots);

    ID3D11SamplerState* nullSamplers[] = {nullptr, nullptr};
    s_deviceContext->PSSetSamplers(0, 2, nullSamplers);
}

void renderClearAndResize(RenderState& state, glm::vec4 color)
{
    if (state.needsToResize)
    {
        s_deviceContext->OMSetRenderTargets(0, 0, nullptr);
        s_rtView.Reset();
        s_dsView.Reset();
        s_dss->Release();
        s_dssNoWrite->Release();
        HR_ASSERT(s_swapChain->ResizeBuffers(0, (UINT)state.screenSize.x, (UINT)state.screenSize.y, DXGI_FORMAT_UNKNOWN, 0));
        s_rtView = createRenderTarget(state.screenSize);
        s_dsView = createDepthStencilView(state.screenSize);
        s_deviceContext->OMSetRenderTargets(1, s_rtView.GetAddressOf(), s_dsView.Get());
        state.needsToResize = false;
    }

    s_deviceContext->ClearRenderTargetView(s_rtView.Get(), &color.x);
    s_deviceContext->ClearDepthStencilView(s_dsView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void renderPresent()
{
    s_swapChain->Present(1, 0);
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
