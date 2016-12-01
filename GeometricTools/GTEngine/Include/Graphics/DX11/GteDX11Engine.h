// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <LowLevel/GteThreadSafeMap.h>
#include <Graphics/GteBlendState.h>
#include <Graphics/GteDepthStencilState.h>
#include <Graphics/GteDrawTarget.h>
#include <Graphics/GteFont.h>
#include <Graphics/GteRasterizerState.h>
#include <Graphics/DX11/GteDX11InputLayoutManager.h>
#include <Graphics/DX11/GteDX11PerformanceCounter.h>
#include <Graphics/DX11/GteDX11Resource.h>

// Allow names to be assigned to DX11-created objects for use by the
// Direct3D11 debug layer when you enable it using the DirectX Control Panel.
#define GTE_USE_NAMED_DX11_OBJECTS

namespace gte
{
    
class ComputeShader;
class ConstantBuffer;
class IndexBuffer;
class IndirectArgumentsBuffer;
class OverlayEffect;
class RawBuffer;
class Resource;
class SamplerState;
class Shader;
class StructuredBuffer;
class StructuredCountBuffer;
class Texture1;
class Texture2;
class TextureRT;
class TextureDS;
class Texture3;
class Texture1Array;
class Texture2Array;
class TextureCube;
class TextureCubeArray;
class TextureBuffer;
class VertexBuffer;
class VertexShader;
class Visual;
class VisualEffect;

class DX11AppendConsumeBuffer;
class DX11BlendState;
class DX11ByteAddressBuffer;
class DX11ComputeShader;
class DX11ConstantBuffer;
class DX11DepthStencilState;
class DX11DrawTarget;
class DX11GeometryShader;
class DX11IndexBuffer;
class DX11IndirectArgumentsBuffer;
class DX11PixelShader;
class DX11RasterizerState;
class DX11Resource;
class DX11SamplerState;
class DX11Shader;
class DX11StructuredBuffer;
class DX11StructuredCountBuffer;
class DX11Texture1;
class DX11Texture2;
class DX11TextureRT;
class DX11TextureDS;
class DX11Texture3;
class DX11Texture1Array;
class DX11Texture2Array;
class DX11TextureCube;
class DX11TextureCubeArray;
class DX11TextureBuffer;
class DX11VertexBuffer;
class DX11VertexShader;

class GTE_IMPEXP DX11Engine
{
public:
    // Construction and destruction.
     ~DX11Engine();

    // Constructors for computing.  The first constructor uses the default
    // adapter and tries for DX11.0 hardware acceleration (the maximum feature
    // level) without debugging support; it is equivalent to the second
    // constructor call:
    //   DX11Engine(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
    //     D3D_FEATURE_LEVEL_11_0);
    //
    DX11Engine(D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0);
    DX11Engine(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
        HMODULE softwareModule, UINT flags,
        D3D_FEATURE_LEVEL minFeatureLevel);

    // Constructors for graphics (automatically get computing capabilities).
    // The first constructor uses the default adapter and tries for DX11.0
    // hardware acceleration (the maximum feature level) without debugging
    // support; it is equivalent to the second constructor call:
    //   DX11Engine(nullptr, handle, xSize, ySize, D3D_DRIVER_TYPE_HARDWARE,
    //     nullptr, 0, D3D_FEATURE_LEVEL_11_0);
    //
    DX11Engine(HWND handle, UINT xSize, UINT ySize,
        D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0);
    DX11Engine(IDXGIAdapter* adapter, HWND handle, UINT xSize, UINT ySize,
        D3D_DRIVER_TYPE driverType, HMODULE softwareModule, UINT flags,
        D3D_FEATURE_LEVEL minFeatureLevel);

    // Access to members that correspond to constructor inputs.
    inline IDXGIAdapter* GetAdapter() const;
    inline D3D_DRIVER_TYPE GetDriverType() const;
    inline HMODULE GetSoftwareModule() const;
    inline UINT GetFlags() const;
    inline D3D_FEATURE_LEVEL GetMinFeatureLevel() const;
    inline ID3D11Device* GetDevice() const;
    inline ID3D11DeviceContext* GetImmediate() const;
    inline D3D_FEATURE_LEVEL GetFeatureLevel() const;

private:
    // Helpers for construction and destruction.
    void Initialize(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
        HMODULE softwareModule, UINT flags, D3D_FEATURE_LEVEL minFeatureLevel);
    bool CreateDevice();
    bool CreateSwapChain(HWND handle, UINT xSize, UINT ySize);
    bool CreateBackBuffer(UINT xSize, UINT ySize);
    void CreateDefaultObjects();
    void DestroyDefaultObjects();
    bool DestroyDevice();
    bool DestroySwapChain();
    bool DestroyBackBuffer();

