// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "Application.h"

Application* TheApplication = nullptr;


Application::~Application()
{
    if (mImmediate)
    {
        ID3D11RenderTargetView* rtViews[1] = { nullptr };
        mImmediate->OMSetRenderTargets(1, rtViews, nullptr);
    }

    if (mRasterizerState)
    {
        mRasterizerState->Release();
    }

    if (mDepthStencilState)
    {
        mDepthStencilState->Release();
    }

    if (mBlendState)
    {
        mBlendState->Release();
    }

    if (mSamplerState)
    {
        mSamplerState->Release();
    }

    if (mShaderResourceView)
    {
        mShaderResourceView->Release();
    }

    if (mTexture)
    {
        mTexture->Release();
    }

    if (mConstantBuffer)
    {
        mConstantBuffer->Release();
    }

    if (mInputLayout)
    {
        mInputLayout->Release();
    }

    if (mVertexBuffer)
    {
        mVertexBuffer->Release();
    }

    if (mPixelShader)
    {
        mPixelShader->Release();
    }

    if (mVertexShaderBlob)
    {
        mVertexShaderBlob->Release();
    }

    if (mVertexShader)
    {
        mVertexShader->Release();
    }

    if (mDepthStencilView)
    {
        mDepthStencilView->Release();
    }

    if (mDepthStencilBuffer)
    {
        mDepthStencilBuffer->Release();
    }

    if (mColorView)
    {
        mColorView->Release();
    }

    if (mColorBuffer)
    {
        mColorBuffer->Release();
    }

    if (mSwapChain)
    {
        mSwapChain->Release();
    }

    if (mImmediate)
    {
        mImmediate->Release();
    }

    if (mDevice)
    {
        mDevice->Release();
    }

    if (mAtom)
    {
        UnregisterClass(L"LowLevelD3D11Class", 0);
    }
}

Application::Application()
    :
    mAtom(0),
    mHandle(nullptr),
    mXOrigin(0),
    mYOrigin(0),
    mXSize(0),
    mYSize(0),
    mDevice(nullptr),
    mImmediate(nullptr),
    mFeatureLevel(D3D_FEATURE_LEVEL_9_1),
    mSwapChain(nullptr),
    mColorBuffer(nullptr),
    mColorView(nullptr),
    mDepthStencilBuffer(nullptr),
    mDepthStencilView(nullptr),
    mVertexShader(nullptr),
    mVertexShaderBlob(nullptr),
    mPixelShader(nullptr),
    mVertexBuffer(nullptr),
    mInputLayout(nullptr),
    mConstantBuffer(nullptr),
    mTexture(nullptr),
    mShaderResourceView(nullptr),
    mSamplerState(nullptr),
    mBlendState(nullptr),
    mDepthStencilState(nullptr),
    mRasterizerState(nullptr),
    mNumVertices(0),
    mVertexOffset(0),
    mXTrack0(0.0f),
    mYTrack0(0.0f),
    mXTrack1(0.0f),
    mYTrack1(0.0f),
    mTrackBallDown(false)
{
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.Width = 0.0f;
    mViewport.Height = 0.0f;
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 0.0f;

    mUpFOVDegrees = 60.0f;
    mAspectRatio = 1.0f;  // mXSize/mYSize but these are not known yet
    mDMin = 0.1f;
    mDMax = 100.0f;
    mEye[0] = 0.0f;
    mEye[1] = 0.0f;
    mEye[2] = 1.25f;
    mDVector[0] = 0.0f;
    mDVector[1] = 0.0f;
    mDVector[2] = -1.0f;
    mUVector[0] = 0.0f;
    mUVector[1] = 1.0f;
    mUVector[2] = 0.0f;
    mRVector[0] = 1.0f;
    mRVector[1] = 0.0f;
    mRVector[2] = 0.0f;

    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            if (r != c)
            {
                mTrackballMatrix[r][c] = 0.0f;
                mSaveTrackballMatrix[r][c] = 0.0f;
            }
            else
            {
                mTrackballMatrix[r][c] = 1.0f;
                mSaveTrackballMatrix[r][c] = 1.0f;
            }
        }
    }

    ComputeProjectionMatrix();
    ComputeViewMatrix();
    ComputeWorldMatrix();
}

