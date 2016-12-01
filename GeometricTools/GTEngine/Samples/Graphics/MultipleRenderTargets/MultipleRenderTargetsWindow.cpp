// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "MultipleRenderTargetsWindow.h"


MultipleRenderTargetsWindow::MultipleRenderTargetsWindow(Parameters& parameters)
    :
    Window3(parameters),
    mActiveOverlay(0)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });
    InitializeCamera();
    CreateOverlays();
}

void MultipleRenderTargetsWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    // Render the square to offscreen memory.
    mEngine->Enable(mDrawTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);
    mEngine->Disable(mDrawTarget);

    // PSMain of MultipleRenderTarget.hlsl has written linearized depth to the
    // depth buffer.  It is not possible to attach a depth-stencil texture as
    // a shader input; you cannot create a shader resource view for it.  The
    // best you can do is read it back from the GPU and copy it to a texture
    // that is a shader input.  NOTE: If you really want to use depth as a
    // shader input, pass the 'perspectiveDepth' to the pixel shader as is
    // done in MultipleRenderTarget.hlsl and write it to a render target, not
    // to the depth-stencil texture.  You can then attach that render target
    // as a shader input.  This avoids the expensive read-back-and-copy step
    // here.
    std::shared_ptr<TextureDS> dsTexture = mDrawTarget->GetDSTexture();

    // Simple code for reading the depth texture from GPU to CPU, repackaging
    // it as a regular texture, and uploading from CPU to GPU is as follows.
    // On the AMD 7970, the initial display runs at 720 fps for a 512x512
    // application window.
    //
    // mEngine->CopyGpuToCpu(dsTexture);
    // memcpy(mLinearDepth->GetData(), dsTexture->GetData(),
    //     dsTexture->GetNumBytes());
    // mEngine->CopyCpuToGpu(mLinearDepth);
    //
    // This block of code does
    //   gpu -> srcStaging -> cpu(lineardepth) -> dstStaging -> gpu
    // The block of code below skips the cpu memory,
    //   gpu -> srcStaging -> dstStaging -> gpu
    //
    // We currently do not have wrappers CopyGpuToStaging, CopyStagingToGpu,
    // or CopyStagingToStaging.  CopyGpuToGpu can use CopySubresourceRegion
    // or CopyRegion as long as the resources are compatible.  But in the
    // situation here, the depth format DF_D24_UNORM_S8_UINT is not compatible
    // with DF_R32_UINT according to the error messages produced by the D3D11
    // debug layer when calling CopyResource, even though the textures are the
    // dimensions and have the same number of bytes.  In fact, CopyResource
    // does not have a return value that indicates the failure to copy, so
    // how does one trap the error?  On the AMD 7970, the initial display
    // runs at 840 fps for a 512x512 application window.
    ID3D11DeviceContext* context = mEngine->GetImmediate();
    DX11TextureDS* srcTexture = (DX11TextureDS*)mEngine->Bind(dsTexture);
    ID3D11Resource* srcResource = srcTexture->GetDXResource();
    ID3D11Resource* srcStaging = srcTexture->GetStagingResource();
    DX11Texture2* dstTexture = (DX11Texture2*)mEngine->Bind(mLinearDepth);
    ID3D11Resource* dstResource = dstTexture->GetDXResource();
    ID3D11Resource* dstStaging = dstTexture->GetStagingResource();
    context->CopySubresourceRegion(srcStaging, 0, 0, 0, 0, srcResource, 0,
        nullptr);
    D3D11_MAPPED_SUBRESOURCE srcSub, dstSub;
    HRESULT hr = context->Map(srcStaging, 0, D3D11_MAP_READ, 0, &srcSub);
    hr = context->Map(dstStaging, 0, D3D11_MAP_WRITE, 0, &dstSub);
    Memcpy(dstSub.pData, srcSub.pData, dsTexture->GetNumBytes());
    context->Unmap(srcStaging, 0);
    context->Unmap(dstStaging, 0);
    context->CopySubresourceRegion(dstResource, 0, 0, 0, 0, dstStaging, 0,
        nullptr);

    if (mActiveOverlay == 0)
    {
        // mSquare was rendered to the render target attached to mOverlay[0].
        // Draw the overlay, which contains the stone-textured 3D rendering.
        mEngine->Draw(mOverlay[0]);
    }
    else if (1 <= mActiveOverlay && mActiveOverlay <= 4)
    {
        mEngine->Draw(mOverlay[mActiveOverlay]);
    }
    else if (mActiveOverlay == 5)
    {
        // The output depth for the rendering of mSquare is set to linearized
        // depth, not the default perspective depth.  The depth texture of the
        // draw target is of the form 0xSSDDDDDD, which means the high-order
        // 8 bits are stencil values and the low-order 24 bits are depth values.
        // The depth texture was read from GPU to CPU as a 32-bit integer and
        // copied to mLinearDepth, a regular 2D texture with format R32_INT.
        // This texture is attached as an input to the gsOverlay1PShader pixel
        // shader and used as the pixel color output, which is the texture
        // attached to mOverlay[1].  The linearized depth as R32_INT has
        // values between 0 and 0xFFFFFF (16777215) but is normalized to
        // [0,1] on output.  The background is white because the depth buffer
        // was cleared to 1.0f, causing linearized depth to be 0xFFFFFF, and
        // normalized output to be 1.0f.  Rotate the square so it is not
        // parallel to the view direction and  move the camera backward
        // (press down arrow) so that the square is clipped by the far plane.
        // You will see the linearized depth become gray-to-white close to the
        // far plane, indicating the depth is varying from 0.0f (close to
        // near) to 1.0f (close to far).
        mEngine->Draw(mOverlay[5]);
    }
    else // mActiveOverlay == 6
    {
        // mOverlay[1] causes the gsOverlay1PShader shader to be executed,
        // which leads to writing the UAV colorTexture that is attached to
        // mOverlay[2].  We then draw that color texture using mOverlay[2].
        // This verifies that indeed the UAV can be written by a pixel
        // shader.
        mEngine->Draw(mOverlay[5]);
        mEngine->Draw(mOverlay[6]);
    }

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MultipleRenderTargetsWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        // Display mSquare with the stone texture.
        mActiveOverlay = 0;
        return true;
    case '1':
        // Display miplevel 1 of the color output of rendering the square.
        mActiveOverlay = 1;
        return true;
    case '2':
        // Display miplevel 2 of the color output of rendering the square.
        mActiveOverlay = 2;
        return true;
    case '3':
        // Display miplevel 3 of the color output of rendering the square.
        mActiveOverlay = 3;
        return true;
    case '4':
        // Display miplevel 4 of the color output of rendering the square.
        mActiveOverlay = 4;
        return true;
    case '5':
        // Display mSquare with the linearized depth, shown as a monochrome
        // image (depth stored in r, g, and b).
        mActiveOverlay = 5;
        return true;
    case '6':
        // Display the colorTexture UAV that is written in the pixel shader
        // that is defined at the beginning of this file.
        mActiveOverlay = 6;
        return true;
    }
    return Window::OnCharPress(key, x, y);
}

