// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngineDEF.h>
#include <Windows.h>
#include <GPUPerfAPITypes.h>
#include <GPUPerfAPIFunctionTypes.h>
#include <functional>
#include <set>
#include <vector>

// You must run MSVS in administrator mode for the AMD performance system to
// initialize.  The system needs access to the high-performance counters,
// which requires administrative privileges.

namespace gte
{

// Counters available in GPUPerfAPI-2.11.739.0.
enum GTE_IMPEXP AMDCounter
{
    CBMemRead,
    CBMemWritten,
    CBSlowPixelPct,
    ClippedPrims,
    CSALUStalledByLDS,
    CSALUBusy,
    CSALUPacking,
    CSALUInsts,
    CSALUFetchRatio,
    CSBusy,
    CSCacheHit,
    CSCompletePath,
    CSFastPath,
    CSFetchInsts,
    CSFetchSize,
    CSGDSInsts,
    CSLDSBankConflict,
    CSLDSBankConflictAccess,
    CSLDSInsts,
    CSLDSFetchInsts,
    CSLDSWriteInsts,
    CSMemUnitBusy,
    CSMemUnitStalled,
    CSPathUtilization,
    CSSALUBusy,
    CSSALUInsts,
    CSSFetchInsts,
    CSTexBusy,
    CSThreadGroups,
    CSThreads,
    CSVALUBusy,
    CSVALUInsts,
    CSVALUUtilization,
    CSVFetchInsts,
    CSVWriteInsts,
    CSWavefronts,
    CSWriteInsts,
    CSWriteSize,
    CSWriteUnitStalled,
    CulledPrims,
    DepthStencilTestBusy,
    DSALUBusy,
    DSALUEfficiency,
    DSALUInstCount,
    DSALUTexRatio,
    DSBusy,
    DSTexBusy,
    DSTexInstCount,
    DSVerticesIn,
    GPUBusy,
    GPUTime,
    GSALUBusy,
    GSALUEfficiency,
    GSALUInstCount,
    GSALUTexRatio,
    GSBusy,
    GSExportPct,
    GSPrimsIn,
    GSTexBusy,
    GSTexInstCount,
    GSVerticesOut,
    HiZQuadsCulled,
    HiZTilesAccepted,
    HSALUBusy,
    HSALUEfficiency,
    HSALUInstCount,
    HSALUTexRatio,
    HSBusy,
    HSTexBusy,
    HSTexInstCount,
    HSPatches,
    InterpBusy,
    PAPixelsPerTriangle,
    PAStalledOnRasterizer,
    Pct64SlowTexels,
    Pct128SlowTexels,
    PctCompressedTexels,
    PctDepthTexels,
    PctInterlacedTexels,
    PctTex1D,
    PctTex1DArray,
    PctTex2D,
    PctTex2DArray,
    PctTex2DMSAA,
    PctTex2DMSAAArray,
    PctTex3D,
    PctTexCube,
    PctTexCubeArray,
    PctUncompressedTexels,
    PctVertex64SlowTexels,
    PctVertex128SlowTexels,
    PctVertexTexels,
    PostZQuads,
    PostZSamplesFailingS,
    PostZSamplesFailingZ,
    PostZSamplesPassing,
    PreZQuadsCulled,
    PreZSamplesFailingS,
    PreZSamplesFailingZ,
    PreZSamplesPassing,
    PreZTilesDetailCulled,
    PrimitiveAssemblyBusy,
    PrimitivesIn,
    PSALUBusy,
    PSALUEfficiency,
    PSALUInstCount,
    PSALUTexRatio,
    PSBusy,
    PSExportStalls,
    PSPixelsIn,
    PSPixelsOut,
    PSTexBusy,
    PSTexInstCount,
    ShaderBusy,
    ShaderBusyCS,
    ShaderBusyDS,
    ShaderBusyGS,
    ShaderBusyHS,
    ShaderBusyPS,
    ShaderBusyVS,
    TessellatorBusy,
    TexAveAnisotropy,
    TexCacheStalled,
    TexCostOfFiltering,
    TexelFetchCount,
    TexMemBytesRead,
    TexMissRate,
    TexTriFilteringPct,
    TexUnitBusy,
    TexVolFilteringPct,
    VertexMemFetched,
    VertexMemFetchedCost,
    VSALUBusy,
    VSALUEfficiency,
    VSALUInstCount,
    VSALUTexRatio,
    VSBusy,
    VSTexBusy,
    VSTexInstCount,
    VSVerticesIn,
    ZUnitStalled,
    MAX_AMD_COUNTERS
};

class GTE_IMPEXP AMDPerformance
{
public:
    // Construction and destruction.
    ~AMDPerformance();
    AMDPerformance(void* dx11Device);

