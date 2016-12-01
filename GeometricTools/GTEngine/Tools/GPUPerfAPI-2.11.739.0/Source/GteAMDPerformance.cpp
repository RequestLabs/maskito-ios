// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GteAMDPerformance.h>
#include <Applications/GteEnvironment.h>
#include <LowLevel/GteLogger.h>
#include <fstream>
#include <iomanip>
using namespace gte;

#define AMD_GET_FUNCTION(name)\
    GPA_##name = (GPA_##name##PtrType)GetProcAddress(mModule, "GPA_"#name); \
    LogAssert(nullptr != GPA_##name, "Null function pointer.")

#define AMD_REPORT_ERROR(name,status)\
    LogError(#name##" failed: " + msStatusName[status])

#define AMD_REPORT_INFORMATION(name,status)\
if (GPA_STATUS_OK != status)\
{\
    LogInformation(#name##" failed: " + msStatusName[status]); \
}

//----------------------------------------------------------------------------
AMDPerformance::~AMDPerformance()
{
    GPA_Status status = GPA_CloseContext();
    if (GPA_STATUS_OK != status)
    {
        LogError("Failed to close device context.");
    }

    status = GPA_Destroy();
    if (GPA_STATUS_OK != status)
    {
        LogError("Failed to terminate AMD performance library.");
    }

    if (mModule)
    {
        FreeLibrary(mModule);
    }
}
//----------------------------------------------------------------------------
AMDPerformance::AMDPerformance(void* dx11Device)
    :
    mModule(nullptr),
    mMaxCounters(0),
    mNumProfileCalls(0)
{
    Environment env;
    std::string path = env.GetVariable("GEOMETRIC_TOOLS");
    if ("" == path)
    {
        LogError("Cannot find environment variable GEOMETRIC_TOOLS.");
        return;
    }
    std::wstring wpath = Environment::Convert(path);

    std::string arch = env.GetVariable("PROCESSOR_ARCHITECTURE");
    if ("" == arch)
    {
        LogError("Cannot find environment variable PROCESSOR_ARCHITECTURE.");
        return;
    }

    if ("AMD64" == arch)
    {
        wpath += L"\\x64\\GPUPerfAPIDX11-x64.dll";
    }
    else
    {
        wpath += L"\\x86\\GPUPerfAPIDX11.dll";
    }

    mModule = LoadLibrary(wpath.c_str());
    if (nullptr == mModule)
    {
        LogError("Failed to load the AMD performance library.");
        return;
    }

    AMD_GET_FUNCTION(RegisterLoggingCallback);
    AMD_GET_FUNCTION(Initialize);
    AMD_GET_FUNCTION(Destroy);
    AMD_GET_FUNCTION(OpenContext);
    AMD_GET_FUNCTION(CloseContext);
    AMD_GET_FUNCTION(SelectContext);
    AMD_GET_FUNCTION(GetNumCounters);
    AMD_GET_FUNCTION(GetCounterName);
    AMD_GET_FUNCTION(GetCounterDescription);
    AMD_GET_FUNCTION(GetCounterDataType);
    AMD_GET_FUNCTION(GetCounterUsageType);
    AMD_GET_FUNCTION(GetDataTypeAsStr);
    AMD_GET_FUNCTION(GetUsageTypeAsStr);
    AMD_GET_FUNCTION(EnableCounter);
    AMD_GET_FUNCTION(DisableCounter);
    AMD_GET_FUNCTION(GetEnabledCount);
    AMD_GET_FUNCTION(GetEnabledIndex);
    AMD_GET_FUNCTION(IsCounterEnabled);
    AMD_GET_FUNCTION(EnableCounterStr);
    AMD_GET_FUNCTION(DisableCounterStr);
    AMD_GET_FUNCTION(EnableAllCounters);
    AMD_GET_FUNCTION(DisableAllCounters);
    AMD_GET_FUNCTION(GetCounterIndex);
    AMD_GET_FUNCTION(GetPassCount);
    AMD_GET_FUNCTION(BeginSession);
    AMD_GET_FUNCTION(EndSession);
    AMD_GET_FUNCTION(BeginPass);
    AMD_GET_FUNCTION(EndPass);
    AMD_GET_FUNCTION(BeginSample);
    AMD_GET_FUNCTION(EndSample);
    AMD_GET_FUNCTION(GetSampleCount);
    AMD_GET_FUNCTION(IsSampleReady);
    AMD_GET_FUNCTION(IsSessionReady);
    AMD_GET_FUNCTION(GetSampleUInt64);
    AMD_GET_FUNCTION(GetSampleUInt32);
    AMD_GET_FUNCTION(GetSampleFloat64);
    AMD_GET_FUNCTION(GetSampleFloat32);
    AMD_GET_FUNCTION(GetStatusAsStr);

    GPA_Status status = GPA_Initialize();
    if (GPA_STATUS_OK != status)
    {
        // If this call fails, you need to run MSVS as Administrator.  This is
        // required so you can access the high-performance counters.
        LogInformation("GPA_Initialize failed: " + msStatusName[status]);
        LogError("Did you run MSVS in administrator mode?");
        return;
    }

    // This function creates and destroys an ID3D11Query object.
    status = GPA_OpenContext(dx11Device);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_OpenContext, status);
        return;
    }

    status = GPA_GetNumCounters(&mMaxCounters);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_GetNumCounters, status);
        return;
    }

    mNames.resize(mMaxCounters);
    mDescriptions.resize(mMaxCounters);
    mDataTypes.resize(mMaxCounters);
    mUsageTypes.resize(mMaxCounters);

    for (gpa_uint32 i = 0; i < mMaxCounters; ++i)
    {
        char const* temp = nullptr;
        status = GPA_GetCounterName(i, &temp);
        AMD_REPORT_INFORMATION(GPA_GetCounterName, status);
        if (nullptr != temp)
        {
            mNames[i] = temp;
        }

        status = GPA_GetCounterDescription(i, &temp);
        AMD_REPORT_INFORMATION(GPA_GetCounterDescription, status);
        if (temp)
        {
            mDescriptions[i] = temp;
        }

        status = GPA_GetCounterDataType(i, &mDataTypes[i]);
        AMD_REPORT_INFORMATION(GPA_GetCounterDataType, status);

        status = GPA_GetCounterUsageType(i, &mUsageTypes[i]);
        AMD_REPORT_INFORMATION(GPA_GetCounterUsageType, status);

        // This code should not fail for GPUPerfAPI-2.11.739.0.  It is
        // included here for when we upgrade to the next version, and AMD
        // modifies counter names or removes counters.
        int j;
        for (j = 0; j < MAX_AMD_COUNTERS; ++j)
        {
            if (mNames[i] == msCounterName[j])
            {
                mSupportedCounters.push_back(static_cast<AMDCounter>(j));
                break;
            }
        }
        if (j == MAX_AMD_COUNTERS)
        {
            // The name cannot be found in the AMD counter enumerations.  This
            // means we have incorrectly typed an enumeration member (fix it).
            LogError("AMD counter name not found.");
        }
    }
}
//----------------------------------------------------------------------------
bool AMDPerformance::SetCounter(AMDCounter counter)
{
    char const* name = msCounterName[counter].c_str();
    gpa_uint32 index;
    GPA_Status status = GPA_GetCounterIndex(name, &index);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_GetCounterIndex, status);
        return false;
    }

    status = GPA_EnableCounter(index);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_EnableCounter, status);
        return false;
    }

    mActiveCounters.insert(counter);
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::SetAllCounters()
{
    for (auto counter : mSupportedCounters)
    {
        if (!SetCounter(counter))
        {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::ClearCounter(AMDCounter counter)
{
    if (mActiveCounters.find(counter) == mActiveCounters.end())
    {
        // The counter is not active.
        return false;
    }

    char const* name = msCounterName[counter].c_str();
    gpa_uint32 index;
    GPA_Status status = GPA_GetCounterIndex(name, &index);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_GetCounterIndex, status);
        return false;
    }

    status = GPA_DisableCounter(index);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_DisableCounter, status);
        return false;
    }

    mActiveCounters.erase(counter);
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::ClearCounters()
{
    mActiveCounters.clear();
    GPA_Status status = GPA_DisableAllCounters();
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_DisableAllCounters, status);
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::Register(GPA_LoggingCallbackPtrType listener)
{
    GPA_Status status = GPA_RegisterLoggingCallback(GPA_LOGGING_ALL, listener);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_RegisterLoggingCallback, status);
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::Profile(std::function<void()> const& myFunction)
{
    std::vector<std::function<void()>> myFunctions(1);
    myFunctions[0] = myFunction;
    return Profile(myFunctions);
}
//----------------------------------------------------------------------------
bool AMDPerformance::Profile(
    std::vector<std::function<void()>> const& myFunctions)
{
    if (myFunctions.size() == 0)
    {
        LogInformation("No functions have been specified.");
        return false;
    }

    if (mActiveCounters.size() == 0)
    {
        LogInformation("No performance counters have been specified.");
        return false;
    }


    GPA_Status status;
    if (mMeasurements.size() != myFunctions.size())
    {
        // A resize is treated as if Profile(...) has been called for the
        // first time.
        mNumProfileCalls = 0;
        mMeasurements.resize(myFunctions.size());
        for (auto& current : mMeasurements)
        {
            current.resize(mActiveCounters.size());
            int i = 0;
            for (auto counter : mActiveCounters)
            {
                current[i].counter = counter;
                char const* name = msCounterName[counter].c_str();
                status = GPA_GetCounterIndex(name, &current[i].index);
                AMD_REPORT_INFORMATION(GPA_GetCounterIndex, status);
                status = GPA_GetCounterDataType(current[i].index,
                    &current[i].type);
                AMD_REPORT_INFORMATION(GPA_GetCounterDataType, status);
                current[i].value.u64 = 0;
                ++i;
            }
        }
    }

    gpa_uint32 session = RecordSession(myFunctions);
    if (session == 0)
    {
        LogInformation("RecordSession(myFunctions) failed.");
        return false;
    }

    gpa_uint32 sample = 0;
    for (auto& current : mMeasurements)
    {
        for (auto& m : current)
        {
            if (GPA_TYPE_FLOAT32 == m.type)
            {
                gpa_float32 value = 0.0f;
                status = GPA_GetSampleFloat32(session, sample, m.index,
                    &m.value.f32);
                AMD_REPORT_INFORMATION(GPA_GetSampleFloat32, status);
                m.value.f32 += value;
            }
            else if (GPA_TYPE_FLOAT64 == m.type)
            {
                gpa_float64 value = 0.0;
                status = GPA_GetSampleFloat64(session, sample, m.index,
                    &value);
                AMD_REPORT_INFORMATION(GPA_GetSampleFloat64, status);
                m.value.f64 += value;
            }
            else if (GPA_TYPE_UINT32 == m.type)
            {
                gpa_uint32 value = 0;
                status = GPA_GetSampleUInt32(session, sample, m.index,
                    &value);
                AMD_REPORT_INFORMATION(GPA_GetSampleUInt32, status);
                m.value.u32 += value;
            }
            else if (GPA_TYPE_UINT64 == m.type)
            {
                gpa_uint64 value = 0;
                status = GPA_GetSampleUInt64(session, sample, m.index,
                    &value);
                AMD_REPORT_INFORMATION(GPA_GetSampleUInt64, status);
                m.value.u64 += value;
            }
        }
        ++sample;
    }
    ++mNumProfileCalls;
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::GetAverage(
    std::vector<std::vector<Measurement>>& measurements) const
{
    if (mNumProfileCalls == 0)
    {
        measurements.clear();
        LogInformation("No profiling calls have been made.");
        return false;
    }

    if (mMeasurements.size() == 0)
    {
        measurements.clear();
        LogInformation("No measurements have been made.");
        return false;
    }

    measurements = mMeasurements;

    // Compute the averages of the measurements and return them.
    for (auto& current : measurements)
    {
        for (auto& m : current)
        {
            if (GPA_TYPE_FLOAT32 == m.type)
            {
                m.value.f32 /= static_cast<gpa_float32>(mNumProfileCalls);
            }
            else if (GPA_TYPE_FLOAT64 == m.type)
            {
                m.value.f64 /= static_cast<gpa_float64>(mNumProfileCalls);
            }
            else if (GPA_TYPE_UINT32 == m.type)
            {
                m.value.u32 /= static_cast<gpa_uint32>(mNumProfileCalls);
            }
            else if (GPA_TYPE_UINT64 == m.type)
            {
                m.value.u64 /= static_cast<gpa_uint64>(mNumProfileCalls);
            }
        }
    }
    return true;
}
//----------------------------------------------------------------------------
bool AMDPerformance::SaveAverage(std::string const& filename)
{
    std::vector<std::vector<Measurement>> measurements;
    if (!GetAverage(measurements))
    {
        return false;
    }

    if ("" == filename)
    {
        LogError("Invalid filename " + filename);
        return false;
    }

    std::ofstream output(filename);
    if (!output)
    {
        LogError("Failed to open file " + filename);
        return false;
    }

    // Column titles in the spreadsheet.
    output << "Counter [numCalls =" << mNumProfileCalls << "]";
    int const numProfiledFunctions = static_cast<int>(measurements.size());
    for (int i = 0; i < numProfiledFunctions; ++i)
    {
        output << ",Profile" << i;
    }
    output << std::endl;

    int const numCounters = static_cast<int>(measurements[0].size());
    for (int j = 0; j < numCounters; ++j)
    {
        char const* name = nullptr;
        GPA_Status status = GPA_GetCounterName(measurements[0][j].index,
            &name);
        AMD_REPORT_INFORMATION(GPA_GetCounterName, status);
        output << name;
        for (int i = 0; i < numProfiledFunctions; ++i)
        {
            auto& m = measurements[i][j];
            if (GPA_TYPE_FLOAT32 == m.type)
            {
                output << "," << m.value.f32;
            }
            else if (GPA_TYPE_FLOAT64 == m.type)
            {
                output << "," << m.value.f64;
            }
            else if (GPA_TYPE_UINT32 == m.type)
            {
                output << "," << m.value.u32;
            }
            else if (GPA_TYPE_UINT64 == m.type)
            {
                output << "," << m.value.u64;
            }
        }
        output << std::endl;
    }

    output.close();
    return true;
}
//----------------------------------------------------------------------------
void AMDPerformance::ClearMeasurements()
{
    mNumProfileCalls = 0;
    for (auto& current : mMeasurements)
    {
        for (auto& m : current)
        {
            m.value.u64 = 0;
        }
    }
}
//----------------------------------------------------------------------------
bool AMDPerformance::SaveCounterInformation(std::string const& filename)
{
    if ("" == filename)
    {
        LogError("Invalid filename " + filename);
        return false;
    }

    std::ofstream output(filename);
    if (!output)
    {
        LogError("Failed to open file " + filename);
        return false;
    }

    for (gpa_uint32 i = 0; i < mMaxCounters; ++i)
    {
        output << std::setw(2) << i << ": "
            << mNames[i].c_str() << std::endl;
        output << "     "
            << mDescriptions[i].c_str() << std::endl;
        output << "     " <<
            msTypeName[mDataTypes[i]].c_str() << std::endl;
        output << "     "
            << msUsageTypeName[mUsageTypes[i]].c_str() << std::endl;
    }

    output.close();
    return true;
}
//----------------------------------------------------------------------------
gpa_uint32 AMDPerformance::RecordSession(
    std::vector<std::function<void()>> const& profileFunctions)
{
    if (mActiveCounters.empty())
    {
        LogInformation("No performance counters have been specified.");
        return 0;
    }

    gpa_uint32 session = 0;
    GPA_Status status = GPA_BeginSession(&session);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_BeginSession, status);
        return 0;
    }

    gpa_uint32 numPasses = 0;
    status = GPA_GetPassCount(&numPasses);
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_GetPassCount, status);
        return 0;
    }

    for (gpa_uint32 pass = 0; pass < numPasses; ++pass)
    {
        status = GPA_BeginPass();
        if (GPA_STATUS_OK != status)
        {
            AMD_REPORT_INFORMATION(GPA_BeginPass, status);
        }

        gpa_uint32 sample = 0;
        for (auto profileFunction : profileFunctions)
        {
            status = GPA_BeginSample(sample);
            if (GPA_STATUS_OK != status)
            {
                AMD_REPORT_INFORMATION(GPA_BeginSample, status);
            }

            profileFunction();

            status = GPA_EndSample();
            if (GPA_STATUS_OK != status)
            {
                AMD_REPORT_INFORMATION(GPA_EndSample, status);
            }

            ++sample;
        }

        status = GPA_EndPass();
        if (GPA_STATUS_OK != status)
        {
            AMD_REPORT_INFORMATION(GPA_EndPass, status);
        }
    }

    status = GPA_EndSession();
    if (GPA_STATUS_OK != status)
    {
        AMD_REPORT_ERROR(GPA_EndSession, status);
        return 0;
    }

    bool ready = false;
    for (int i = 0; i < MAX_QUERIES; ++i)
    {
        status = GPA_IsSessionReady(&ready, session);
        if (GPA_STATUS_OK != status)
        {
            AMD_REPORT_INFORMATION(GPA_IsSessionReady, status);
        }
        if (ready)
        {
            break;
        }
    }

    return (ready ? session : 0);
}
//----------------------------------------------------------------------------