LRESULT CALLBACK Application::WindowProcedure(HWND handle, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    if (handle == TheApplication->mHandle)
    {
        switch (message)
        {
        case WM_ERASEBKGND:
            return 1;
        case WM_KEYDOWN:
            if (VK_ESCAPE == (int)wParam)
            {
                PostQuitMessage(0);
            }
            return 0;
        case WM_LBUTTONDOWN:
        {
            int x = (int)(short)(lParam & 0xFFFF);
            int y = (int)(short)((lParam & 0xFFFF0000) >> 16);
            TheApplication->OnLeftMouseDown(x, y);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            int x = (int)(short)(lParam & 0xFFFF);
            int y = (int)(short)((lParam & 0xFFFF0000) >> 16);
            TheApplication->OnLeftMouseDrag(x, y);
            return 0;
        }
        case WM_LBUTTONUP:
        {
            int x = (int)(short)(lParam & 0xFFFF);
            int y = (int)(short)((lParam & 0xFFFF0000) >> 16);
            TheApplication->OnLeftMouseUp(x, y);
            return 0;
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

bool Application::Create(int xOrigin, int yOrigin, int xSize, int ySize,
    D3D_FEATURE_LEVEL featureLevel, UINT flags)
{
    return CreateAppWindow(xOrigin, yOrigin, xSize, ySize)
        && CreateGraphics(featureLevel, flags)
        && CreateShaders()
        && CreateVertexBuffer()
        && CreateInputLayout()
        && CreateConstantBuffer()
        && CreateTexture()
        && CreateShaderResourceView()
        && CreateBlendState()
        && CreateDepthStencilState()
        && CreateRasterizerState()
        && CreateSamplerState();
}

void Application::Run()
{
    // Set state once.  This application does not need it to change
    // dynamically.
    FLOAT const blendColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT const sampleMask = 0xFFFFFFFF;
    UINT const stencilRef = 0;
    mImmediate->OMSetBlendState(mBlendState, blendColor, sampleMask);
    mImmediate->OMSetDepthStencilState(mDepthStencilState, stencilRef);
    mImmediate->RSSetState(mRasterizerState);

    ShowWindow(mHandle, SW_SHOW);
    UpdateWindow(mHandle);
    for (;;)
    {
        MSG msg;
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            UpdateConstantBuffer();
            ClearBuffers();
            Draw();
            SwapBuffers(0);
        }
    }
}

bool Application::CreateAppWindow(int xOrigin, int yOrigin, int xSize,
    int ySize)
{
    mXOrigin = xOrigin;
    mYOrigin = yOrigin;
    mXSize = xSize;
    mYSize = ySize;

    WNDCLASS wc;
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProcedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = L"LowLevelD3D11Class";
    wc.lpszMenuName = nullptr;
    mAtom = RegisterClass(&wc);
    if (mAtom == 0)
    {
        return false;
    }

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = static_cast<LONG>(xSize - 1);
    rect.bottom = static_cast<LONG>(ySize - 1);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&rect, style, FALSE);
    int xAdjusted = (int)(rect.right - rect.left + 1);
    int yAdjusted = (int)(rect.bottom - rect.top + 1);
    mHandle = CreateWindow(L"LowLevelD3D11Class", L"LowLevelD3D11Window",
        style, xOrigin, yOrigin, xAdjusted, yAdjusted, nullptr,
        nullptr, nullptr, nullptr);
    if (!mHandle)
    {
        return false;
    }

    return true;
}

bool Application::CreateGraphics(D3D_FEATURE_LEVEL featureLevel, UINT flags)
{
    D3D_FEATURE_LEVEL featureLevels[1] = { featureLevel };
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, featureLevels, 1, D3D11_SDK_VERSION, &mDevice, &mFeatureLevel,
        &mImmediate);
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIDevice* dxgiDevice = nullptr;
    hr = mDevice->QueryInterface(__uuidof(IDXGIDevice),
        (void**)&dxgiDevice);
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr))
    {
        dxgiDevice->Release();
        return false;
    }

    IDXGIFactory1* dxgiFactory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1),
        (void**)&dxgiFactory);
    if (FAILED(hr))
    {
        dxgiDevice->Release();
        dxgiAdapter->Release();
        return false;
    }

    DXGI_SWAP_CHAIN_DESC scDesc;
    scDesc.BufferDesc.Width = mXSize;
    scDesc.BufferDesc.Height = mYSize;
    scDesc.BufferDesc.RefreshRate.Numerator = 0;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.BufferUsage =
        DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 2;
    scDesc.OutputWindow = mHandle;
    scDesc.Windowed = TRUE;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Flags = 0;

    hr = dxgiFactory->CreateSwapChain(dxgiDevice, &scDesc, &mSwapChain);
    if (FAILED(hr))
    {
        dxgiDevice->Release();
        dxgiAdapter->Release();
        dxgiFactory->Release();
        return false;
    }

    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();

    // Create the color buffer and its view.
    hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&mColorBuffer));
    if (FAILED(hr))
    {
        return false;
    }

    hr = mDevice->CreateRenderTargetView(mColorBuffer, nullptr, &mColorView);
    if (FAILED(hr))
    {
        return false;
    }

    // Create the depth-stencil buffer and its view.
    D3D11_TEXTURE2D_DESC txDesc;
    txDesc.Width = mXSize;
    txDesc.Height = mYSize;
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Usage = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;

    hr = mDevice->CreateTexture2D(&txDesc, nullptr, &mDepthStencilBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    hr = mDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr,
        &mDepthStencilView);
    if (FAILED(hr))
    {
        return false;
    }

    mViewport.Width = static_cast<float>(mXSize);
    mViewport.Height = static_cast<float>(mYSize);
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;
    mImmediate->RSSetViewports(1, &mViewport);

    ID3D11RenderTargetView* rtViews[1] = { mColorView };
    mImmediate->OMSetRenderTargets(1, rtViews, mDepthStencilView);
    return true;
}