    // Specify the performance counters you want to be measured during a
    // recording session.  The SetAllCounters() queries for the counters
    // supported by the dx11Device, so will vary depending on graphics card.
    bool SetCounter(AMDCounter counter);
    bool SetAllCounters();
    bool ClearCounter(AMDCounter counter);
    bool ClearCounters();

    // Provide a callback to obtain extra information during a session.
    bool Register(GPA_LoggingCallbackPtrType listener);

    // Support for profiling lambdas or functions.  The first Profile
    // may be used for a lambda expression.  The second Profile may be
    // used for a collection of functions.
    bool Profile(std::function<void()> const& myFunction);
    bool Profile(std::vector<std::function<void()>> const& myFunctions);

    // Average measurements for a collection of profiles.  GetAverage allows
    // you to access the measurements during application run time.  SaveAverage
    // calls GetAverage and writes the results to a spreadsheet.  The first
    // column contains the counter names.  The remaining columns are the
    // counter values, each column corresponding to a function provided as
    // input to Profile(...).

    struct GTE_IMPEXP Measurement
    {
        AMDCounter counter;
        gpa_uint32 index;
        GPA_Type type;
        union
        {
            gpa_float32 f32;
            gpa_float64 f64;
            gpa_uint32  u32;
            gpa_uint64  u64;
        }
        value;
    };

    inline size_t GetNumProfileCalls() const;
    bool GetAverage(std::vector<std::vector<Measurement>>& measurements) const;
    bool SaveAverage(std::string const& filename);
    void ClearMeasurements();

    // Information about the counters support by the device passed to
    // OpenDevice.  The 'counter' input is relative to the mNames[] array;
    // 0 <= counter < GetMaxCounters().  SaveCounterInformation saves the
    // counter information to the specified file; call it after the
    // OpenDevice(device) call.
    inline gpa_uint32 GetMaxCounters() const;
    inline std::string GetName(unsigned int counter) const;
    inline std::string GetDescription(unsigned int counter) const;
    inline GPA_Type GetDataType(unsigned int counter) const;
    inline GPA_Usage_Type GetAccess(unsigned int counter) const;
    bool SaveCounterInformation(std::string const& filename);

private:
    // Support for measuring and reporting performance.  The return value
    // is the session number, which is positive on success.  It is zero
    // if the recording failed.
    gpa_uint32 RecordSession(
        std::vector<std::function<void()>> const& profileFunctions);

    // The handle for the AMD performance DLL and the functions provided by the DLL.
    HMODULE mModule;
    std::function<GPA_Status(GPA_Logging_Type, GPA_LoggingCallbackPtrType)> GPA_RegisterLoggingCallback;
    std::function<GPA_Status()> GPA_Initialize;
    std::function<GPA_Status()> GPA_Destroy;
    std::function<GPA_Status(void*)> GPA_OpenContext;
    std::function<GPA_Status()> GPA_CloseContext;
    std::function<GPA_Status(void*)> GPA_SelectContext;
    std::function<GPA_Status(gpa_uint32*)> GPA_GetNumCounters;
    std::function<GPA_Status(gpa_uint32, const char**)> GPA_GetCounterName;
    std::function<GPA_Status(gpa_uint32, const char**)> GPA_GetCounterDescription;
    std::function<GPA_Status(gpa_uint32, GPA_Type*)> GPA_GetCounterDataType;
    std::function<GPA_Status(gpa_uint32, GPA_Usage_Type*)> GPA_GetCounterUsageType;
    std::function<GPA_Status(GPA_Type, const char**)> GPA_GetDataTypeAsStr;
    std::function<GPA_Status(GPA_Usage_Type, const char**)> GPA_GetUsageTypeAsStr;
    std::function<GPA_Status(gpa_uint32)> GPA_EnableCounter;
    std::function<GPA_Status(gpa_uint32)> GPA_DisableCounter;
    std::function<GPA_Status(gpa_uint32*)> GPA_GetEnabledCount;
    std::function<GPA_Status(gpa_uint32, gpa_uint32*)> GPA_GetEnabledIndex;
    std::function<GPA_Status(gpa_uint32)> GPA_IsCounterEnabled;
    std::function<GPA_Status(const char*)> GPA_EnableCounterStr;
    std::function<GPA_Status(const char*)> GPA_DisableCounterStr;
    std::function<GPA_Status()> GPA_EnableAllCounters;
    std::function<GPA_Status()> GPA_DisableAllCounters;
    std::function<GPA_Status(const char*, gpa_uint32*)> GPA_GetCounterIndex;
    std::function<GPA_Status(gpa_uint32*)> GPA_GetPassCount;
    std::function<GPA_Status(gpa_uint32*)> GPA_BeginSession;
    std::function<GPA_Status()> GPA_EndSession;
    std::function<GPA_Status()> GPA_BeginPass;
    std::function<GPA_Status()> GPA_EndPass;
    std::function<GPA_Status(gpa_uint32)> GPA_BeginSample;
    std::function<GPA_Status()> GPA_EndSample;
    std::function<GPA_Status(gpa_uint32, gpa_uint32*)> GPA_GetSampleCount;
    std::function<GPA_Status(bool*, gpa_uint32, gpa_uint32)> GPA_IsSampleReady;
    std::function<GPA_Status(bool*, gpa_uint32)> GPA_IsSessionReady;
    std::function<GPA_Status(gpa_uint32, gpa_uint32, gpa_uint32, gpa_uint64*)> GPA_GetSampleUInt64;
    std::function<GPA_Status(gpa_uint32, gpa_uint32, gpa_uint32, gpa_uint32*)> GPA_GetSampleUInt32;
    std::function<GPA_Status(gpa_uint32, gpa_uint32, gpa_uint32, gpa_float64*)> GPA_GetSampleFloat64;
    std::function<GPA_Status(gpa_uint32, gpa_uint32, gpa_uint32, gpa_float32*)> GPA_GetSampleFloat32;
    std::function<const char*(GPA_Status)> GPA_GetStatusAsStr;