    // Inputs to the constructors.
    IDXGIAdapter* mAdapter;
    D3D_DRIVER_TYPE mDriverType;
    HMODULE mSoftwareModule;
    UINT mFlags;
    D3D_FEATURE_LEVEL mMinFeatureLevel;

    // Objects created by the constructors.
    ID3D11Device* mDevice;
    ID3D11DeviceContext* mImmediate;
    D3D_FEATURE_LEVEL mFeatureLevel;

    // Objects created by the constructors for graphics engines.
    unsigned int mXSize, mYSize;
    IDXGISwapChain* mSwapChain;
    ID3D11Texture2D* mColorBuffer;
    ID3D11RenderTargetView* mColorView;
    ID3D11Texture2D* mDepthStencilBuffer;
    ID3D11DepthStencilView* mDepthStencilView;
    D3D11_VIEWPORT mViewport;

public:
    // Graphics object management.  The Bind function creates a DX11 object
    // that corresponds to the input GTEngine object.  DX11Engine manages
    // this bridge mapping internally.  The Unbind function destroys the DX11
    // object.  These may be called explicitly, but the engine is designed
    // to create on demand and to destroy on device destruction.
    DX11GraphicsObject* Bind(std::shared_ptr<GraphicsObject> const& object);
    inline bool Unbind(std::shared_ptr<GraphicsObject> const& object);
    DX11GraphicsObject* Get(std::shared_ptr<GraphicsObject> const& object)
        const;
    DX11DrawTarget* Bind(std::shared_ptr<DrawTarget> const& target);
    inline bool Unbind(std::shared_ptr<DrawTarget> const& target);
    DX11DrawTarget* Get(std::shared_ptr<DrawTarget> const& target) const;

    // Count the number of objects and bytes of system memory used by the
    // graphics-object map.
    size_t GetTotalAllocation(size_t& numObjects) const;

private:
    // Support for GOListener::OnDestroy and DTListener::OnDestroy, because
    // they are passed raw pointers from resource destructors.  These are
    // also used by the Unbind calls whose inputs are std::shared_ptr<T>.
    bool Unbind(GraphicsObject const* object);
    bool Unbind(DrawTarget const* target);

public:
    // Support for sharing textures by devices.  If the 'texture' is bound to
    // the 'engine', then it is shared by 'this'.  If it is not bound to
    // 'engine', this function behaves just like 'Bind'.  In both cases,
    // a new DX11Texture2 wrapper is returned that is associated with 'this'.
    // You may unbind as desired.
    DX11GraphicsObject* Share(std::shared_ptr<Texture2> const& texture,
        DX11Engine* engine);

    // Mapped copying from CPU to GPU.
    D3D11_MAPPED_SUBRESOURCE MapForWrite(
        std::shared_ptr<Resource> const& resource, unsigned int sri);
    void Unmap(std::shared_ptr<Resource> const& resource, unsigned int sri);

    // Support for copying from CPU to GPU via mapped memory.
    bool Update(std::shared_ptr<Buffer> const& buffer);
    bool Update(std::shared_ptr<Texture> const& texture);
    bool Update(std::shared_ptr<Texture> const& texture,
        unsigned int level);
    bool Update(std::shared_ptr<TextureArray> const& textureArray);
    bool Update(std::shared_ptr<TextureArray> const& textureArray,
        unsigned int item, unsigned int level);