#define AMD_MAKE_NAME(counter) #counter

std::string const AMDPerformance::msCounterName[MAX_AMD_COUNTERS] =
{
    AMD_MAKE_NAME(CBMemRead),
    AMD_MAKE_NAME(CBMemWritten),
    AMD_MAKE_NAME(CBSlowPixelPct),
    AMD_MAKE_NAME(ClippedPrims),
    AMD_MAKE_NAME(CSALUStalledByLDS),
    AMD_MAKE_NAME(CSALUBusy),
    AMD_MAKE_NAME(CSALUPacking),
    AMD_MAKE_NAME(CSALUInsts),
    AMD_MAKE_NAME(CSALUFetchRatio),
    AMD_MAKE_NAME(CSBusy),
    AMD_MAKE_NAME(CSCacheHit),
    AMD_MAKE_NAME(CSCompletePath),
    AMD_MAKE_NAME(CSFastPath),
    AMD_MAKE_NAME(CSFetchInsts),
    AMD_MAKE_NAME(CSFetchSize),
    AMD_MAKE_NAME(CSGDSInsts),
    AMD_MAKE_NAME(CSLDSBankConflict),
    AMD_MAKE_NAME(CSLDSBankConflictAccess),
    AMD_MAKE_NAME(CSLDSInsts),
    AMD_MAKE_NAME(CSLDSFetchInsts),
    AMD_MAKE_NAME(CSLDSWriteInsts),
    AMD_MAKE_NAME(CSMemUnitBusy),
    AMD_MAKE_NAME(CSMemUnitStalled),
    AMD_MAKE_NAME(CSPathUtilization),
    AMD_MAKE_NAME(CSSALUBusy),
    AMD_MAKE_NAME(CSSALUInsts),
    AMD_MAKE_NAME(CSSFetchInsts),
    AMD_MAKE_NAME(CSTexBusy),
    AMD_MAKE_NAME(CSThreadGroups),
    AMD_MAKE_NAME(CSThreads),
    AMD_MAKE_NAME(CSVALUBusy),
    AMD_MAKE_NAME(CSVALUInsts),
    AMD_MAKE_NAME(CSVALUUtilization),
    AMD_MAKE_NAME(CSVFetchInsts),
    AMD_MAKE_NAME(CSVWriteInsts),
    AMD_MAKE_NAME(CSWavefronts),
    AMD_MAKE_NAME(CSWriteInsts),
    AMD_MAKE_NAME(CSWriteSize),
    AMD_MAKE_NAME(CSWriteUnitStalled),
    AMD_MAKE_NAME(CulledPrims),
    AMD_MAKE_NAME(DepthStencilTestBusy),
    AMD_MAKE_NAME(DSALUBusy),
    AMD_MAKE_NAME(DSALUEfficiency),
    AMD_MAKE_NAME(DSALUInstCount),
    AMD_MAKE_NAME(DSALUTexRatio),
    AMD_MAKE_NAME(DSBusy),
    AMD_MAKE_NAME(DSTexBusy),
    AMD_MAKE_NAME(DSTexInstCount),
    AMD_MAKE_NAME(DSVerticesIn),
    AMD_MAKE_NAME(GPUBusy),
    AMD_MAKE_NAME(GPUTime),
    AMD_MAKE_NAME(GSALUBusy),
    AMD_MAKE_NAME(GSALUEfficiency),
    AMD_MAKE_NAME(GSALUInstCount),
    AMD_MAKE_NAME(GSALUTexRatio),
    AMD_MAKE_NAME(GSBusy),
    AMD_MAKE_NAME(GSExportPct),
    AMD_MAKE_NAME(GSPrimsIn),
    AMD_MAKE_NAME(GSTexBusy),
    AMD_MAKE_NAME(GSTexInstCount),
    AMD_MAKE_NAME(GSVerticesOut),
    AMD_MAKE_NAME(HiZQuadsCulled),
    AMD_MAKE_NAME(HiZTilesAccepted),
    AMD_MAKE_NAME(HSALUBusy),
    AMD_MAKE_NAME(HSALUEfficiency),
    AMD_MAKE_NAME(HSALUInstCount),
    AMD_MAKE_NAME(HSALUTexRatio),
    AMD_MAKE_NAME(HSBusy),
    AMD_MAKE_NAME(HSTexBusy),
    AMD_MAKE_NAME(HSTexInstCount),
    AMD_MAKE_NAME(HSPatches),
    AMD_MAKE_NAME(InterpBusy),
    AMD_MAKE_NAME(PAPixelsPerTriangle),
    AMD_MAKE_NAME(PAStalledOnRasterizer),
    AMD_MAKE_NAME(Pct64SlowTexels),
    AMD_MAKE_NAME(Pct128SlowTexels),
    AMD_MAKE_NAME(PctCompressedTexels),
    AMD_MAKE_NAME(PctDepthTexels),
    AMD_MAKE_NAME(PctInterlacedTexels),
    AMD_MAKE_NAME(PctTex1D),
    AMD_MAKE_NAME(PctTex1DArray),
    AMD_MAKE_NAME(PctTex2D),
    AMD_MAKE_NAME(PctTex2DArray),
    AMD_MAKE_NAME(PctTex2DMSAA),
    AMD_MAKE_NAME(PctTex2DMSAAArray),
    AMD_MAKE_NAME(PctTex3D),
    AMD_MAKE_NAME(PctTexCube),
    AMD_MAKE_NAME(PctTexCubeArray),
    AMD_MAKE_NAME(PctUncompressedTexels),
    AMD_MAKE_NAME(PctVertex64SlowTexels),
    AMD_MAKE_NAME(PctVertex128SlowTexels),
    AMD_MAKE_NAME(PctVertexTexels),
    AMD_MAKE_NAME(PostZQuads),
    AMD_MAKE_NAME(PostZSamplesFailingS),
    AMD_MAKE_NAME(PostZSamplesFailingZ),
    AMD_MAKE_NAME(PostZSamplesPassing),
    AMD_MAKE_NAME(PreZQuadsCulled),
    AMD_MAKE_NAME(PreZSamplesFailingS),
    AMD_MAKE_NAME(PreZSamplesFailingZ),
    AMD_MAKE_NAME(PreZSamplesPassing),
    AMD_MAKE_NAME(PreZTilesDetailCulled),
    AMD_MAKE_NAME(PrimitiveAssemblyBusy),
    AMD_MAKE_NAME(PrimitivesIn),
    AMD_MAKE_NAME(PSALUBusy),
    AMD_MAKE_NAME(PSALUEfficiency),
    AMD_MAKE_NAME(PSALUInstCount),
    AMD_MAKE_NAME(PSALUTexRatio),
    AMD_MAKE_NAME(PSBusy),
    AMD_MAKE_NAME(PSExportStalls),
    AMD_MAKE_NAME(PSPixelsIn),
    AMD_MAKE_NAME(PSPixelsOut),
    AMD_MAKE_NAME(PSTexBusy),
    AMD_MAKE_NAME(PSTexInstCount),
    AMD_MAKE_NAME(ShaderBusy),
    AMD_MAKE_NAME(ShaderBusyCS),
    AMD_MAKE_NAME(ShaderBusyDS),
    AMD_MAKE_NAME(ShaderBusyGS),
    AMD_MAKE_NAME(ShaderBusyHS),
    AMD_MAKE_NAME(ShaderBusyPS),
    AMD_MAKE_NAME(ShaderBusyVS),
    AMD_MAKE_NAME(TessellatorBusy),
    AMD_MAKE_NAME(TexAveAnisotropy),
    AMD_MAKE_NAME(TexCacheStalled),
    AMD_MAKE_NAME(TexCostOfFiltering),
    AMD_MAKE_NAME(TexelFetchCount),
    AMD_MAKE_NAME(TexMemBytesRead),
    AMD_MAKE_NAME(TexMissRate),
    AMD_MAKE_NAME(TexTriFilteringPct),
    AMD_MAKE_NAME(TexUnitBusy),
    AMD_MAKE_NAME(TexVolFilteringPct),
    AMD_MAKE_NAME(VertexMemFetched),
    AMD_MAKE_NAME(VertexMemFetchedCost),
    AMD_MAKE_NAME(VSALUBusy),
    AMD_MAKE_NAME(VSALUEfficiency),
    AMD_MAKE_NAME(VSALUInstCount),
    AMD_MAKE_NAME(VSALUTexRatio),
    AMD_MAKE_NAME(VSBusy),
    AMD_MAKE_NAME(VSTexBusy),
    AMD_MAKE_NAME(VSTexInstCount),
    AMD_MAKE_NAME(VSVerticesIn),
    AMD_MAKE_NAME(ZUnitStalled)
};

