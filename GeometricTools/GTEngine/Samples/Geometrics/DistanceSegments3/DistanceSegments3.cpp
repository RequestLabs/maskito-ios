// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "GTEngine.h"
#include <iostream>
using namespace gte;


// The function dist3D_Segment_to_Segment is from Dan Sunday's website:
//   http://geomalgorithms.com/a07-_distance.html
// with some modifications.  The inputs of type Segment were replaced by
// point pairs of type Vector3<double> and the algebraic operator calls were
// replaced accordingly.  The distance is now returned (with other parameters)
// as arguments to the function.  The SMALL_NUM macro was replaced by a 
// 'const' declaration.  The modified code computes the closest points.  See
// the revised document (as of 2014/11/05)
//   http://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
// that describes an algorithm that is robust, particularly for nearly
// segments, and that uses floating-point arithmetic.  An example in this PDF
// shows that there is a problem with the logic of Sunday's algorithm when
// D < SMALL_NUM and the search is started on the s=0 edge. Specifically,
// the closest points are not found correctly--the closest point on the
// first segment occurs when s=1.  No contact information is at his website,
// so we are unable to report the problem to him.

void dist3D_Segment_to_Segment(
    Vector3<double> const& P0, Vector3<double> const& P1,
    Vector3<double> const& Q0, Vector3<double> const& Q1,
    double& sqrDistance, double& s, double& t, Vector3<double> closest[2])
{
    double const SMALL_NUM = 0.00000001;
    Vector3<double>   u = P1 - P0;
    Vector3<double>   v = Q1 - Q0;
    Vector3<double>   w = P0 - Q0;
    double    a = Dot(u, u);         // always >= 0
    double    b = Dot(u, v);
    double    c = Dot(v, v);         // always >= 0
    double    d = Dot(u, w);
    double    e = Dot(v, w);
    double    D = a*c - b*b;        // always >= 0
    double    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    double    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < SMALL_NUM) { // the lines are almost parallel
        sN = 0.0;         // force using point P0 on segment S1
        sD = 1.0;         // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else {                 // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d + b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (std::abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
    tc = (std::abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

    // get the difference of the two closest points
    s = sc;
    t = tc;
    closest[0] = (1.0 - sc) * P0 + sc * P1;
    closest[1] = (1.0 - tc) * Q0 + tc * Q1;
    Vector3<double> diff = closest[0] - closest[1];
    sqrDistance = Dot(diff, diff);
}

enum { PERF_SUNDAY, PERF_ROBUST, PERF_RATIONAL };
typedef BSRational<UIntegerFP32<128>> Rational;
typedef DCPQuery<double, Segment<3, double>, Segment<3, double>> RobustQuery;
typedef DistanceSegmentSegmentExact<3, Rational> RationalQuery;

void CPUAccuracyTest(bool compareUsingExact, bool testNonparallel)
{
    // NOTE:  When comparing to exact arithmetic results, the number of inputs
    // needs to be smaller because the exact algorithm is expensive to compute.
    // In this case the maximum errors are all small (4e-16).  However, when
    // not comparing to exact results, maxError01 is on the order of 1e-4.
    // The pair of segments that generate the maximum error shows that the
    // logic of dist3D_Segment_to_Segment when segments are nearly parallel
    // is not correct.
    unsigned int const numInputs = (compareUsingExact ? 1024 : 16384);
    unsigned int const numBlocks = 16;
    std::vector<Segment<3, double>> segment(numInputs);

    std::mt19937 mte;
    if (testNonparallel)
    {
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                segment[i].p[0][j] = rnd(mte);
                segment[i].p[1][j] = rnd(mte);
            }
        }
    }
    else
    {
        std::uniform_real_distribution<double> rnd(0.9, 1.1);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            segment[i].p[0][0] = -rnd(mte);
            segment[i].p[0][1] = 0.0;
            segment[i].p[0][2] = 1e-06 * rnd(mte);
            segment[i].p[1][0] = rnd(mte);
            segment[i].p[1][1] = 0.0;
            segment[i].p[1][2] = -1e-06 * rnd(mte);
        }
    }

    double maxError01 = 0.0, maxError02 = 0.0, maxError12 = 0.0, error;
    unsigned int xmax01 = 0, ymax01 = 0;
    unsigned int xmax02 = 0, ymax02 = 0;
    unsigned int xmax12 = 0, ymax12 = 0;

    for (unsigned int y = 0; y < numInputs; ++y)
    {
        if ((y % numBlocks) == 0)
        {
            std::cout << "y = " << y << std::endl;
        }

        Vector3<double> Q0 = segment[y].p[0];
        Vector3<double> Q1 = segment[y].p[1];

        for (unsigned int x = y + 1; x < numInputs; ++x)
        {
            Vector3<double> P0 = segment[x].p[0];
            Vector3<double> P1 = segment[x].p[1];

            // Sunday's query
            double sqrDistance0, s0, t0;
            Vector3<double> closest0[2];
            dist3D_Segment_to_Segment(P0, P1, Q0, Q1, sqrDistance0,
                s0, t0, closest0);
            double distance0 = sqrt(sqrDistance0);

            // robust query
            RobustQuery query1;
            auto result1 = query1(P0, P1, Q0, Q1);
            double distance1 = result1.distance;

            if (compareUsingExact)
            {
                // rational query
                Vector3<Rational> RP0{ P0[0], P0[1], P0[2] };
                Vector3<Rational> RP1{ P1[0], P1[1], P1[2] };
                Vector3<Rational> RQ0{ Q0[0], Q0[1], Q0[2] };
                Vector3<Rational> RQ1{ Q1[0], Q1[1], Q1[2] };
                RationalQuery query2;
                auto result2 = query2(RP0, RP1, RQ0, RQ1);
                double distance2 = sqrt((double)result2.sqrDistance);

                error = std::abs(distance0 - distance2);
                if (error > maxError02)
                {
                    maxError02 = error;
                    xmax02 = x;
                    ymax02 = y;
                }

                error = std::abs(distance1 - distance2);
                if (error > maxError12)
                {
                    maxError12 = error;
                    xmax12 = x;
                    ymax12 = y;
                }
            }

            error = std::abs(distance0 - distance1);
            if (error > maxError01)
            {
                maxError01 = error;
                xmax01 = x;
                ymax01 = y;
            }
        }
    }

    if (compareUsingExact)
    {
        std::cout << "max error02 = " << maxError02 << std::endl;
        std::cout << "x, y = " << xmax02 << " " << ymax02 << std::endl;
        std::cout << "max error12 = " << maxError12 << std::endl;
        std::cout << "x, y = " << xmax12 << " " << ymax12 << std::endl;
    }
    std::cout << "max error01 = " << maxError01 << std::endl;
    std::cout << "x, y = " << xmax01 << " " << ymax01 << std::endl;
}