    // Support for copying from CPU to GPU via staging memory.
    bool CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer);
    bool CopyCpuToGpu(std::shared_ptr<Texture> const& texture);
    bool CopyCpuToGpu(std::shared_ptr<Texture> const& texture,
        unsigned int level);
    bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray);
    bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray,
        unsigned int item, unsigned int level);

    // Support for copying from GPU to CPU via staging memory.
    bool CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer);
    bool CopyGpuToCpu(std::shared_ptr<Texture> const& texture);
    bool CopyGpuToCpu(std::shared_ptr<Texture> const& texture,
        unsigned int level);
    bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray);
    bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray,
        unsigned int item, unsigned int level);

    // Counted buffer management.  GetNumActiveElements stores the
    // result in 'buffer'.
    bool GetNumActiveElements(
        std::shared_ptr<StructuredBuffer> const& buffer);

    // Viewport management.  The DX11 viewport is specified in left-handed
    // window coordinates.  The origin (x,y) is the upper-left corner of the
    // window, the y-axis is directed downward, and the x-axis is directed
    // rightward.  The width is w and the height is h.  The depth range
    // [zmin,zmax] is a subset of [0,1].  Read the comments below about the
    // default rasterizer state; in particular, the triangle winding order
    // is determined by this state and has an implicit relationship to the
    // left-handedness of the viewport.
    void SetViewport(int x, int y, int w, int h);
    void GetViewport(int& x, int& y, int& w, int& h) const;
    void SetDepthRange(float zmin, float zmax);
    void GetDepthRange(float& zmin, float& zmax) const;

    // Window resizing.
    bool Resize(unsigned int w, unsigned int h);

    // Support for toggling between window and fullscreen modes.  The return
    // value is 'true' iff the operation succeeded.
    bool IsFullscreen(IDXGIOutput* output) const;
    bool SetFullscreen(IDXGIOutput* output, bool fullscreen);

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
    inline void SetDefaultBlendState();
    void SetBlendState(std::shared_ptr<BlendState> const& state);
    inline std::shared_ptr<BlendState> const& GetDefaultBlendState() const;
    inline std::shared_ptr<BlendState> const& GetBlendState() const;

    // The default depth-stencil state is listed in GteDepthStencilState.h.
    inline void SetDefaultDepthStencilState();
    void SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state);
    inline std::shared_ptr<DepthStencilState> const&
        GetDefaultDepthStencilState() const;
    inline std::shared_ptr<DepthStencilState> const& GetDepthStencilState()
        const;

    // The default rasterizer state is listed in GteRasterizerState.h.
    inline void SetDefaultRasterizerState();
    void SetRasterizerState(std::shared_ptr<RasterizerState> const& state);
    inline std::shared_ptr<RasterizerState> const&
        GetDefaultRasterizerState() const;
    inline std::shared_ptr<RasterizerState> const& GetRasterizerState() const;

    // Support for bitmapped fonts used in text rendering.  The default font
    // is Arial (height 18, no italics, no bold).
    inline void SetDefaultFont();
    void SetFont(std::shared_ptr<Font> const& font);
    inline std::shared_ptr<Font> const& GetDefaultFont() const;
    inline std::shared_ptr<Font> const& GetFont() const;

    // Support for drawing to offscreen memory (i.e. not to the back buffer).
    // The DrawTarget object encapsulates render targets (color information)
    // and depth-stencil target.
    void Enable(std::shared_ptr<DrawTarget> const& target);
    void Disable(std::shared_ptr<DrawTarget> const& target);

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

    // Draw 2D text.  If occlusion queries are enabled, the return value is
    // the number of samples that passed the depth and stencil tests,
    // effectively the number of pixels drawn.
    uint64_t Draw(int x, int y, Vector4<float> const& color,
        std::string const& message);

    // Draw a 2D rectangular overlay.  This is useful for adding buttons,
    // controls, thumbnails, and other GUI objects to an application window.
    uint64_t Draw(std::shared_ptr<OverlayEffect> const& overlay);

private:
    // Support for drawing.  If occlusion queries are enabled, the return
    // value of Draw* is the number of samples that passed the depth and
    // stencil tests.
    uint64_t DrawPrimitive(VertexBuffer const* vbuffer,
        IndexBuffer const* ibuffer);
    ID3D11Query* BeginOcclusionQuery();
    uint64_t EndOcclusionQuery(ID3D11Query* occlusionQuery);