bool Application::CreateShaders()
{
    std::wstring path = L"_Output\\v120\\";
#if defined(ARCH_WIN32)
    path += L"Win32\\";
#else
    path += L"x64\\";
#endif

#if defined(_DEBUG)
    path += L"Debug\\";
#else
    path += L"Release\\";
#endif

    ID3DBlob* vsBlob = LoadShaderBlob(path + L"SimpleVShader.cso");
    if (!vsBlob)
    {
        return false;
    }

    ID3DBlob* psBlob = LoadShaderBlob(path + L"SimplePShader.cso");
    if (!psBlob)
    {
        vsBlob->Release();
        return false;
    }

    HRESULT hr = mDevice->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), nullptr, &mVertexShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = mDevice->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, &mPixelShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    mVertexShaderBlob = vsBlob;
    psBlob->Release();
    return true;
}

ID3DBlob* Application::LoadShaderBlob(std::wstring const& filename)
{
    std::ifstream input(filename, std::ios::binary);
    if (!input)
    {
        return nullptr;
    }

    // Get the number of bytes in the file.
    std::streampos fpos = input.tellg();
    input.seekg(0, std::ios::end);
    fpos = input.tellg() - fpos;
    SIZE_T numBytes = static_cast<SIZE_T>(fpos);
    input.seekg(0, std::ios::beg);

    ID3DBlob* compiled = nullptr;
    HRESULT hr = D3DCreateBlob(numBytes, &compiled);
    if (FAILED(hr))
    {
        input.close();
        return nullptr;
    }

    if (input.read((char*)compiled->GetBufferPointer(),
        compiled->GetBufferSize()).bad())
    {
        compiled->Release();
        return nullptr;
    }

    return compiled;
}

bool Application::CreateVertexBuffer()
{
    mNumVertices = 3;
    mVertexOffset = 0;
    Vertex* vertex = new Vertex[mNumVertices];

    vertex[0].position[0] = 0.0f;
    vertex[0].position[1] = 0.0f;
    vertex[0].position[2] = 0.0f;
    vertex[0].color[0] = 1.0f;
    vertex[0].color[1] = 0.0f;
    vertex[0].color[2] = 0.0f;
    vertex[0].color[3] = 1.0f;
    vertex[0].tcoord[0] = 0.0f;
    vertex[0].tcoord[1] = 0.0f;

    vertex[1].position[0] = 1.0f;
    vertex[1].position[1] = 0.0f;
    vertex[1].position[2] = 0.0f;
    vertex[1].color[0] = 0.0f;
    vertex[1].color[1] = 1.0f;
    vertex[1].color[2] = 0.0f;
    vertex[1].color[3] = 1.0f;
    vertex[1].tcoord[0] = 1.0f;
    vertex[1].tcoord[1] = 0.0f;

    vertex[2].position[0] = 0.0f;
    vertex[2].position[1] = 1.0f;
    vertex[2].position[2] = 0.0f;
    vertex[2].color[0] = 0.0f;
    vertex[2].color[1] = 0.0f;
    vertex[2].color[2] = 1.0f;
    vertex[2].color[3] = 1.0f;
    vertex[2].tcoord[0] = 0.0f;
    vertex[2].tcoord[1] = 1.0f;

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth = mNumVertices*sizeof(Vertex);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = 0;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = vertex;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;
    HRESULT hr = mDevice->CreateBuffer(&vbDesc, &data, &mVertexBuffer);
    delete[] vertex;
    return SUCCEEDED(hr);
}

