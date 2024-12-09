// RenderUtils.cpp
#include "RenderUtils.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct ConstantBuffer {
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
};

const char* vertexShaderSource = R"(
    cbuffer ConstantBuffer : register(b0) {
        matrix World;
        matrix View;
        matrix Projection;
    };

    struct VS_INPUT {
        float3 Pos : POSITION;
        float4 Color : COLOR;
    };

    struct VS_OUTPUT {
        float4 Pos : SV_POSITION;
        float4 Color : COLOR;
    };

    VS_OUTPUT main(VS_INPUT input) {
        VS_OUTPUT output;
        output.Pos = float4(input.Pos, 1.0f);
        output.Pos = mul(output.Pos, World);
        output.Pos = mul(output.Pos, View);
        output.Pos = mul(output.Pos, Projection);
        output.Color = input.Color;
        return output;
    }
)";

const char* pixelShaderSource = R"(
    struct PS_INPUT {
        float4 Pos : SV_POSITION;
        float4 Color : COLOR;
    };

    float4 main(PS_INPUT input) : SV_Target {
        return input.Color;
    }
)";



bool RenderUtils::InitDevice(HWND hwnd, int width, int height) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        featureLevels, 1, D3D11_SDK_VERSION,
        &scd, &swapChain, &device, nullptr, &context);

    if (FAILED(hr)) return false;

    // Create render target view
    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr)) return false;

    // Create depth stencil
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthStencil;
    hr = device->CreateTexture2D(&depthDesc, nullptr, &depthStencil);
    if (FAILED(hr)) return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    hr = device->CreateDepthStencilView(depthStencil.Get(), &dsvDesc, &depthStencilView);
    if (FAILED(hr)) return false;

    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    // Set viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    return true;
}

bool RenderUtils::CreateGeometry() {
    // Create 5 small black cubes as points in different 3D positions
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    // Function to add a cube at a specific position
    auto addCube = [&vertices, &indices](float x, float y, float z, float size = 0.1f) {
        uint32_t baseIndex = static_cast<uint32_t>(vertices.size());
        XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black color

        // Add cube vertices
        vertices.push_back({ {x - size, y - size, z - size}, color });
        vertices.push_back({ {x - size, y + size, z - size}, color });
        vertices.push_back({ {x + size, y + size, z - size}, color });
        vertices.push_back({ {x + size, y - size, z - size}, color });
        vertices.push_back({ {x - size, y - size, z + size}, color });
        vertices.push_back({ {x - size, y + size, z + size}, color });
        vertices.push_back({ {x + size, y + size, z + size}, color });
        vertices.push_back({ {x + size, y - size, z + size}, color });

        // Add cube indices
        uint32_t cubeIndices[] = {
            0, 1, 2,    0, 2, 3,  // Front
            4, 5, 6,    4, 6, 7,  // Back
            0, 1, 5,    0, 5, 4,  // Left
            2, 3, 7,    2, 7, 6,  // Right
            1, 2, 6,    1, 6, 5,  // Top
            0, 3, 7,    0, 7, 4   // Bottom
        };

        for (uint32_t i = 0; i < 36; i++) {
            indices.push_back(baseIndex + cubeIndices[i]);
        }
        };

    // Add 5 cubes in different positions
    addCube(0.0f, 0.0f, 2.0f);       // Center point, closer to camera
    addCube(1.0f, 1.0f, 3.0f);       // Right and up
    addCube(-1.0f, -1.0f, 4.0f);     // Left, down, and further
    addCube(1.0f, -1.0f, 2.5f);      // Right and down
    addCube(-1.0f, 1.0f, 3.5f);      // Left and up

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) return false;

    bd.ByteWidth = sizeof(UINT) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices.data();

    hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr)) return false;

    // Create constant buffer
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = device->CreateBuffer(&bd, nullptr, &constantBuffer);
    if (FAILED(hr)) return false;

    numIndices = static_cast<UINT>(indices.size()); // Store the number of indices
    return true;
}

void RenderUtils::Render() {
    // Clear with white background
    float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);

    ConstantBuffer cb;
    cb.mWorld = XMMatrixIdentity();
    cb.mView = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp);
    cb.mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 800.0f / 600.0f, 0.01f, 100.0f);

    context->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    // Draw all cubes
    context->DrawIndexed(numIndices, 0, 0);

    swapChain->Present(1, 0);
}

bool RenderUtils::Initialize(HWND hwnd, int width, int height) {
    if (!InitDevice(hwnd, width, height)) return false;
    if (!CreateGeometry()) return false;
    if (!CreateShaders()) return false;

    // Initialize camera position for better initial view
    cameraPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  // Start at origin
    cameraTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);    // Look forward
    cameraUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    cameraPitch = 0.0f;
    cameraYaw = 0.0f;

    return true;
}
bool RenderUtils::CreateShaders() {
    // Compile and create vertex shader
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr,
        "main", "vs_4_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(hr)) return false;

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    hr = device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr)) return false;

    // Compile and create pixel shader
    ComPtr<ID3DBlob> psBlob;
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), nullptr, nullptr, nullptr,
        "main", "ps_4_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
    if (FAILED(hr)) return false;

    return true;
}

// Add this helper function at the top of RenderUtils.cpp, before the class methods
namespace {
    float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
}

void RenderUtils::UpdateCamera(float forward, float right, float up, float pitchDelta, float yawDelta) {
    // Update rotation with higher sensitivity
    cameraPitch += pitchDelta * 2.0f;  // Increased sensitivity
    cameraYaw += yawDelta * 2.0f;      // Increased sensitivity

    // Clamp pitch to prevent camera flipping
    const float maxPitch = XM_PIDIV2 - 0.1f;
    const float minPitch = -XM_PIDIV2 + 0.1f;
    cameraPitch = Clamp(cameraPitch, minPitch, maxPitch);

    // Calculate forward vector
    XMVECTOR forwardVector = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        XMMatrixRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f));
    XMVECTOR rightVector = XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), forwardVector);

    // Update position with adjusted speed
    float moveSpeed = 0.05f;  // Reduced speed for finer control
    cameraPosition += forwardVector * forward * moveSpeed;
    cameraPosition += rightVector * right * moveSpeed;
    cameraPosition += XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) * up * moveSpeed;

    // Update target
    cameraTarget = XMVectorAdd(cameraPosition, forwardVector);
}



void RenderUtils::Update(float deltaTime) {
    // This method can be used for any per-frame updates
    // Currently empty as our scene is static
}

void RenderUtils::Cleanup() {
    if (context) context->ClearState();
}