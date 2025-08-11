#include "app/platform.hpp"
#include "renderer.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>

#include "game/shaders/dx11/basic.hpp"
#include "lib/log.hpp"

namespace Renderer
{

ID3D11Device* device;
IDXGISwapChain* swapChain;
ID3D11DeviceContext* context;
ID3D11RenderTargetView* rtView;

ID3D11Buffer* vertexBuffers[Mesh::Max] = {nullptr, nullptr};

struct Shader
{
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* layout;
};

Shader shaders[2] = {};

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
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA sd = {};

    bd.StructureByteStride = sizeof(Vertex);
    bd.ByteWidth = sizeof(Vertex) * vertexCount;
    sd.pSysMem = vertices;

    HR_ASSERT(device->CreateBuffer(&bd, &sd, &buffer));

    return buffer;
}

void init(Platform::Window window, float windowWidth, float windowHeight)
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
        &context));

    ID3D11Resource* backbuffer{};
    HR_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backbuffer));
    HR_ASSERT(device->CreateRenderTargetView(backbuffer, nullptr, &rtView));
    backbuffer->Release();

    context->OMSetRenderTargets(1, &rtView, nullptr);

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = windowWidth;
    vp.Height = windowHeight;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    context->RSSetViewports(1, &vp);

    vertexBuffers[Mesh::Triangle] = createVertexBuffer(TRIANGLE_VERTICES, ARR_LENGTH(TRIANGLE_VERTICES));
    vertexBuffers[Mesh::Quad] = createVertexBuffer(QUAD_VERTICES, ARR_LENGTH(QUAD_VERTICES));

    const u32 strides[] = {
        (u32)sizeof(Vertex),
        (u32)sizeof(Vertex),
    };
    const u32 offsets[] = {0, 0};
    context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);

    shaders[0] = createShader(Shaders::Basic::vs, Shaders::Basic::fs);
}

void deinit()
{
    if (device)
        device->Release();
    if (swapChain)
        swapChain->Release();
    if (context)
        context->Release();
    if (rtView)
        rtView->Release();

    arrayFree(drawCommands);
}

void addDrawCommand(DrawCommand command)
{
    arrayPush(drawCommands, command);
}

void draw(DrawCommand const& command)
{
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    switch (command.shader)
    {
        case ShaderType::Basic:
            context->IASetInputLayout(shaders[0].layout);
            context->VSSetShader(shaders[0].vs, nullptr, 0);
            context->PSSetShader(shaders[0].ps, nullptr, 0);
            break;

        default: LOGIC_ERROR();
    }

    switch (command.mesh.type)
    {
        case Mesh::Triangle: context->Draw(ARR_LENGTH(TRIANGLE_VERTICES), 0); break;
        case Mesh::Quad: context->Draw(ARR_LENGTH(QUAD_VERTICES), 0); break;

        default: LOGIC_ERROR();
    }
}

void clear(glm::vec4 color)
{
    context->ClearRenderTargetView(rtView, &color.x);
}

void present()
{
    swapChain->Present(1, 0);
}

}  // namespace Renderer