bool Application::CreateInputLayout()
{
    UINT const numElements = 3;
    D3D11_INPUT_ELEMENT_DESC ieDesc[numElements];
    ieDesc[0].SemanticName = "POSITION";
    ieDesc[0].SemanticIndex = 0;
    ieDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    ieDesc[0].InputSlot = 0;
    ieDesc[0].AlignedByteOffset = 0;  // Start of Vertex
    ieDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ieDesc[0].InstanceDataStepRate = 0;

    ieDesc[1].SemanticName = "COLOR";
    ieDesc[1].SemanticIndex = 0;
    ieDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ieDesc[1].InputSlot = 0;
    ieDesc[1].AlignedByteOffset = 12;  // Start of Vertex.color
    ieDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ieDesc[1].InstanceDataStepRate = 0;

    ieDesc[2].SemanticName = "TEXCOORD";
    ieDesc[2].SemanticIndex = 0;
    ieDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    ieDesc[2].InputSlot = 0;
    ieDesc[2].AlignedByteOffset = 28;  // Start of Vertex.tcoord
    ieDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ieDesc[2].InstanceDataStepRate = 0;

    HRESULT hr = mDevice->CreateInputLayout(ieDesc, numElements,
        mVertexShaderBlob->GetBufferPointer(),
        mVertexShaderBlob->GetBufferSize(), &mInputLayout);
    return SUCCEEDED(hr);
}

bool Application::CreateConstantBuffer()
{

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = 4 * 4 * sizeof(float);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = mDevice->CreateBuffer(&cbDesc, nullptr, &mConstantBuffer);
    return SUCCEEDED(hr);
}

bool Application::CreateTexture()
{
    UINT const width = 128;
    UINT const height = 128;
    D3D11_TEXTURE2D_DESC txDesc;
    txDesc.Width = width;
    txDesc.Height = height;
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_R8_UNORM;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    txDesc.MiscFlags = 0;
    txDesc.Usage = D3D11_USAGE_IMMUTABLE;
    txDesc.CPUAccessFlags = 0;

    unsigned char* texels = new unsigned char[width*height];
    for (UINT y = 0; y < height; ++y)
    {
        for (UINT x = 0; x < width; ++x)
        {
            texels[x + width*y] = static_cast<unsigned char>(x + y);
        }
    }

    // Create the texture.
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = texels;
    data.SysMemPitch = width;
    data.SysMemSlicePitch = 0;
    HRESULT hr = mDevice->CreateTexture2D(&txDesc, &data, &mTexture);
    delete[] texels;
    return SUCCEEDED(hr);
}

bool Application::CreateShaderResourceView()
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = DXGI_FORMAT_R8_UNORM;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = 1;

    HRESULT hr = mDevice->CreateShaderResourceView(mTexture, &srDesc,
        &mShaderResourceView);
    return SUCCEEDED(hr);
}

bool Application::CreateBlendState()
{
    D3D11_BLEND_DESC bsDesc;
    bsDesc.AlphaToCoverageEnable = FALSE;
    bsDesc.IndependentBlendEnable = FALSE;
    for (unsigned int i = 0; i < 8; ++i)
    {
        D3D11_RENDER_TARGET_BLEND_DESC& out = bsDesc.RenderTarget[i];
        out.BlendEnable = false;
        out.SrcBlend = D3D11_BLEND_ONE;
        out.DestBlend = D3D11_BLEND_ZERO;
        out.BlendOp = D3D11_BLEND_OP_ADD;
        out.SrcBlendAlpha = D3D11_BLEND_ONE;
        out.DestBlendAlpha = D3D11_BLEND_ZERO;
        out.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        out.RenderTargetWriteMask = 0xF;
    }

    HRESULT hr = mDevice->CreateBlendState(&bsDesc, &mBlendState);
    return SUCCEEDED(hr);
}

