// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <LowLevel/GteThreadSafeMap.h>
#include <Mathematics/GteVector4.h>
#include <Graphics/GteBlendState.h>
#include <Graphics/GteDepthStencilState.h>
#include <Graphics/GteDrawTarget.h>
#include <Graphics/GteFont.h>
#include <Graphics/GteRasterizerState.h>
#include <Graphics/GteVisual.h>
#include <Graphics/GL4/GteGL4InputLayoutManager.h>

namespace gte
{

class GL4GraphicsObject;

class GTE_IMPEXP GL4Engine
{
public:
    // Construction and destruction.
    ~GL4Engine();
    GL4Engine(HWND handle, bool pixelFormatAlreadyKnown);

    // Access to members that correspond to constructor inputs.
    inline HDC GetDevice() const;
    inline HGLRC GetImmediate() const;

    // Allow the user to switch between OpenGL contexts when there are
    // multiple instances of GL4Engine in an application.
    bool IsActive() const;
    void MakeActive();

    // Viewport management.  OpenGL uses a right-handed system with origin at
    // lower-left corner of rectangle.
    void SetViewport(int x, int y, int w, int h);
    void GetViewport(int& x, int& y, int& w, int& h) const;
    void SetDepthRange(float zmin, float zmax);
    void GetDepthRange(float& zmin, float& zmax) const;

    // Window resizing.
    bool Resize(unsigned int w, unsigned int h);

    // Access to the current clearing parameters for the color, depth, and
    // stencil buffers.  The color buffer is the back buffer.
    inline void SetClearColor(Vector4<float> const& clearColor);
    inline void SetClearDepth(float clearDepth);
    inline void SetClearStencil(unsigned int clearStencil);
    inline Vector4<float> const& GetClearColor() const;
    inline float GetClearDepth() const;
    inline unsigned int GetClearStencil() const;
    void ClearColorBuffer();
    void ClearDepthBuffer();
    void ClearStencilBuffer();
    void ClearBuffers();
    void DisplayColorBuffer(unsigned int syncInterval);

    // The default blend state is listed in GteBlendState.h.
    void SetDefaultBlendState();
    void SetBlendState(std::shared_ptr<BlendState> const& state);
    std::shared_ptr<BlendState> const& GetDefaultBlendState() const;
    std::shared_ptr<BlendState> const& GetBlendState() const;

    // The default depth-stencil state is listed in GteDepthStencilState.h.
    void SetDefaultDepthStencilState();
    void SetDepthStencilState(
        std::shared_ptr<DepthStencilState> const& state);
    std::shared_ptr<DepthStencilState> const& GetDefaultDepthStencilState()
        const;
    std::shared_ptr<DepthStencilState> const& GetDepthStencilState()
        const;

    // The default rasterizer state is listed in GteRasterizerState.h.
    void SetDefaultRasterizerState();
    void SetRasterizerState(std::shared_ptr<RasterizerState> const& state);
    std::shared_ptr<RasterizerState> const& GetDefaultRasterizerState() const;
    std::shared_ptr<RasterizerState> const& GetRasterizerState() const;

    // Support for bitmapped fonts used in text rendering.  The default font
    // is Arial (height 18, no italics, no bold).
    void SetDefaultFont();
    void SetFont(std::shared_ptr<Font> const& font);
    std::shared_ptr<Font> const& GetDefaultFont() const;
    std::shared_ptr<Font> const& GetFont() const;

    // Drawing a geometric primitive.  If occlusion queries are enabled, the
    // return value is the number of samples that passed the depth and stencil
    // tests, effectively the number of pixels drawn.  If occlusion queries
    // are not enabled, the functions return 0.
    uint64_t Draw(Visual const* visual);
    uint64_t Draw(std::shared_ptr<Visual> const& visual);
    uint64_t Draw(
        std::shared_ptr<VertexBuffer> const& vbuffer,
        std::shared_ptr<IndexBuffer> const& ibuffer,
        std::shared_ptr<VisualEffect> const& effect);

    // Execute the compute shader.  If you want the CPU to stall to wait for
    // the results, call WaitForFinish() immediately after Execute(...).
    // However, you can synchronize CPU and GPU activity by calling
    // WaitForFinish() at some later time, the goal being not to stall the
    // CPU before obtaining the GPU results.
    void Execute(std::shared_ptr<ComputeShader> const&,
        unsigned int, unsigned int, unsigned int)
    {
        // TODO
    }

    // Have the CPU wait until the GPU finishes its current command buffer.
    void WaitForFinish()
    {
        // TODO
    }

    // Graphics object management.
    GL4GraphicsObject* Bind(std::shared_ptr<GraphicsObject> const& object);
    bool Unbind(std::shared_ptr<GraphicsObject> const& object);
    GL4GraphicsObject* Get(std::shared_ptr<GraphicsObject> const& object)
        const;

    // Support for copying from CPU to GPU via mapped memory.
    bool Update(std::shared_ptr<Buffer> const& buffer);

    // Support for occlusion queries.  When enabled, Draw functions return the
    // number of samples that passed the depth and stencil tests, effectively
    // the number of pixels drawn.  The default value is 'false'.
    inline void AllowOcclusionQuery(bool allow);