std::string const AMDPerformance::msStatusName[MAX_AMD_STATUS_NAMES] =
{
    AMD_MAKE_NAME(GPA_STATUS_OK),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_NULL_POINTER),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_COUNTERS_NOT_OPEN),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_INDEX_OUT_OF_RANGE),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_NOT_FOUND),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_ALREADY_ENABLED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_NO_COUNTERS_ENABLED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_NOT_ENABLED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLING_NOT_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLING_ALREADY_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLING_NOT_ENDED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_NOT_ENOUGH_PASSES),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_PASS_NOT_ENDED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_PASS_NOT_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_PASS_ALREADY_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLE_NOT_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLE_ALREADY_STARTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLE_NOT_ENDED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_CANNOT_CHANGE_COUNTERS_WHEN_SAMPLING),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SESSION_NOT_FOUND),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLE_NOT_FOUND),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_SAMPLE_NOT_FOUND_IN_ALL_PASSES),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_COUNTER_NOT_OF_SPECIFIED_TYPE),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_READING_COUNTER_RESULT),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_FAILED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_HARDWARE_NOT_SUPPORTED),
    AMD_MAKE_NAME(GPA_STATUS_ERROR_CATALYST_VER_UNSUPPORTED_SI)
};

std::string const AMDPerformance::msTypeName[MAX_AMD_TYPE_NAMES] =
{
    AMD_MAKE_NAME(GPA_TYPE_FLOAT32),
    AMD_MAKE_NAME(GPA_TYPE_FLOAT64),
    AMD_MAKE_NAME(GPA_TYPE_UINT32),
    AMD_MAKE_NAME(GPA_TYPE_UINT64),
    AMD_MAKE_NAME(GPA_TYPE_INT32),
    AMD_MAKE_NAME(GPA_TYPE_INT64)
};

std::string const AMDPerformance::msUsageTypeName[MAX_AMD_USAGE_TYPE_NAMES] =
{
    AMD_MAKE_NAME(GPA_USAGE_TYPE_RATIO),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_PERCENTAGE),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_CYCLES),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_MILLISECONDS),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_BYTES),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_ITEMS),
    AMD_MAKE_NAME(GPA_USAGE_TYPE_KILOBYTES)
};