bool Application::CreateDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = FALSE;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HRESULT hr = mDevice->CreateDepthStencilState(&dsDesc,
        &mDepthStencilState);
    return SUCCEEDED(hr);
}

bool Application::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rsDesc;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = TRUE;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.SlopeScaledDepthBias = 0.0f;
    rsDesc.DepthClipEnable = TRUE;
    rsDesc.ScissorEnable = FALSE;
    rsDesc.MultisampleEnable = FALSE;
    rsDesc.AntialiasedLineEnable = FALSE;

    HRESULT hr = mDevice->CreateRasterizerState(&rsDesc, &mRasterizerState);
    return SUCCEEDED(hr);
}

bool Application::CreateSamplerState()
{
    D3D11_SAMPLER_DESC ssDesc;
    ssDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    ssDesc.MipLODBias = 0.0f;
    ssDesc.MaxAnisotropy = 1;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    ssDesc.BorderColor[0] = 1.0f;
    ssDesc.BorderColor[1] = 1.0f;
    ssDesc.BorderColor[2] = 1.0f;
    ssDesc.BorderColor[3] = 1.0f;
    ssDesc.MinLOD = -FLT_MAX;
    ssDesc.MaxLOD = FLT_MAX;

    HRESULT hr = mDevice->CreateSamplerState(&ssDesc, &mSamplerState);
    return SUCCEEDED(hr);
}

void Application::ClearBuffers()
{
    FLOAT const clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    mImmediate->ClearRenderTargetView(mColorView, clearColor);

    float const clearDepth = 1.0f;
    UINT8 const clearStencil = 0;
    mImmediate->ClearDepthStencilView(mDepthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

void Application::Draw()
{
    // Set the vertex shader.
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        mImmediate->VSSetShader(mVertexShader, instances, numInstances);

    }

    // Set the constant buffer inputs for the vertex shader.
    {
        // SimpleVShader.vs_5_0.txt shows a bind point of 0.
        ID3D11Buffer* buffers[1] = { mConstantBuffer };
        unsigned int bindPoint = 0;
        mImmediate->VSSetConstantBuffers(bindPoint, 1, buffers);
    }

    // Set the pixel shader.
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        mImmediate->PSSetShader(mPixelShader, instances, numInstances);
    }

    // Set the shader resource view for the texture of the pixel shader.
    {
        // SimplePShader.ps_5_0.txt shows a bind point of 0.
        ID3D11ShaderResourceView* views[1] = { mShaderResourceView };
        unsigned int bindPoint = 0;
        mImmediate->PSSetShaderResources(bindPoint, 1, views);
    }

    // Set the sampler state for the pixel shader.
    {
        // SimplePShader.ps_5_0.txt shows a bind point of 0.
        ID3D11SamplerState* samplers[1] = { mSamplerState };
        unsigned int bindPoint = 0;
        mImmediate->PSSetSamplers(bindPoint, 1, samplers);
    }

    // Set the vertex buffer.
    {
        ID3D11Buffer* buffers[1] = { mVertexBuffer };
        UINT strides[1] = { static_cast<UINT>(sizeof(Vertex)) };
        UINT offsets[1] = { (UINT)mVertexOffset };
        mImmediate->IASetVertexBuffers(0, 1, buffers, strides, offsets);
    }

    // Set the input layout for the vertex buffer and vertex shader.
    {
        mImmediate->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mImmediate->IASetInputLayout(mInputLayout);
    }

    // Draw the triangle.
    mImmediate->Draw(mNumVertices, mVertexOffset);

    // Clear the input layout for the vertex buffer and vertex shader.
    {
        mImmediate->IASetInputLayout(nullptr);
    }

    // Clear the vertex buffer.
    {
        ID3D11Buffer* buffers[1] = { nullptr };
        UINT strides[1] = { 0 };
        UINT offsets[1] = { 0 };
        mImmediate->IASetVertexBuffers(0, 1, buffers, strides, offsets);
    }

    // Clear the sampler state for the pixel shader.
    {
        // SimplePShader.ps_5_0.txt shows a bind point of 0.
        ID3D11SamplerState* samplers[1] = { nullptr };
        unsigned int bindPoint = 0;
        mImmediate->PSSetSamplers(bindPoint, 1, samplers);
    }

    // Clear the shader resource view for the texture of the pixel shader.
    {
        // SimplePShader.ps_5_0.txt shows a bind point of 0.
        ID3D11ShaderResourceView* views[1] = { nullptr };
        unsigned int bindPoint = 0;
        mImmediate->PSSetShaderResources(bindPoint, 1, views);
    }

    // Clear the pixel shader.
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        mImmediate->PSSetShader(nullptr, instances, numInstances);
    }

    // Clear the constant buffer inputs for the vertex shader.
    {
        // SimpleVShader.vs_5_0.txt shows a bind point of 0.
        ID3D11Buffer* buffers[1] = { nullptr };
        unsigned int bindPoint = 0;
        mImmediate->VSSetConstantBuffers(bindPoint, 1, buffers);
    }

    // Clear the vertex shader.
    {
        ID3D11ClassInstance* instances[1] = { nullptr };
        UINT numInstances = 0;
        mImmediate->VSSetShader(nullptr, instances, numInstances);
    }
}