    // Set the warning to 'true' if you want the DX11Engine destructor to
    // report that the bridge maps are nonempty.  If they are, the application
    // did not destroy GraphicsObject items before the engine was destroyed.
    // The default values is 'true'.
    inline void WarnOnNonemptyBridges(bool warn);

public:  // TODO: Temporary public scope for testing/debugging.
    bool Unbind(GraphicsObject const* object);

    // Helpers for construction and destruction.
    void Initialize();
    void CreateDefaultObjects();
    void DestroyDefaultObjects();

    // Support for enabling and disabling resources used by shaders.
    bool EnableShaders(std::shared_ptr<VisualEffect> const& effect);
    void DisableShaders(std::shared_ptr<VisualEffect> const& effect);
    void Enable(Shader const* shader);
    void Disable(Shader const* shader);
    void EnableCBuffers(Shader const* shader);
    void DisableCBuffers(Shader const* shader);
    void EnableTBuffers(Shader const* shader);
    void DisableTBuffers(Shader const* shader);
    void EnableSBuffers(Shader const* shader);
    void DisableSBuffers(Shader const* shader);
    void EnableRBuffers(Shader const* shader);
    void DisableRBuffers(Shader const* shader);
    void EnableTextures(Shader const* shader);
    void DisableTextures(Shader const* shader);
    void EnableTextureArrays(Shader const* shader);
    void DisableTextureArrays(Shader const* shader);
    void EnableSamplers(Shader const* shader);
    void DisableSamplers(Shader const* shader);

    // Support for drawing.
    uint64_t DrawPrimitive(VertexBuffer const* vbuffer,
        IndexBuffer const* ibuffer);

    // Inputs to the constructor.
    HWND mHandle;

    // Objects created by the constructors.
    HDC mDevice;
    HGLRC mImmediate;

    // Objects created by the constructors for graphics engines.
    unsigned int mXSize, mYSize;

    // Support for framebuffer clearing.
    Vector4<float> mClearColor;
    float mClearDepth;
    unsigned int mClearStencil;

    // Support for render state.
    std::shared_ptr<BlendState> mDefaultBlendState;
    std::shared_ptr<BlendState> mActiveBlendState;
    std::shared_ptr<DepthStencilState> mDefaultDepthStencilState;
    std::shared_ptr<DepthStencilState> mActiveDepthStencilState;
    std::shared_ptr<RasterizerState> mDefaultRasterizerState;
    std::shared_ptr<RasterizerState> mActiveRasterizerState;

    // Support for fonts.
    std::shared_ptr<Font> mDefaultFont;
    std::shared_ptr<Font> mActiveFont;

    // Bridge pattern to create API-dependent objects that correspond to GTE
    // objects.
    ThreadSafeMap<GraphicsObject const*, GL4GraphicsObject*> mGOMap;
    ThreadSafeMap<DrawTarget const*, GL4GraphicsObject*> mDTMap;

    // Track GraphicsObject destruction and delete to-be-destroyed objects
    // from the bridge map.
    class GOListener
        :
        public GraphicsObject::ListenerForDestruction
    {
    public:
        virtual ~GOListener();
        GOListener(GL4Engine* engine);
        virtual void OnDestroy(GraphicsObject const* object);
    private:
        GL4Engine* mEngine;
    };

    // Track DrawTarget destruction and delete to-be-destroyed objects from
    // the draw target map.
    class DTListener : public DrawTarget::ListenerForDestruction
    {
    public:
        virtual ~DTListener();
        DTListener(GL4Engine* engine);
        //virtual void OnDestroy(DrawTarget const* target);
    private:
        GL4Engine* mEngine;
    };

    GOListener* mGOListener;
    DTListener* mDTListener;

    bool mAllowOcclusionQuery;
    bool mWarnOnNonemptyBridges;

    // Creation functions for adding objects to the bridges.
    typedef GL4GraphicsObject* (*CreateFunction)(GraphicsObject const*);
    static CreateFunction const msCreateFunctions[GT_NUM_TYPES];

    // Bridge pattern to create GL4 objects that correspond to GTE objects.
    GL4InputLayoutManager mILMap;
};


inline HDC GL4Engine::GetDevice() const
{
    return mDevice;
}

inline HGLRC GL4Engine::GetImmediate() const
{
    return mImmediate;
}

inline void GL4Engine::SetClearColor(Vector4<float> const& clearColor)
{
    mClearColor = clearColor;
}

inline void GL4Engine::SetClearDepth(float clearDepth)
{
    mClearDepth = clearDepth;
}

inline void GL4Engine::SetClearStencil(unsigned int clearStencil)
{
    mClearStencil = clearStencil;
}

inline Vector4<float> const& GL4Engine::GetClearColor() const
{
    return mClearColor;
}

inline float GL4Engine::GetClearDepth() const
{
    return mClearDepth;
}

inline unsigned int GL4Engine::GetClearStencil() const
{
    return mClearStencil;
}

inline void GL4Engine::AllowOcclusionQuery(bool allow)
{
    mAllowOcclusionQuery = allow;
}

inline void GL4Engine::WarnOnNonemptyBridges(bool warn)
{
    mWarnOnNonemptyBridges = warn;
}


}