void CPUPerformanceTest(int select, bool testNonparallel)
{
    unsigned int const numInputs = (select == PERF_RATIONAL ? 1024 : 16384);
    std::vector<Segment<3, double>> segment(numInputs);

    std::mt19937 mte;
    if (testNonparallel)
    {
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                segment[i].p[0][j] = rnd(mte);
                segment[i].p[1][j] = rnd(mte);
            }
        }
    }
    else
    {
        std::uniform_real_distribution<double> rnd(0.9, 1.1);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            segment[i].p[0][0] = -rnd(mte);
            segment[i].p[0][1] = 0.0;
            segment[i].p[0][2] = 1e-06 * rnd(mte);
            segment[i].p[1][0] = rnd(mte);
            segment[i].p[1][1] = 0.0;
            segment[i].p[1][2] = -1e-06 * rnd(mte);
        }
    }

    Timer timer;

    if (select == PERF_SUNDAY)
    {
        // 7.43 seconds (134209536 queries, 5.536119e-8 seconds/query)
        double sqrDistance0, s0, t0;
        Vector3<double> closest0[2];
        for (unsigned int y = 0; y < numInputs; ++y)
        {
            for (unsigned int x = y + 1; x < numInputs; ++x)
            {
                dist3D_Segment_to_Segment(
                    segment[x].p[0], segment[x].p[1],
                    segment[y].p[0], segment[y].p[1],
                    sqrDistance0, s0, t0, closest0);
            }
        }
    }
    else if (select == PERF_ROBUST)
    {
        // 9.18 seconds (134209536 queries, 6.840050e-8 seconds/query)
        RobustQuery query;
        RobustQuery::Result result;
        for (unsigned int y = 0; y < numInputs; ++y)
        {
            for (unsigned int x = y + 1; x < numInputs; ++x)
            {
                result = query(segment[x], segment[y]);
            }
        }
    }
    else  // select == PERF_RATIONAL
    {
        // 11.00 seconds (523776 queries, 2.100134e-05 seconds/query,
        // approximately 307 times longer per query for exact rational,
        // so 134209536 queries takes on the order of 47 minutes)
        RationalQuery query;
        RationalQuery::Result result;
        Vector3<Rational> RP0, RP1, RQ0, RQ1;
        for (unsigned int y = 0; y < numInputs; ++y)
        {
            for (int i = 0; i < 3; ++i)
            {
                RQ0[i] = segment[y].p[0][i];
                RQ1[i] = segment[y].p[1][i];
            }
            for (unsigned int x = y + 1; x < numInputs; ++x)
            {
                for (int i = 0; i < 3; ++i)
                {
                    RP0[i] = segment[x].p[0][i];
                    RP1[i] = segment[x].p[1][i];
                }
                result = query(RP0, RP1, RQ0, RQ1);
            }
        }
    }

    std::cout << timer.GetSeconds() << std::endl;
}