void Application::SwapBuffers(unsigned int syncInterval)
{
    mSwapChain->Present(syncInterval, 0);
}

void Application::OnLeftMouseDown(int x, int y)
{
    // Get the start point of the drag.
    float mult = 1.0f / (mXSize >= mYSize ? mYSize : mXSize);
    memcpy(&mSaveTrackballMatrix[0][0], &mTrackballMatrix[0][0],
        16 * sizeof(float));
    mXTrack0 = (2 * x - mXSize)*mult;
    mYTrack0 = (2 * (mYSize - 1 - y) - mYSize)*mult;
    mTrackBallDown = true;
}

void Application::OnLeftMouseDrag(int x, int y)
{
    if (mTrackBallDown)
    {
        // Get the final point of the drag.
        float mult = 1.0f / (mXSize >= mYSize ? mYSize : mXSize);
        mXTrack1 = (2 * x - mXSize)*mult;
        mYTrack1 = (2 * (mYSize - 1 - y) - mYSize)*mult;

        // Update the object's local rotation.
        RotateTrackball(mXTrack0, mYTrack0, mXTrack1, mYTrack1);
    }
}

void Application::OnLeftMouseUp(int, int)
{
    mTrackBallDown = false;
}

void Application::RotateTrackball(float x0, float y0, float x1, float y1)
{
    if (x0 == x1 && y0 == y1)
    {
        // Nothing to rotate.
        return;
    }

    // Get the first vector on the sphere.
    float length = sqrt(x0*x0 + y0*y0), invLength, z0, z1;
    if (length > 1.0f)
    {
        // Outside the unit disk, project onto it.
        invLength = 1.0f / length;
        x0 *= invLength;
        y0 *= invLength;
        z0 = 0.0f;
    }
    else
    {
        // Compute point (x0,y0,z0) on negative unit hemisphere.
        z0 = 1.0f - x0*x0 - y0*y0;
        z0 = (z0 <= 0.0f ? 0.0f : sqrt(z0));
    }
    z0 *= -1.0f;

    // Use camera world coordinates, order is (D,U,R), so point is (z,y,x).
    float vec0[3] = { z0, y0, x0 };

    // Get the second vector on the sphere.
    length = sqrt(x1*x1 + y1*y1);
    if (length > 1.0f)
    {
        // Outside unit disk, project onto it.
        invLength = 1.0f / length;
        x1 *= invLength;
        y1 *= invLength;
        z1 = 0.0f;
    }
    else
    {
        // Compute point (x1,y1,z1) on negative unit hemisphere.
        z1 = 1.0f - x1*x1 - y1*y1;
        z1 = (z1 <= 0.0f ? 0.0f : sqrt(z1));
    }
    z1 *= -1.0f;

    // Use camera world coordinates, order is (D,U,R), so point is (z,y,x).
    float vec1[3] = { z1, y1, x1 };

    // Create axis and angle for the rotation.
    float axis[3] =
    {
        vec0[1] * vec1[2] - vec0[2] * vec1[1],
        vec0[2] * vec1[0] - vec0[0] * vec1[2],
        vec0[0] * vec1[1] - vec0[1] * vec1[0]
    };
    length = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);

    float dot = vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2];
    float angle;
    if (length > 0.0f)
    {
        invLength = 1.0f / length;
        axis[0] *= invLength;
        axis[1] *= invLength;
        axis[2] *= invLength;
        if (dot < -1.0f)
        {
            dot = -1.0f;
        }
        else if (dot > 1.0f)
        {
            dot = +1.0f;
        }
        angle = acos(dot);
    }
    else  // Vectors are parallel.
    {
        if (dot < 0.0f)
        {
            // Rotated pi radians.
            invLength = 1.0f / sqrt(x0*x0 + y0*y0);
            axis[0] = y0*invLength;
            axis[1] = -x0*invLength;
            axis[2] = 0.0f;
            angle = 4.0f*atan(1.0f);  // pi
        }
        else
        {
            // Rotation by zero radians.
            axis[0] = 1.0f;
            axis[1] = 0.0f;
            axis[2] = 0.0f;
            angle = 0.0f;
        }
    }

    // Compute the rotation matrix implied by trackball motion.  The axis
    // vector was computed in camera coordinates.  It must be converted
    // to world coordinates.  Once again, I use the camera ordering (D,U,R).
    float worldAxis[3];
    for (int i = 0; i < 3; ++i)
    {
        worldAxis[i] = axis[0] * mDVector[i] + axis[1] * mUVector[i] +
            axis[2] * mRVector[i];

    }

    float cs = cos(angle);
    float sn = sin(angle);
    float oneMinusCos = 1.0f - cs;
    float x0sqr = worldAxis[0] * worldAxis[0];
    float x1sqr = worldAxis[1] * worldAxis[1];
    float x2sqr = worldAxis[2] * worldAxis[2];
    float x0x1m = worldAxis[0] * worldAxis[1] * oneMinusCos;
    float x0x2m = worldAxis[0] * worldAxis[2] * oneMinusCos;
    float x1x2m = worldAxis[1] * worldAxis[2] * oneMinusCos;
    float x0Sin = worldAxis[0] * sn;
    float x1Sin = worldAxis[1] * sn;
    float x2Sin = worldAxis[2] * sn;
    float trackRotate[4][4];
    trackRotate[0][0] = x0sqr*oneMinusCos + cs;
    trackRotate[0][1] = x0x1m - x2Sin;
    trackRotate[0][2] = x0x2m + x1Sin;
    trackRotate[0][3] = 0.0;
    trackRotate[1][0] = x0x1m + x2Sin;
    trackRotate[1][1] = x1sqr*oneMinusCos + cs;
    trackRotate[1][2] = x1x2m - x0Sin;
    trackRotate[1][3] = 0.0;
    trackRotate[2][0] = x0x2m - x1Sin;
    trackRotate[2][1] = x1x2m + x0Sin;
    trackRotate[2][2] = x2sqr*oneMinusCos + cs;
    trackRotate[2][3] = 0.0;
    trackRotate[3][0] = 0.0;
    trackRotate[3][1] = 0.0;
    trackRotate[3][2] = 0.0;
    trackRotate[3][3] = 1.0;

    // Compute the new rotation, which is the incremental rotation of
    // the trackball appiled after the object has been rotated by its old
    // rotation.
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            mTrackballMatrix[r][c] = 0.0f;
            for (int m = 0; m < 4; ++m)
            {
                mTrackballMatrix[r][c] +=
                    trackRotate[r][m] * mSaveTrackballMatrix[m][c];
            }
        }
    }
}

