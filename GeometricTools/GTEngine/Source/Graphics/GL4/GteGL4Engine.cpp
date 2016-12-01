// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GteFontArialW400H18.h>
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGL4BlendState.h>
#include <Graphics/GL4/GteGL4DepthStencilState.h>
#include <Graphics/GL4/GteGL4IndexBuffer.h>
//#include <Graphics/GL4/GteGL4DrawTarget.h>
#include <Graphics/GL4/GteGL4RasterizerState.h>
#include <Graphics/GL4/GteGL4VertexBuffer.h>
#include <Graphics/GL4/GteGL4ConstantBuffer.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <Graphics/GL4/GteGLSLComputeProgram.h>
#include <Graphics/GL4/GteGLSLVisualProgram.h>
using namespace gte;


GL4Engine::~GL4Engine()
{
    // The render state objects (and fonts) are destroyed first so that the
    // render state objects are removed from the bridges before they are
    // cleared later in the destructor.
    DestroyDefaultObjects();

    GraphicsObject::UnsubscribeForDestruction(mGOListener);
    delete mGOListener;

    DrawTarget::UnsubscribeForDestruction(mDTListener);
    delete mDTListener;

    if (mGOMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Bridge map is nonempty on destruction.");
        }

        std::vector<GL4GraphicsObject*> objects;
        mGOMap.GatherAll(objects);
        for (auto object : objects)
        {
            delete object;
        }
        mGOMap.RemoveAll();
    }

#if 0
    if (mDTMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Draw target map nonempty on destruction.");
        }

        std::vector<GL4DrawTarget*> targets;
        mDTMap.GatherAll(targets);
        for (auto target : targets)
        {
            delete target;
        }
        mDTMap.RemoveAll();
    }
#endif

    if (mILMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Input layout map nonempty on destruction.");
        }

        mILMap.UnbindAll();
    }

    if (mHandle && mDevice && mImmediate)
    {
        wglDeleteContext(mImmediate);
        ReleaseDC(mHandle, mDevice);
    }
}

GL4Engine::GL4Engine(HWND handle, bool pixelFormatAlreadyKnown)
    :
    mHandle(handle),
    mImmediate(nullptr)
{
    Initialize();

    if (!mHandle)
    {
        LogError("Invalid window handle.");
        return;
    }

    mDevice = GetDC(handle);
    if (!mDevice)
    {
        LogError("Invalid device context.");
        mHandle = nullptr;
        return;
    }

    RECT rect;
    GetClientRect(mHandle, &rect);
    mXSize = static_cast<unsigned int>(rect.right - rect.left);
    mYSize = static_cast<unsigned int>(rect.bottom - rect.top);

    if (!pixelFormatAlreadyKnown)
    {
        // Select the format for the drawing surface.
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags =
            PFD_DRAW_TO_WINDOW |
            PFD_SUPPORT_OPENGL |
            PFD_GENERIC_ACCELERATED |
            PFD_DOUBLEBUFFER;

        // Create an R8G8B8A8 buffer.
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        // Create a D24S8 depth-stencil buffer.
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        // Set the pixel format for the rendering context.
        int pixelFormat = ChoosePixelFormat(mDevice, &pfd);
        if (pixelFormat == 0)
        {
            LogError("ChoosePixelFormat failed.");
            ReleaseDC(mHandle, mDevice);
            mXSize = 0;
            mYSize = 0;
            mDevice = nullptr;
            mHandle = nullptr;
            return;
        }

        if (!SetPixelFormat(mDevice, pixelFormat, &pfd))
        {
            LogError("SetPixelFormat failed.");
            ReleaseDC(mHandle, mDevice);
            mXSize = 0;
            mYSize = 0;
            mDevice = nullptr;
            mHandle = nullptr;
            return;
        }
    }

    // Create an OpenGL context.
    mImmediate = wglCreateContext(mDevice);
    if (!mImmediate)
    {
        LogError("wglCreateContext failed.");
        ReleaseDC(mHandle, mDevice);
        mXSize = 0;
        mYSize = 0;
        mDevice = nullptr;
        mHandle = nullptr;
        return;
    }

    // Activate the context.
    if (!wglMakeCurrent(mDevice, mImmediate))
    {
        LogError("wglMakeCurrent failed.");
        wglDeleteContext(mImmediate);
        ReleaseDC(mHandle, mDevice);
        mXSize = 0;
        mYSize = 0;
        mImmediate = nullptr;
        mDevice = nullptr;
        mHandle = nullptr;
        return;
    }

    InitializeOpenGL("OpenGLDriverInfo.txt");
    InitializeWGL();

    SetViewport(0, 0, mXSize, mYSize);
    SetDepthRange(0.0f, 1.0f);
    CreateDefaultObjects();
}

