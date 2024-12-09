// RenderUtils.h
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl/client.h>
#include <algorithm> // Add this header

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex {
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

class RenderUtils {
public:
    bool Initialize(HWND hwnd, int width, int height);
    void Cleanup();
    void Render();
    void Update(float deltaTime);
    void UpdateCamera(float forward, float right, float up, float pitchDelta, float yawDelta);

private:
    bool InitDevice(HWND hwnd, int width, int height);
    bool CreateGeometry();
    bool CreateShaders();
    UINT numIndices;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11DepthStencilView> depthStencilView;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11Buffer> constantBuffer;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;

    XMVECTOR cameraPosition;
    XMVECTOR cameraTarget;
    XMVECTOR cameraUp;
    float cameraPitch;
    float cameraYaw;
};