void Application::ComputeProjectionMatrix()
{
    float const degToRad = 0.01745329252f;

    float halfAngleRadians = 0.5f*mUpFOVDegrees*degToRad;
    mUMax = mDMin*tan(halfAngleRadians);
    mRMax = mAspectRatio*mUMax;
    mUMin = -mUMax;
    mRMin = -mRMax;
    float invDDiff = 1.0f / (mDMax - mDMin);
    float invUDiff = 1.0f / (mUMax - mUMin);
    float invRDiff = 1.0f / (mRMax - mRMin);

    mProjectionMatrix[0][0] = 2.0f*mDMin*invRDiff;
    mProjectionMatrix[0][1] = 0.0f;
    mProjectionMatrix[0][2] = -(mRMin + mRMax)*invRDiff;
    mProjectionMatrix[0][3] = 0.0f;
    mProjectionMatrix[1][0] = 0.0f;
    mProjectionMatrix[1][1] = 2.0f*mDMin*invUDiff;
    mProjectionMatrix[1][2] = -(mUMin + mUMax)*invUDiff;
    mProjectionMatrix[1][3] = 0.0f;
    mProjectionMatrix[2][0] = 0.0f;
    mProjectionMatrix[2][1] = 0.0f;
    mProjectionMatrix[2][2] = mDMax*invDDiff;
    mProjectionMatrix[2][3] = -mDMin*mDMax*invDDiff;
    mProjectionMatrix[3][0] = 0.0f;
    mProjectionMatrix[3][1] = 0.0f;
    mProjectionMatrix[3][2] = 1.0f;
    mProjectionMatrix[3][3] = 0.0f;
}