bool MultipleRenderTargetsWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(
        path + "/Samples/Graphics/MultipleRenderTargets/Shaders/");

    if (mEnvironment.GetPath("MultipleRenderTargets.hlsl") == "")
    {
        LogError("Cannot find file MultipleRenderTargets.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("StoneWall.png") == "")
    {
        LogError("Cannot find file StoneWall.png.");
        return false;
    }

    return true;
}

bool MultipleRenderTargetsWindow::CreateScene()
{
    // Create a visual effect that populates the draw target.
    std::string filename = mEnvironment.GetPath("MultipleRenderTargets.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(filename, filename, "");
    if (!program)
    {
        return false;
    }

    std::shared_ptr<ConstantBuffer> cbuffer(new ConstantBuffer(
        sizeof(Matrix4x4<float>), true));
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    std::shared_ptr<PixelShader> pshader = program->GetPShader();
    std::shared_ptr<ConstantBuffer> farNearRatio(new ConstantBuffer(
        sizeof(float), false));
    pshader->Set("FarNearRatio", farNearRatio);
    farNearRatio->SetMember("farNearRatio",
        mCamera->GetDMax() / mCamera->GetDMin());

    std::string path = mEnvironment.GetPath("StoneWall.png");
    std::shared_ptr<Texture2> baseTexture(WICFileIO::Load(path, true));
    baseTexture->AutogenerateMipmaps();
    pshader->Set("baseTexture", baseTexture);

    std::shared_ptr<SamplerState> baseSampler(new SamplerState());
    baseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    baseSampler->mode[0] = SamplerState::CLAMP;
    baseSampler->mode[1] = SamplerState::CLAMP;
    pshader->Set("baseSampler", baseSampler);

    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    // Create a vertex buffer for a two-triangle square.  The PNG is stored
    // in left-handed coordinates.  The texture coordinates are chosen to
    // reflect the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat, 4));
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = { -1.0f, -1.0f, 0.0f };
    vertex[0].tcoord = { 0.0f, 1.0f };
    vertex[1].position = { 1.0f, -1.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 1.0f };
    vertex[2].position = { -1.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 0.0f, 0.0f };
    vertex[3].position = { 1.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 1.0f, 0.0f };

    // Create an indexless buffer for a triangle mesh with two triangles.
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRISTRIP, 2));

    // Create the geometric object for drawing and enable automatic updates
    // of pvw-matrices and w-matrices.
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mTrackball.Attach(mSquare);
    mTrackball.Update();
    mCameraRig.Subscribe(mSquare->worldTransform, cbuffer);
    return true;
}

void MultipleRenderTargetsWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 10.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, 4.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, -1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.001f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