public:
    // Execute the compute shader.  If you want the CPU to stall to wait for
    // the results, call WaitForFinish() immediately after Execute(...).
    // However, you can synchronize CPU and GPU activity by calling
    // WaitForFinish() at some later time, the goal being not to stall the
    // CPU before obtaining the GPU results.
    void Execute(std::shared_ptr<ComputeShader> const& cshader,
        unsigned int numXGroups, unsigned int numYGroups,
        unsigned int numZGroups);

    // Have the CPU wait until the GPU finishes its current command buffer.
    void WaitForFinish();

    // Support for occlusion queries.  When enabled, Draw functions return the
    // number of samples that passed the depth and stencil tests, effectively
    // the number of pixels drawn.  The default value is 'false'.
    inline void AllowOcclusionQuery(bool allow);

    // Set the warning to 'true' if you want the DX11Engine destructor to
    // report that the bridge maps are nonempty.  If they are, the application
    // did not destroy GraphicsObject items before the engine was destroyed.
    // The default values is 'true'.
    inline void WarnOnNonemptyBridges(bool warn);

    // Support for GPU timing. This is a coarse-level measurement system.
    // Each graphics card manufacturer provides more accurate measurements
    // and more performance counters than just simple timing.
    void BeginTimer(DX11PerformanceCounter& counter);
    void EndTimer(DX11PerformanceCounter& counter);