void Application::ComputeViewMatrix()
{
    mViewMatrix[0][0] = mRVector[0];
    mViewMatrix[0][1] = mRVector[1];
    mViewMatrix[0][2] = mRVector[2];
    mViewMatrix[0][3] = -(mEye[0] * mRVector[0] + mEye[1] * mRVector[1] +
        mEye[2] * mRVector[2]);
    mViewMatrix[1][0] = mUVector[0];
    mViewMatrix[1][1] = mUVector[1];
    mViewMatrix[1][2] = mUVector[2];
    mViewMatrix[1][3] = -(mEye[0] * mUVector[0] + mEye[1] * mUVector[1] +
        mEye[2] * mUVector[2]);
    mViewMatrix[2][0] = mDVector[0];
    mViewMatrix[2][1] = mDVector[1];
    mViewMatrix[2][2] = mDVector[2];
    mViewMatrix[2][3] = -(mEye[0] * mDVector[0] + mEye[1] * mDVector[1] +
        mEye[2] * mDVector[2]);
    mViewMatrix[3][0] = 0.0f;
    mViewMatrix[3][1] = 0.0f;
    mViewMatrix[3][2] = 0.0f;
    mViewMatrix[3][3] = 1.0f;
}

void Application::ComputeWorldMatrix()
{
    // Translate the model-space triangle so that its center of mass is at
    // the origin of the trackball.
    mWorldMatrix[0][0] = 1.0f;
    mWorldMatrix[0][1] = 0.0f;
    mWorldMatrix[0][2] = 0.0f;
    mWorldMatrix[0][3] = -1.0f / 3.0f;
    mWorldMatrix[1][0] = 0.0f;
    mWorldMatrix[1][1] = 1.0f;
    mWorldMatrix[1][2] = 0.0f;
    mWorldMatrix[1][3] = -1.0f / 3.0f;
    mWorldMatrix[2][0] = 0.0f;
    mWorldMatrix[2][1] = 0.0f;
    mWorldMatrix[2][2] = 1.0f;
    mWorldMatrix[2][3] = 0.0f;
    mWorldMatrix[3][0] = 0.0f;
    mWorldMatrix[3][1] = 0.0f;
    mWorldMatrix[3][2] = 0.0f;
    mWorldMatrix[3][3] = 1.0f;
}

void Application::UpdateConstantBuffer()
{
    // The world matrix is the model-to-world transform of the triangle
    // followed by the trackball world-to-world transform.
    float worldMatrix[4][4];
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            worldMatrix[r][c] = 0.0f;
            for (int m = 0; m < 4; ++m)
            {
                worldMatrix[r][c] +=
                    mTrackballMatrix[r][m] * mWorldMatrix[m][c];
            }
        }
    }

    float pvMatrix[4][4];
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            pvMatrix[r][c] = 0.0f;
            for (int m = 0; m < 4; ++m)
            {
                pvMatrix[r][c] += mProjectionMatrix[r][m] * mViewMatrix[m][c];
            }
        }
    }

    float pvwMatrix[4][4];
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            pvwMatrix[r][c] = 0.0f;
            for (int m = 0; m < 4; ++m)
            {
                pvwMatrix[r][c] += pvMatrix[r][m] * worldMatrix[m][c];
            }
        }
    }

    // Map the buffer.
    D3D11_MAPPED_SUBRESOURCE sub;
    HRESULT hr = mImmediate->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD,
        0, &sub);
    if (SUCCEEDED(hr))
    {
        memcpy(sub.pData, &pvwMatrix[0][0], 16 * sizeof(float));
        mImmediate->Unmap(mConstantBuffer, 0);
    }
}