void MultipleRenderTargetsWindow::CreateOverlays()
{
    int api = mProgramFactory.GetAPI();

    // Create the draw target with 2 render targets and 1 depth-stencil
    // texture.  Each of these is used as a texture for an overlay.
    mDrawTarget = std::make_shared<DrawTarget>(2, DF_R32G32B32A32_FLOAT,
        mXSize, mYSize, true, true, DF_D24_UNORM_S8_UINT, true);
    mDrawTarget->AutogenerateRTMipmaps();
    mDrawTarget->GetRTTexture(0)->SetUsage(Resource::SHADER_OUTPUT);
    mDrawTarget->GetDSTexture()->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    // Display mSquare that was rendered to a draw target with mipmaps
    // enabled.  The depth texture output is linearized depth, not perspective
    // depth.  The mipmap selection is the standard algorithm used in the
    // HLSL Texture2D.Sample function.
    mOverlay[0] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_L_MAG_L_MIP_L,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay[0]->SetTexture(mDrawTarget->GetRTTexture(0));

    // Display mSquare using only miplevel i and using nearest-neighbor
    // sampling.
    std::shared_ptr<PixelShader> pshader;
    std::shared_ptr<SamplerState> nearestSampler(new SamplerState());
    nearestSampler->filter = SamplerState::MIN_P_MAG_P_MIP_P;
    nearestSampler->mode[0] = SamplerState::CLAMP;
    nearestSampler->mode[1] = SamplerState::CLAMP;
    for (int i = 1; i < 5; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
            mYSize, mXSize, mYSize, *msOverlayPSSource[api][i]);
        pshader = mOverlay[i]->GetProgram()->GetPShader();
        pshader->Set("inTexture", mDrawTarget->GetRTTexture(0));
        pshader->Set("nSampler", nearestSampler);
    }

    // Display mSquare using linearized depth.
    mLinearDepth = std::make_shared<Texture2>(DF_R32_UINT, mXSize, mYSize);
    mLinearDepth->SetUsage(Resource::SHADER_OUTPUT);
    mLinearDepth->SetCopyType(Resource::COPY_CPU_TO_STAGING);
    mOverlay[5] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, *msOverlayPSSource[api][0]);
    std::shared_ptr<SamplerState> linearSampler(new SamplerState());
    linearSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    linearSampler->mode[0] = SamplerState::CLAMP;
    linearSampler->mode[1] = SamplerState::CLAMP;
    pshader = mOverlay[5]->GetProgram()->GetPShader();
    pshader->Set("depthTexture", mLinearDepth);
    pshader->Set("positionTexture", mDrawTarget->GetRTTexture(1));
    pshader->Set("colorTexture", mDrawTarget->GetRTTexture(0));
    pshader->Set("nSampler", linearSampler);

    // Display the UAV color texture that is written by mOverlay[5].
    mOverlay[6] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_L_MAG_L_MIP_L,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay[6]->SetTexture(mDrawTarget->GetRTTexture(0));
}


// TODO:  Write these shaders.
std::string const MultipleRenderTargetsWindow::msGLSLOverlayPSSource[5] =
{
    "", "", "", "", ""
};

std::string const MultipleRenderTargetsWindow::msHLSLOverlayPSSource[5] =
{
    "Texture2D<uint> depthTexture;\n"
    "Texture2D<float4> positionTexture;\n"
    "RWTexture2D<float4> colorTexture;\n"
    "SamplerState nSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 pixelColor0 : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    float4 pos = positionTexture.Sample(nSampler, input.vertexTCoord);\n"
    "    uint depthR24S8 = depthTexture[(int2)pos.xy];\n"
    "    float gray = (depthR24S8 & 0x00FFFFFF) / 16777215.0f;\n"
    "    output.pixelColor0 = float4(gray, gray, gray, 1.0f);\n"
    "    colorTexture[(int2)pos.xy] = float4(0.4f, 0.5f, 0.6f, 1.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState nSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(nSampler, input.vertexTCoord, 1.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState nSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(nSampler, input.vertexTCoord, 2.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState nSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(nSampler, input.vertexTCoord, 3.0f);\n"
    "    return output;\n"
    "}\n",

    "Texture2D<float4> inTexture;\n"
    "SamplerState nSampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float2 vertexTCoord : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_OUTPUT\n"
    "{\n"
    "    float4 color : SV_TARGET0;\n"
    "};\n"
    "\n"
    "PS_OUTPUT PSMain(PS_INPUT input)\n"
    "{\n"
    "    PS_OUTPUT output;\n"
    "    output.color = inTexture.SampleLevel(nSampler, input.vertexTCoord, 4.0f);\n"
    "    return output;\n"
    "}\n"
};

std::string const*
MultipleRenderTargetsWindow::msOverlayPSSource[ProgramFactory::PF_NUM_API][5] =
{
    {
        &msGLSLOverlayPSSource[0],
        &msGLSLOverlayPSSource[1],
        &msGLSLOverlayPSSource[2],
        &msGLSLOverlayPSSource[3],
        &msGLSLOverlayPSSource[4]
    },
    {
        &msHLSLOverlayPSSource[0],
        &msHLSLOverlayPSSource[1],
        &msHLSLOverlayPSSource[2],
        &msHLSLOverlayPSSource[3],
        &msHLSLOverlayPSSource[4]
    }
};
