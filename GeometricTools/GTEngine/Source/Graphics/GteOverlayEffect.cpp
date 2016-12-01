// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteOverlayEffect.h>
using namespace gte;


OverlayEffect::OverlayEffect(ProgramFactory& factory, int windowWidth,
    int windowHeight, int textureWidth, int textureHeight,
    SamplerState::Filter filter, SamplerState::Mode mode0,
    SamplerState::Mode mode1, bool useColorPShader)
    :
    mWindowWidth(static_cast<float>(windowWidth)),
    mWindowHeight(static_cast<float>(windowHeight))
{
    Initialize(windowWidth, windowHeight, textureWidth, textureHeight);

    int i = factory.GetAPI();
    std::string psSource =
        (useColorPShader ? *msPSColorSource[i] : *msPSGraySource[i]);

    mProgram = factory.CreateFromSources(*msVSSource[i], psSource, "");
    if (mProgram)
    {
        std::shared_ptr<SamplerState> sampler =
            std::make_shared<SamplerState>();
        sampler->filter = filter;
        sampler->mode[0] = mode0;
        sampler->mode[1] = mode1;
        mProgram->GetPShader()->Set("imageSampler", sampler);
        mEffect = std::make_shared<VisualEffect>(mProgram);
    }
}

OverlayEffect::OverlayEffect(ProgramFactory& factory, int windowWidth,
    int windowHeight, int textureWidth, int textureHeight,
    std::string const& psSource)
    :
    mWindowWidth(static_cast<float>(windowWidth)),
    mWindowHeight(static_cast<float>(windowHeight))
{
    int i = factory.GetAPI();
    Initialize(windowWidth, windowHeight, textureWidth, textureHeight);

    mProgram = factory.CreateFromSources(*msVSSource[i], psSource, "");
    if (mProgram)
    {
        mEffect = std::make_shared<VisualEffect>(mProgram);
    }
}

void OverlayEffect::SetTextureRectangle(std::array<int, 4> const& rectangle)
{
    mTextureRectangle = rectangle;
    UpdateVertexBuffer();
}

void OverlayEffect::SetRectangles(std::array<int, 4> const& overlayRectangle,
    std::array<int, 4> const& textureRectangle)
{
    mOverlayRectangle = overlayRectangle;
    mTextureRectangle = textureRectangle;
    UpdateVertexBuffer();
}

bool OverlayEffect::Contains(int x, int y) const
{
    return mOverlayRectangle[0] <= x
        && x < mOverlayRectangle[0] + mOverlayRectangle[2]
        && mOverlayRectangle[1] <= y
        && y < mOverlayRectangle[1] + mOverlayRectangle[3];
}

void OverlayEffect::SetTexture(std::shared_ptr<Texture2> const& texture)
{
    if (texture)
    {
        mEffect->GetPixelShader()->Set("imageTexture", texture);
    }
}

void OverlayEffect::SetTexture(std::string const& textureName,
    std::shared_ptr<Texture2> const& texture)
{
    if (texture)
    {
        mEffect->GetPixelShader()->Set(textureName, texture);
    }
}

void OverlayEffect::Initialize(int windowWidth, int windowHeight,
    int textureWidth, int textureHeight)
{
    if (windowWidth <= 0 || windowHeight <= 0
    ||  textureWidth <= 0 || textureHeight <= 0)
    {
        LogError("Invalid input rectangle.");

        // Use dummy sizes.
        windowWidth = 1;
        windowHeight = 1;
        textureWidth = 1;
        textureHeight = 1;
    }

    mInvTextureWidth = 1.0f/static_cast<float>(textureWidth);
    mInvTextureHeight = 1.0f/static_cast<float>(textureHeight);

    mOverlayRectangle[0] = 0;
    mOverlayRectangle[1] = 0;
    mOverlayRectangle[2] = windowWidth;
    mOverlayRectangle[3] = windowHeight;

    mTextureRectangle[0] = 0;
    mTextureRectangle[1] = 0;
    mTextureRectangle[2] = textureWidth;
    mTextureRectangle[3] = textureHeight;

    // Create the vertex buffer.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    mVBuffer = std::make_shared<VertexBuffer>(vformat, 4);
    mVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    UpdateVertexBuffer();

    // Create the index buffer.
    mIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2,
        sizeof(unsigned int));
    unsigned int* indices = mIBuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 2;  indices[2] = 3;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 1;
}

void OverlayEffect::UpdateVertexBuffer()
{
    // Convert to normalized coordinates.
    float invWindowWidth = 1.0f/mWindowWidth;
    float invWindowHeight = 1.0f/mWindowHeight;
    float px = static_cast<float>(mOverlayRectangle[0])*invWindowWidth;
    float py = static_cast<float>(mOverlayRectangle[1])*invWindowHeight;
    float pw = static_cast<float>(mOverlayRectangle[2])*invWindowWidth;
    float ph = static_cast<float>(mOverlayRectangle[3])*invWindowHeight;

    float tx = static_cast<float>(mTextureRectangle[0])*mInvTextureWidth;
    float ty = static_cast<float>(mTextureRectangle[1])*mInvTextureHeight;
    float tw = static_cast<float>(mTextureRectangle[2])*mInvTextureWidth;
    float th = static_cast<float>(mTextureRectangle[3])*mInvTextureHeight;

    Vertex* vertex = mVBuffer->Get<Vertex>();
    vertex[0].position = { px, py };
    vertex[0].tcoord = { tx, ty };
    vertex[1].position = { px + pw, py };
    vertex[1].tcoord = { tx + tw, ty };
    vertex[2].position = { px, py + ph };
    vertex[2].tcoord = { tx, ty + th };
    vertex[3].position = { px + pw, py + ph };
    vertex[3].tcoord = { tx + tw, ty + th };
}


// TODO:  Write these shaders.
std::string const OverlayEffect::msGLSLVSSource = "";
std::string const OverlayEffect::msGLSLPSColorSource = "";
std::string const OverlayEffect::msGLSLPSGraySource = "";

std::string const OverlayEffect::msHLSLVSSource =
"struct VS_INPUT\n"
"{\n"
"    float2 modelPosition : POSITION;\n"
"    float2 modelTCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float2 vertexTCoord : TEXCOORD0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain (VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"    output.clipPosition.x = 2.0f*input.modelPosition.x - 1.0f;\n"
"    output.clipPosition.y = -2.0f*input.modelPosition.y + 1.0f;\n"
"    output.clipPosition.z = 0.0f;\n"
"    output.clipPosition.w = 1.0f;\n"
"    output.vertexTCoord = input.modelTCoord;\n"
"    return output;\n"
"}\n";

std::string const OverlayEffect::msHLSLPSColorSource =
"Texture2D<float4> imageTexture;\n"
"SamplerState imageSampler;\n"
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
"    output.pixelColor0 = imageTexture.Sample(imageSampler, input.vertexTCoord);\n"
"    return output;\n"
"}\n";

std::string const OverlayEffect::msHLSLPSGraySource =
"Texture2D<float> imageTexture;\n"
"SamplerState imageSampler;\n"
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
"    float gray = imageTexture.Sample(imageSampler, input.vertexTCoord);\n"
"    output.pixelColor0 = float4(gray, gray, gray, 1.0f);\n"
"    return output;\n"
"}\n";

std::string const* OverlayEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

std::string const* OverlayEffect::msPSColorSource[] =
{
    &msGLSLPSColorSource,
    &msHLSLPSColorSource
};

std::string const* OverlayEffect::msPSGraySource[] =
{
    &msGLSLPSGraySource,
    &msHLSLPSGraySource
};