private:
    // Support for enabling and disabling resources used by shaders.
    bool EnableShaders(std::shared_ptr<VisualEffect> const& effect,
        DX11VertexShader*& dxVShader, DX11GeometryShader*& dxGShader,
        DX11PixelShader*& dxPShader);
    void DisableShaders(std::shared_ptr<VisualEffect> const& effect,
        DX11VertexShader* dxVShader, DX11GeometryShader* dxGShader,
        DX11PixelShader* dxPShader);
    void Enable(Shader const* shader, DX11Shader* dxShader);
    void Disable(Shader const* shader, DX11Shader* dxShader);
    void EnableCBuffers(Shader const* shader, DX11Shader* dxShader);
    void DisableCBuffers(Shader const* shader, DX11Shader* dxShader);
    void EnableTBuffers(Shader const* shader, DX11Shader* dxShader);
    void DisableTBuffers(Shader const* shader, DX11Shader* dxShader);
    void EnableSBuffers(Shader const* shader, DX11Shader* dxShader);
    void DisableSBuffers(Shader const* shader, DX11Shader* dxShader);
    void EnableRBuffers(Shader const* shader, DX11Shader* dxShader);
    void DisableRBuffers(Shader const* shader, DX11Shader* dxShader);
    void EnableTextures(Shader const* shader, DX11Shader* dxShader);
    void DisableTextures(Shader const* shader, DX11Shader* dxShader);
    void EnableTextureArrays(Shader const* shader, DX11Shader* dxShader);
    void DisableTextureArrays(Shader const* shader, DX11Shader* dxShader);
    void EnableSamplers(Shader const* shader, DX11Shader* dxShader);
    void DisableSamplers(Shader const* shader, DX11Shader* dxShader);

    // Store the ID of the thread on which the device was created.  This is
    // useful for diagnosing when the immediate context is called in a thread
    // different from the one on which the device was created (which is not
    // thread safe).
    DWORD mThreadID;

    // Support for draw target enabling and disabling.
    unsigned int mNumActiveRTs;
    ID3D11RenderTargetView* mActiveRT[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    ID3D11DepthStencilView* mActiveDS;
    D3D11_VIEWPORT mSaveViewport;
    ID3D11RenderTargetView* mSaveRT[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    ID3D11DepthStencilView* mSaveDS;

    // Keep track of whether the window is fullscreen or normal mode.  As
    // recommended by MSDN documentation, we do not use swapChainDesc.Windowed
    // to control this; rather, we create a windowed-mode device and allow the
    // programmer to use SetFullscreenState to toggle between windowed and
    // fullscreen.
    std::map<IDXGIOutput*,bool> mFullscreenState;

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

    // Bridge pattern to create DX11 objects that correspond to GTE objects.
    ThreadSafeMap<GraphicsObject const*, DX11GraphicsObject*> mGOMap;
    ThreadSafeMap<DrawTarget const*, DX11DrawTarget*> mDTMap;
    DX11InputLayoutManager mILMap;

    // Track GraphicsObject destruction and delete to-be-destroyed objects
    // from the bridge map.
    class GOListener
        :
        public GraphicsObject::ListenerForDestruction
    {
    public:
        virtual ~GOListener();
        GOListener(DX11Engine* engine);
        virtual void OnDestroy(GraphicsObject const* object);
    private:
        DX11Engine* mEngine;
    };

    GOListener* mGOListener;

    // Track DrawTarget destruction and delete to-be-destroyed objects from
    // the draw target map.
    class DTListener : public DrawTarget::ListenerForDestruction
    {
    public:
        virtual ~DTListener();
        DTListener(DX11Engine* engine);
        virtual void OnDestroy(DrawTarget const* target);
    private:
        DX11Engine* mEngine;
    };

    DTListener* mDTListener;

    bool mAllowOcclusionQuery;
    bool mWarnOnNonemptyBridges;

    // Creation functions for adding objects to the bridges.
    typedef DX11GraphicsObject* (*CreateFunction)(ID3D11Device*,
        GraphicsObject const*);
    static CreateFunction const msCreateFunctions[GT_NUM_TYPES];

    // Conversions from GTEngine values to DX11 values.
    // Lookups are by Log2(IP_*).
    static D3D11_PRIMITIVE_TOPOLOGY const msPrimitiveType[IP_NUM_TYPES];
};


inline IDXGIAdapter* DX11Engine::GetAdapter() const
{
    return mAdapter;
}

inline D3D_DRIVER_TYPE DX11Engine::GetDriverType() const
{
    return mDriverType;
}

inline HMODULE DX11Engine::GetSoftwareModule() const
{
    return mSoftwareModule;
}

inline UINT DX11Engine::GetFlags() const
{
    return mFlags;
}

inline D3D_FEATURE_LEVEL DX11Engine::GetMinFeatureLevel() const
{
    return mMinFeatureLevel;
}

inline ID3D11Device* DX11Engine::GetDevice() const
{
    return mDevice;
}

inline ID3D11DeviceContext* DX11Engine::GetImmediate() const
{
    return mImmediate;
}

inline D3D_FEATURE_LEVEL DX11Engine::GetFeatureLevel() const
{
    return mFeatureLevel;
}

inline void DX11Engine::SetClearColor(Vector4<float> const& clearColor)
{
    mClearColor = clearColor;
}

inline void DX11Engine::SetClearDepth(float clearDepth)
{
    mClearDepth = clearDepth;
}

inline void DX11Engine::SetClearStencil(unsigned int clearStencil)
{
    mClearStencil = clearStencil;
}

inline Vector4<float> const& DX11Engine::GetClearColor() const
{
    return mClearColor;
}

inline float DX11Engine::GetClearDepth() const
{
    return mClearDepth;
}

inline unsigned int DX11Engine::GetClearStencil() const
{
    return mClearStencil;
}

inline void DX11Engine::SetDefaultBlendState()
{
    SetBlendState(mDefaultBlendState);
}

inline std::shared_ptr<BlendState> const&
DX11Engine::GetDefaultBlendState() const
{
    return mDefaultBlendState;
}

inline std::shared_ptr<BlendState> const& DX11Engine::GetBlendState() const
{
    return mActiveBlendState;
}

inline void DX11Engine::SetDefaultDepthStencilState()
{
    SetDepthStencilState(mDefaultDepthStencilState);
}

inline std::shared_ptr<DepthStencilState> const&
DX11Engine::GetDefaultDepthStencilState() const
{
    return mDefaultDepthStencilState;
}

inline std::shared_ptr<DepthStencilState> const&
DX11Engine::GetDepthStencilState() const
{
    return mActiveDepthStencilState;
}

inline void DX11Engine::SetDefaultRasterizerState()
{
    SetRasterizerState(mDefaultRasterizerState);
}

inline std::shared_ptr<RasterizerState> const&
DX11Engine::GetDefaultRasterizerState() const
{
    return mDefaultRasterizerState;
}

inline std::shared_ptr<RasterizerState> const&
DX11Engine::GetRasterizerState() const
{
    return mActiveRasterizerState;
}

inline void DX11Engine::SetDefaultFont()
{
    SetFont(mDefaultFont);
}

inline std::shared_ptr<Font> const& DX11Engine::GetDefaultFont() const
{
    return mDefaultFont;
}

inline std::shared_ptr<Font> const& DX11Engine::GetFont() const
{
    return mActiveFont;
}

inline void DX11Engine::AllowOcclusionQuery(bool allow)
{
    mAllowOcclusionQuery = allow;
}

inline void DX11Engine::WarnOnNonemptyBridges(bool warn)
{
    mWarnOnNonemptyBridges = warn;
}

inline bool DX11Engine::Unbind(std::shared_ptr<GraphicsObject> const& object)
{
    return Unbind(object.get());
}

inline bool DX11Engine::Unbind(std::shared_ptr<DrawTarget> const& target)
{
    return Unbind(target.get());
}


}