    // Information about the performance counters for the device specified in
    // the OpenDevice(...) call.
    gpa_uint32 mMaxCounters;
    std::vector<std::string> mNames;
    std::vector<std::string> mDescriptions;
    std::vector<GPA_Type> mDataTypes;
    std::vector<GPA_Usage_Type> mUsageTypes;
    std::vector<AMDCounter> mSupportedCounters;
    static std::string const msCounterName[MAX_AMD_COUNTERS];

    // The currently active performance counters.
    std::set<AMDCounter> mActiveCounters;
    std::vector<std::vector<Measurement>> mMeasurements;
    size_t mNumProfileCalls;

    enum
    {
        // The calls to GPA_IsSessionReady are executed as many times as you
        // wish until the function returns a 'ready' value of 'true'.  It is
        // not clear from the documentation whether or not this function will
        // eventually succeed.  Thus, the loop is limited by MAX_QUERIES.  We
        // should determine whether this number needs to be increased.
        MAX_QUERIES = 100
    };

    // Strings for the GPA_ enumerations in AMD's source code.  If we change
    // GPA library version, we will need to modify the number of enums hard
    // coded here.
    enum
    {
        MAX_AMD_STATUS_NAMES = GPA_STATUS_ERROR_CATALYST_VER_UNSUPPORTED_SI + 1,
        MAX_AMD_TYPE_NAMES = GPA_TYPE__LAST,
        MAX_AMD_USAGE_TYPE_NAMES = GPA_USAGE_TYPE__LAST
    };

    static std::string const msStatusName[MAX_AMD_STATUS_NAMES];
    static std::string const msTypeName[MAX_AMD_TYPE_NAMES];
    static std::string const msUsageTypeName[MAX_AMD_USAGE_TYPE_NAMES];
};

//----------------------------------------------------------------------------
inline size_t AMDPerformance::GetNumProfileCalls() const
{
    return mNumProfileCalls;
}
//----------------------------------------------------------------------------
inline gpa_uint32 AMDPerformance::GetMaxCounters() const
{
    return mMaxCounters;
}
//----------------------------------------------------------------------------
inline std::string AMDPerformance::GetName(unsigned int counter) const
{
    return (counter < mMaxCounters ? mNames[counter] : "");
}
//----------------------------------------------------------------------------
inline std::string AMDPerformance::GetDescription(unsigned int counter) const
{
    return (counter < mMaxCounters ? mDescriptions[counter] : "");
}
//----------------------------------------------------------------------------
inline GPA_Type AMDPerformance::GetDataType(unsigned int counter) const
{
    return (counter < mMaxCounters ? mDataTypes[counter] : GPA_TYPE__LAST);
}
//----------------------------------------------------------------------------
inline GPA_Usage_Type AMDPerformance::GetAccess(unsigned int counter) const
{
    return (counter < mMaxCounters ? mUsageTypes[counter]
        : GPA_USAGE_TYPE__LAST);
}
//----------------------------------------------------------------------------

}