bool GL4Engine::IsActive() const
{
    return mImmediate == wglGetCurrentContext();
}

void GL4Engine::MakeActive()
{
    if (mImmediate != wglGetCurrentContext())
    {
        wglMakeCurrent(mDevice, mImmediate);
    }
}

void GL4Engine::SetViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
}

void GL4Engine::GetViewport(int& x, int& y, int& w, int& h) const
{
    int param[4];
    glGetIntegerv(GL_VIEWPORT, param);
    x = param[0];
    y = param[1];
    w = param[2];
    h = param[3];
}

void GL4Engine::SetDepthRange(float zmin, float zmax)
{
    glDepthRange((GLdouble)zmin, (GLdouble)zmax);
}

void GL4Engine::GetDepthRange(float& zmin, float& zmax) const
{
    GLdouble param[2];
    glGetDoublev(GL_DEPTH_RANGE, param);
    zmin = (float)param[0];
    zmax = (float)param[1];
}

bool GL4Engine::Resize(unsigned int w, unsigned int h)
{
    mXSize = w;
    mYSize = h;
    int param[4];
    glGetIntegerv(GL_VIEWPORT, param);
    glViewport(param[0], param[1], (GLint)w, (GLint)h);
    return true;
}

void GL4Engine::ClearColorBuffer()
{
    glClearColor(mClearColor[0], mClearColor[1], mClearColor[2],
        mClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GL4Engine::ClearDepthBuffer()
{
    glClearDepth(mClearDepth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL4Engine::ClearStencilBuffer()
{
    glClearStencil((GLint)mClearStencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void GL4Engine::ClearBuffers()
{
    glClearColor(mClearColor[0], mClearColor[1], mClearColor[2],
        mClearColor[3]);
    glClearDepth(mClearDepth);
    glClearStencil((GLint)mClearStencil);
    glClear(
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GL4Engine::DisplayColorBuffer(unsigned int syncInterval)
{
    wglSwapIntervalEXT(syncInterval > 0 ? 1 : 0);
    SwapBuffers(mDevice);
}

inline void GL4Engine::SetDefaultBlendState()
{
    SetBlendState(mDefaultBlendState);
}

void GL4Engine::SetBlendState(std::shared_ptr<BlendState> const& state)
{
    if (state)
    {
        if (state != mActiveBlendState)
        {
            GL4BlendState* gl4State =
                static_cast<GL4BlendState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveBlendState = state;
            }
            else
            {
                LogError("Failed to bind blend state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

std::shared_ptr<BlendState> const& GL4Engine::GetDefaultBlendState() const
{
    return mDefaultBlendState;
}

std::shared_ptr<BlendState> const& GL4Engine::GetBlendState() const
{
    return mActiveBlendState;
}

void GL4Engine::SetDefaultDepthStencilState()
{
    SetDepthStencilState(mDefaultDepthStencilState);
}

void GL4Engine::SetDepthStencilState(
    std::shared_ptr<DepthStencilState> const& state)
{
    if (state)
    {
        if (state != mActiveDepthStencilState)
        {
            GL4DepthStencilState* gl4State =
                static_cast<GL4DepthStencilState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveDepthStencilState = state;
            }
            else
            {
                LogError("Failed to bind depth-stencil state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

std::shared_ptr<DepthStencilState> const&
GL4Engine::GetDefaultDepthStencilState() const
{
    return mDefaultDepthStencilState;
}

std::shared_ptr<DepthStencilState> const&
GL4Engine::GetDepthStencilState() const
{
    return mActiveDepthStencilState;
}

void GL4Engine::SetDefaultRasterizerState()
{
    SetRasterizerState(mDefaultRasterizerState);
}

void GL4Engine::SetRasterizerState(
    std::shared_ptr<RasterizerState> const& state)
{
    if (state)
    {
        if (state != mActiveRasterizerState)
        {
            GL4RasterizerState* gl4State =
                static_cast<GL4RasterizerState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveRasterizerState = state;
            }
            else
            {
                LogError("Failed to bind rasterizer state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

std::shared_ptr<RasterizerState> const&
GL4Engine::GetDefaultRasterizerState() const
{
    return mDefaultRasterizerState;
}

std::shared_ptr<RasterizerState> const&
GL4Engine::GetRasterizerState() const
{
    return mActiveRasterizerState;
}

void GL4Engine::SetDefaultFont()
{
    SetFont(mDefaultFont);
}

void GL4Engine::SetFont(std::shared_ptr<Font> const& font)
{
    if (font)
    {
        if (font != mActiveFont)
        {
            // Destroy font resources in GPU memory.  The mActiveFont should
            // be null once, only when the mDefaultFont is created.
            if (mActiveFont)
            {
                Unbind(mActiveFont->GetVertexBuffer());
                Unbind(mActiveFont->GetIndexBuffer());
                Unbind(mActiveFont->GetTextEffect()->GetTranslate());
                Unbind(mActiveFont->GetTextEffect()->GetColor());
                Unbind(mActiveFont->GetTextEffect()->GetVertexShader());
                Unbind(mActiveFont->GetTextEffect()->GetPixelShader());
            }

            mActiveFont = font;

            // Create font resources in GPU memory.
            Bind(mActiveFont->GetVertexBuffer());
            Bind(mActiveFont->GetIndexBuffer());
            Bind(mActiveFont->GetTextEffect()->GetTranslate());
            Bind(mActiveFont->GetTextEffect()->GetColor());
            Bind(mActiveFont->GetTextEffect()->GetVertexShader());
            Bind(mActiveFont->GetTextEffect()->GetPixelShader());
        }
    }
    else
    {
        LogError("Input font is null.");
    }
}

std::shared_ptr<Font> const& GL4Engine::GetDefaultFont() const
{
    return mDefaultFont;
}

std::shared_ptr<Font> const& GL4Engine::GetFont() const
{
    return mActiveFont;
}

uint64_t GL4Engine::Draw(std::shared_ptr<Visual> const& visual)
{
    return Draw(visual.get());
}

uint64_t GL4Engine::Draw(Visual const* visual)
{
    if (visual)
    {
        auto const& vbuffer = visual->GetVertexBuffer();
        auto const& ibuffer = visual->GetIndexBuffer();
        auto const& effect = visual->GetEffect();
        if (vbuffer && ibuffer && effect)
        {
            return Draw(vbuffer, ibuffer, effect);
        }
    }

    LogError("Null input to Draw.");
    return 0;
}

uint64_t GL4Engine::Draw(
    std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer,
    std::shared_ptr<VisualEffect> const& effect)
{
    uint64_t numPixelsDrawn = 0;

    if (EnableShaders(effect))
    {
        // TODO:  Need to make changes to VisualEffect before I can finish Draw.
        GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(
            effect->GetProgram().get());
        LogAssert(gl4program != nullptr, "HLSL effect passed to GLSL engine.");
        unsigned int programHandle = gl4program->GetProgramHandle();
        glUseProgram(programHandle);

        // Enable the vertex buffer and input layout.
        GL4VertexBuffer* gl4VBuffer = nullptr;
        GL4InputLayout* gl4Layout = nullptr;
        if (vbuffer->IsFormatted())
        {
            gl4VBuffer = static_cast<GL4VertexBuffer*>(Bind(vbuffer));
            gl4Layout = mILMap.Bind(programHandle, gl4VBuffer->GetGLHandle(),
                vbuffer.get());
            gl4Layout->Enable();
        }
        else
        {
            // TODO: Determine how to do the formatless vertex drawing in
            // OpenGL.
            LogError("Not yet implemented.");
        }

        // Enable the index buffer.
        GL4IndexBuffer* gl4IBuffer = nullptr;
        if (ibuffer->IsIndexed())
        {
            gl4IBuffer = static_cast<GL4IndexBuffer*>(Bind(ibuffer));
            gl4IBuffer->Enable();
        }

        numPixelsDrawn = DrawPrimitive(vbuffer.get(), ibuffer.get());

        // Disable the vertex buffer and input layout.
        if (vbuffer->IsFormatted())
        {
            gl4Layout->Disable();
        }

        // Disable the index buffer.
        if (gl4IBuffer)
        {
            gl4IBuffer->Disable();
        }

        DisableShaders(effect);

        glUseProgram(0);
    }

    return numPixelsDrawn;
}

GL4GraphicsObject* GL4Engine::Bind(
    std::shared_ptr<GraphicsObject> const& object)
{
    if (!object)
    {
        LogError("Attempt to bind a null object.");
        return nullptr;
    }

    GraphicsObject const* gtObject = object.get();
    GL4GraphicsObject* glObject = nullptr;
    if (!mGOMap.Get(gtObject, glObject))
    {
        // The 'create' function is not null with the current engine design.
        // If the assertion is triggered, someone changed the hierarchy of
        // GraphicsObjectType but did not change msCreateFunctions[] to match.
        CreateFunction create = msCreateFunctions[object->GetType()];
        // TEMP
        if (!create)
        {
            return nullptr;
        }
        // END TEMP
        LogAssert(create, "Null creation function.");

        glObject = create(gtObject);
        LogAssert(glObject, "Null object.  Out of memory?");

        mGOMap.Insert(gtObject, glObject);
    }
    return glObject;
}

bool GL4Engine::Unbind(std::shared_ptr<GraphicsObject> const& object)
{
    return Unbind(object.get());
}

GL4GraphicsObject* GL4Engine::Get(
    std::shared_ptr<GraphicsObject> const& object) const
{
    GraphicsObject const* gtObject = object.get();
    GL4GraphicsObject* gl4Object = nullptr;
    if (mGOMap.Get(gtObject, gl4Object))
    {
        return gl4Object;
    }
    return nullptr;
}

bool GL4Engine::Unbind(GraphicsObject const* object)
{
    GL4GraphicsObject* glObject = nullptr;
    if (mGOMap.Get(object, glObject))
    {
        GraphicsObjectType type = object->GetType();
        if (type == GT_VERTEX_BUFFER)
        {
            mILMap.Unbind(static_cast<VertexBuffer const*>(object));
        }
        //else if (type == GT_VERTEX_SHADER)  // Need GT_PROGRAM
        //{
        //    mILMap.Unbind(static_cast<Shader const*>(object));
        //}

        if (mGOMap.Remove(object, glObject))
        {
            delete glObject;
        }

        return true;
    }
    return false;
}

bool GL4Engine::Update(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        LogWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    GL4Buffer* gl4Buffer = static_cast<GL4Buffer*>(Bind(buffer));
    return gl4Buffer->Update();
}

bool GL4Engine::EnableShaders(std::shared_ptr<VisualEffect> const& effect)
{
    VertexShader* vshader = effect->GetVertexShader().get();
    if (!vshader)
    {
        LogError("Effect does not have a vertex shader.");
        return false;
    }

    PixelShader* pshader = effect->GetPixelShader().get();
    if (!pshader)
    {
        LogError("Effect does not have a pixel shader.");
        return false;
    }

    GeometryShader* gshader = effect->GetGeometryShader().get();

    // Enable the shader resources.
    Enable(vshader);
    Enable(pshader);
    if (gshader)
    {
        Enable(gshader);
    }

    return true;
}

void GL4Engine::DisableShaders(std::shared_ptr<VisualEffect> const& effect)
{
    VertexShader* vshader = effect->GetVertexShader().get();
    PixelShader* pshader = effect->GetPixelShader().get();
    GeometryShader* gshader = effect->GetGeometryShader().get();

    if (gshader)
    {
        Disable(gshader);
    }
    Disable(pshader);
    Disable(vshader);
}

void GL4Engine::Enable(Shader const* shader)
{
    EnableCBuffers(shader);
    EnableTBuffers(shader);
    EnableSBuffers(shader);
    EnableRBuffers(shader);
    EnableTextures(shader);
    EnableTextureArrays(shader);
    EnableSamplers(shader);
}

void GL4Engine::Disable(Shader const* shader)
{
    DisableSamplers(shader);
    DisableTextureArrays(shader);
    DisableTextures(shader);
    DisableRBuffers(shader);
    DisableSBuffers(shader);
    DisableTBuffers(shader);
    DisableCBuffers(shader);
}

void GL4Engine::EnableCBuffers(Shader const* shader)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        if (cb.object)
        {
            GL4ConstantBuffer* gl4CB =
                static_cast<GL4ConstantBuffer*>(Bind(cb.object));

            if (gl4CB)
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, cb.bindPoint,
                    gl4CB->GetGLHandle());
            }
            else
            {
                LogError("Failed to bind constant buffer.");
            }
        }
        else
        {
            LogError(cb.name + " is null constant buffer.");
        }
    }
}

void GL4Engine::DisableCBuffers(Shader const* shader)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, cb.bindPoint, 0);
    }
}

void GL4Engine::EnableTBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::DisableTBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::EnableSBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::DisableSBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::EnableRBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::DisableRBuffers(Shader const*)
{
    // TODO
}

void GL4Engine::EnableTextures(Shader const*)
{
    // TODO
}

void GL4Engine::DisableTextures(Shader const*)
{
    // TODO
}

void GL4Engine::EnableTextureArrays(Shader const*)
{
    // TODO
}

void GL4Engine::DisableTextureArrays(Shader const*)
{
    // TODO
}

void GL4Engine::EnableSamplers(Shader const*)
{
    // TODO
}

void GL4Engine::DisableSamplers(Shader const*)
{
    // TODO
}

uint64_t GL4Engine::DrawPrimitive(VertexBuffer const* vbuffer,
    IndexBuffer const* ibuffer)
{
    // TODO.  Only supports triangle primitives right now.  Need to support
    // 16-bit and 32-bit index buffer indices.
    unsigned int numActiveVertices = vbuffer->GetNumActiveElements();
    unsigned int vertexOffset = vbuffer->GetOffset();

    unsigned int numActiveIndices = ibuffer->GetNumActiveIndices();
    //unsigned int firstIndex = ibuffer->GetFirstIndex();
    unsigned int indexSize = ibuffer->GetElementSize();
    GLenum indexType = (indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT);
    IPType type = ibuffer->GetPrimitiveType();
    (void)type;  // TODO: topology lookup, for now use GL_TRIANGLES
    GLenum topology = GL_TRIANGLES;

    unsigned int offset = ibuffer->GetOffset();
    if (ibuffer->IsIndexed())
    {
        void const* data = (char*)0 + indexSize * offset;
        glDrawRangeElements(topology, 0, numActiveVertices - 1,
            (GLsizei)numActiveIndices, indexType, data);
    }
    else
    {
        glDrawArrays(topology, (GLint)vertexOffset, (GLint)numActiveVertices);
    }
    return 0;
}

void GL4Engine::Initialize()
{
    mXSize = 0;
    mYSize = 0;

    mClearColor[0] = 1.0f;
    mClearColor[1] = 1.0f;
    mClearColor[2] = 1.0f;
    mClearColor[3] = 1.0f;
    mClearDepth = 1.0f;
    mClearStencil = 0;

    mDefaultBlendState = nullptr;
    mActiveBlendState = nullptr;
    mDefaultDepthStencilState = nullptr;
    mActiveDepthStencilState = nullptr;
    mDefaultRasterizerState = nullptr;
    mActiveRasterizerState = nullptr;

    mDefaultFont = nullptr;
    mActiveFont = nullptr;

    mGOListener = new GOListener(this);
    GraphicsObject::SubscribeForDestruction(mGOListener);

    mDTListener = new DTListener(this);
    //DrawTarget::SubscribeForDestruction(mDTListener);

    mAllowOcclusionQuery = false;
    mWarnOnNonemptyBridges = true;
}

void GL4Engine::CreateDefaultObjects()
{
    GLSLProgramFactory factory;
    mDefaultFont = std::make_shared<FontArialW400H18>(factory, 256);
    mDefaultBlendState = std::make_shared<BlendState>();
    mDefaultDepthStencilState = std::make_shared<DepthStencilState>();
    mDefaultRasterizerState = std::make_shared<RasterizerState>();

    SetDefaultFont();
    SetDefaultBlendState();
    SetDefaultDepthStencilState();
    SetDefaultRasterizerState();
}

void GL4Engine::DestroyDefaultObjects()
{
    if (mDefaultFont)
    {
        mDefaultFont = nullptr;
        mActiveFont = nullptr;
    }

    if (mDefaultBlendState)
    {
        Unbind(mDefaultBlendState);
        mDefaultBlendState = nullptr;
        mActiveBlendState = nullptr;
    }

    if (mDefaultDepthStencilState)
    {
        Unbind(mDefaultDepthStencilState);
        mDefaultDepthStencilState = nullptr;
        mActiveDepthStencilState = nullptr;
    }

    if (mDefaultRasterizerState)
    {
        Unbind(mDefaultRasterizerState);
        mDefaultRasterizerState = nullptr;
        mActiveRasterizerState = nullptr;
    }
}

GL4Engine::GOListener::~GOListener()
{
}

GL4Engine::GOListener::GOListener(GL4Engine* engine)
    :
    mEngine(engine)
{
}

void GL4Engine::GOListener::OnDestroy(GraphicsObject const* object)
{
    if (mEngine)
    {
        mEngine->Unbind(object);
    }
}

GL4Engine::DTListener::~DTListener()
{
}

GL4Engine::DTListener::DTListener(GL4Engine* engine)
    :
    mEngine(engine)
{
}

//void GL4Engine::DTListener::OnDestroy(DrawTarget const* target)
//{
//    if (mEngine)
//    {
//        mEngine->Unbind(target);
//    }
//}


// Factory creation for bridge building in Bind(...).
GL4Engine::CreateFunction const GL4Engine::msCreateFunctions[GT_NUM_TYPES] =
{
    nullptr,    // GT_GRAPHICS_OBJECT (abstract base)
    nullptr,    // GT_RESOURCE (abstract base)
    nullptr,    // GT_BUFFER (abstract base)
    &GL4ConstantBuffer::Create,
    nullptr, // &DX11TextureBuffer::Create,
    &GL4VertexBuffer::Create,
    &GL4IndexBuffer::Create,
    nullptr, // &DX11StructuredBuffer::Create,
    nullptr,  // TODO:  Implement TypedBuffer
    nullptr, // &DX11RawBuffer::Create,
    nullptr, // &DX11IndirectArgumentsBuffer::Create,
    nullptr,    // GT_TEXTURE (abstract base)
    nullptr,    // GT_TEXTURE_SINGLE (abstract base)
    nullptr, // &DX11Texture1::Create,
    nullptr, // &DX11Texture2::Create,
    nullptr, // &DX11TextureRT::Create,
    nullptr, // &DX11TextureDS::Create,
    nullptr, // &DX11Texture3::Create,
    nullptr,  // GT_TEXTURE_ARRAY (abstract base)
    nullptr, // &DX11Texture1Array::Create,
    nullptr, // &DX11Texture2Array::Create,
    nullptr, // &DX11TextureCube::Create,
    nullptr, // &DX11TextureCubeArray::Create,
    nullptr,    // GT_SHADER (abstract base)
    nullptr, // &DX11VertexShader::Create,
    nullptr, // &DX11GeometryShader::Create,
    nullptr, // &DX11PixelShader::Create,
    nullptr, // &DX11ComputeShader::Create,
    nullptr,    // GT_DRAWING_STATE (abstract base)
    nullptr, // &DX11SamplerState::Create,
    &GL4BlendState::Create,
    &GL4DepthStencilState::Create,
    &GL4RasterizerState::Create
};