void GPUAccuracyTest(bool getClosest, bool testNonparallel)
{
    unsigned int const numInputs = 16384;
    unsigned int const blockSize = 1024;
    unsigned int const numBlocks = numInputs / blockSize;
    unsigned int const numThreads = 8;
    unsigned int const numGroups = blockSize / numThreads;

    DX11Engine* engine = new DX11Engine();

    HLSLProgramFactory factory;
    factory.defines.Set("NUM_X_THREADS", numThreads);
    factory.defines.Set("NUM_Y_THREADS", numThreads);
    factory.defines.Set("BLOCK_SIZE", blockSize);
    factory.defines.Set("REAL", "double");
    factory.defines.Set("REAL3", "double3");
    factory.defines.Set("GET_CLOSEST", (getClosest ? 1 : 0));

    std::shared_ptr<ComputeProgram> cprogram = factory.CreateFromFile(
        "DistanceSeg3Seg3.hlsl");
    std::shared_ptr<ComputeShader> cshader = cprogram->GetCShader();
    std::shared_ptr<ConstantBuffer> block(
        new ConstantBuffer(2 * sizeof(uint32_t), true));
    cshader->Set("Block", block);
    uint32_t* origin = block->Get<uint32_t>();

    std::shared_ptr<StructuredBuffer> input(
        new StructuredBuffer(numInputs, sizeof(Segment<3, double>)));
    input->SetUsage(Resource::DYNAMIC_UPDATE);
    cshader->Set("input", input);
    Segment<3, double>* segment = input->Get<Segment<3, double>>();

    std::mt19937 mte;
    if (testNonparallel)
    {
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                segment[i].p[0][j] = rnd(mte);
                segment[i].p[1][j] = rnd(mte);
            }
        }
    }
    else
    {
        std::uniform_real_distribution<double> rnd(0.9, 1.1);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            segment[i].p[0][0] = -rnd(mte);
            segment[i].p[0][1] = 0.0;
            segment[i].p[0][2] = 1e-06 * rnd(mte);
            segment[i].p[1][0] = rnd(mte);
            segment[i].p[1][1] = 0.0;
            segment[i].p[1][2] = -1e-06 * rnd(mte);
        }
    }

    // AMD 7970 (not overclocked), use 'double':
    //   Get s, t:  0.99 seconds
    //   Get closest0, closest1:  2.86785 seconds
    std::shared_ptr<StructuredBuffer> output;
    double maxError = 0.0;
    int xmax = 0, ymax = 0;
    if (getClosest)
    {
        struct Result
        {
            double sqrDistance;
            double parameter[2];
            Vector3<double> closest[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize,
            sizeof(Result));
        output->SetUsage(Resource::SHADER_OUTPUT);
        output->SetCopyType(Resource::COPY_STAGING_TO_CPU);
        Result* gpuResult = output->Get<Result>();
        cshader->Set("output", output);

        for (unsigned int y = 0, i = 0; y < numBlocks; ++y)
        {
            std::cout << "block = " << y << std::endl;
            origin[1] = y * blockSize;
            for (unsigned int x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                engine->Update(block);
                engine->Execute(cshader, numGroups, numGroups, 1);
                engine->CopyGpuToCpu(output);

                for (unsigned int r = 0; r < blockSize; ++r)
                {
                    int sy = origin[1] + r;
                    Vector3<double> Q0 = segment[sy].p[0];
                    Vector3<double> Q1 = segment[sy].p[1];

                    unsigned int cmin = (x != y ? 0 : r + 1);
                    for (unsigned int c = cmin; c < blockSize; ++c)
                    {
                        int sx = origin[0] + c;
                        Vector3<double> P0 = segment[sx].p[0];
                        Vector3<double> P1 = segment[sx].p[1];

                        Result result0 = gpuResult[c + blockSize * r];
                        double distance0 = sqrt(result0.sqrDistance);

                        RobustQuery query1;
                        auto result1 = query1(P0, P1, Q0, Q1);
                        double distance1 = result1.distance;

                        double error = std::abs(distance0 - distance1);
                        if (error > maxError)
                        {
                            maxError = error;
                            xmax = sx;
                            ymax = sy;
                        }
                    }
                }
            }
        }
    }
    else
    {
        struct Result
        {
            double sqrDistance;
            double parameter[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize,
            sizeof(Result));
        output->SetUsage(Resource::SHADER_OUTPUT);
        output->SetCopyType(Resource::COPY_STAGING_TO_CPU);
        Result* gpuResult = output->Get<Result>();
        cshader->Set("output", output);

        for (unsigned int y = 0, i = 0; y < numBlocks; ++y)
        {
            std::cout << "block = " << y << std::endl;
            origin[1] = y * blockSize;
            for (unsigned int x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                engine->Update(block);
                engine->Execute(cshader, numGroups, numGroups, 1);
                engine->CopyGpuToCpu(output);

                for (unsigned int r = 0; r < blockSize; ++r)
                {
                    int sy = origin[1] + r;
                    Vector3<double> Q0 = segment[sy].p[0];
                    Vector3<double> Q1 = segment[sy].p[1];

                    unsigned int cmin = (x != y ? 0 : r + 1);
                    for (unsigned int c = cmin; c < blockSize; ++c)
                    {
                        int sx = origin[0] + c;
                        Vector3<double> P0 = segment[sx].p[0];
                        Vector3<double> P1 = segment[sx].p[1];

                        Result result0 = gpuResult[c + blockSize * r];
                        double distance0 = sqrt(result0.sqrDistance);

                        RobustQuery query1;
                        auto result1 = query1(P0, P1, Q0, Q1);
                        double distance1 = result1.distance;

                        double error = std::abs(distance0 - distance1);
                        if (error > maxError)
                        {
                            maxError = error;
                            xmax = sx;
                            ymax = sy;
                        }
                    }
                }
            }
        }
    }

    std::cout << "max error = " << maxError << std::endl;
    std::cout << "x, y = " << xmax << " " << ymax << std::endl;

    output = nullptr;
    input = nullptr;
    block = nullptr;
    cprogram = nullptr;
    cshader = nullptr;
    delete engine;
}

void GPUPerformanceTest(bool getClosest, bool testNonparallel)
{
    unsigned int const numInputs = 16384;
    unsigned int const blockSize = 1024;
    unsigned int const numBlocks = numInputs / blockSize;
    unsigned int const numThreads = 8;
    unsigned int const numGroups = blockSize / numThreads;

    DX11Engine* engine = new DX11Engine();

    HLSLProgramFactory factory;
    factory.defines.Set("NUM_X_THREADS", numThreads);
    factory.defines.Set("NUM_Y_THREADS", numThreads);
    factory.defines.Set("BLOCK_SIZE", blockSize);
    factory.defines.Set("REAL", "double");
    factory.defines.Set("REAL3", "double3");
    factory.defines.Set("GET_CLOSEST", (getClosest ? 1 : 0));
    std::shared_ptr<ComputeProgram> cprogram = factory.CreateFromFile(
        "DistanceSeg3Seg3.hlsl");
    std::shared_ptr<ComputeShader> cshader = cprogram->GetCShader();

    std::shared_ptr<ConstantBuffer> block(
        new ConstantBuffer(2 * sizeof(uint32_t), true));
    cshader->Set("Block", block);
    uint32_t* origin = block->Get<uint32_t>();

    std::shared_ptr<StructuredBuffer> input(
        new StructuredBuffer(numInputs, sizeof(Segment<3, double>)));
    input->SetUsage(Resource::DYNAMIC_UPDATE);
    cshader->Set("input", input);
    Segment<3, double>* segment = input->Get<Segment<3, double>>();

    std::mt19937 mte;
    if (testNonparallel)
    {
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                segment[i].p[0][j] = rnd(mte);
                segment[i].p[1][j] = rnd(mte);
            }
        }
    }
    else
    {
        std::uniform_real_distribution<double> rnd(0.9, 1.1);
        for (unsigned int i = 0; i < numInputs; ++i)
        {
            segment[i].p[0][0] = -rnd(mte);
            segment[i].p[0][1] = 0.0;
            segment[i].p[0][2] = 1e-06 * rnd(mte);
            segment[i].p[1][0] = rnd(mte);
            segment[i].p[1][1] = 0.0;
            segment[i].p[1][2] = -1e-06 * rnd(mte);
        }
    }

    // AMD 7970 (not overclocked), use 'double':
    //   Get s, t:  1.04 seconds
    //   Get closest0, closest1:  2.85 seconds
    Timer timer;
    std::shared_ptr<StructuredBuffer> output;
    if (getClosest)
    {
        struct Result
        {
            double sqrDistance;
            double parameter[2];
            Vector3<double> closest[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize,
            sizeof(Result));
        output->SetUsage(Resource::SHADER_OUTPUT);
        output->SetCopyType(Resource::COPY_STAGING_TO_CPU);
        cshader->Set("output", output);

        for (unsigned int y = 0, i = 0; y < numBlocks; ++y)
        {
            origin[1] = y * blockSize;
            for (unsigned int x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                engine->Update(block);
                engine->Execute(cshader, numGroups, numGroups, 1);
                engine->CopyGpuToCpu(output);
            }
        }

    }
    else
    {
        struct Result
        {
            double sqrDistance;
            double parameter[2];
        };

        output = std::make_shared<StructuredBuffer>(blockSize * blockSize,
            sizeof(Result));
        output->SetUsage(Resource::SHADER_OUTPUT);
        output->SetCopyType(Resource::COPY_STAGING_TO_CPU);
        cshader->Set("output", output);

        for (unsigned int y = 0, i = 0; y < numBlocks; ++y)
        {
            origin[1] = y * blockSize;
            for (unsigned int x = y; x < numBlocks; ++x, ++i)
            {
                origin[0] = x * blockSize;
                engine->Update(block);
                engine->Execute(cshader, numGroups, numGroups, 1);
                engine->CopyGpuToCpu(output);
            }
        }
    }
    std::cout << "seconds = " << timer.GetSeconds() << std::endl;

    output = nullptr;
    input = nullptr;
    block = nullptr;
    cshader = nullptr;
    cprogram = nullptr;
    delete engine;
}

int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    CPUAccuracyTest(true, true);
    //CPUAccuracyTest(true, false);
    //CPUAccuracyTest(false, true);
    //CPUAccuracyTest(false, false);

    //CPUPerformanceTest(PERF_SUNDAY, true);
    //CPUPerformanceTest(PERF_SUNDAY, false);
    //CPUPerformanceTest(PERF_ROBUST, true);
    //CPUPerformanceTest(PERF_ROBUST, false);
    //CPUPerformanceTest(PERF_RATIONAL, true);
    //CPUPerformanceTest(PERF_RATIONAL, false);

    //GPUAccuracyTest(true, true);
    //GPUAccuracyTest(true, false);
    //GPUAccuracyTest(false, true);
    //GPUAccuracyTest(false, false);

    //GPUPerformanceTest(true, true);
    //GPUPerformanceTest(true, false);
    //GPUPerformanceTest(false, true);
    //GPUPerformanceTest(false, false);

    return 0;
}

