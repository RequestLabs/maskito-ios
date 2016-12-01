You must download AMD's GPU performance measurement library (www.amd.com).
We had downloaded version 2.11.739.0 of the API.  The files must be copied
to create the following folder structure.

    #Tools
        GPUPerfAPI-2.11.739.0
            GPUPerfAPI-UserGuide.pdf
            License.rtf
            Bin
                x64
                    GPUPerfAPICL-x64.dll
                    GPUPerfAPIDX10-x64.dll
                    GPUPerfAPIDX11-x64.dll
                    GPUPerfAPIGL-x64.dll
                x86
                    GPUPerfAPICL.dll
                    GPUPerfAPIDX10.dll
                    GPUPerfAPIDX11.dll
                    GPUPerfAPIGL.dll
            Include
                GPUPerfAPI.h
                GPUPerfAPIFunctionsTypes.h
                GPUPerfAPITypes.h

To run the AMD performance sample (on an AMD card, of course), you must
ensure that the DLLs can be found.  You can set your PATH to include the
folders mentioned previously.  We chose instead to create folders
    C:\Program Files (x86)\GeometricTools\{x64,x86}
and added these folders to our PATH.  This allows us to copy our
executable tools to this folder and be able to execute them from any
other folder.

The files we provided for the performance tool are

    #Tools
        GPUPerfAPI-2.11.739.0
            AMD7970Counters.txt
            ReadMe.txt
            GPUPerfAPI-UserGuide.pdf
            License.rtf
            Include
                GteAMDPerformance.h
                GteAMDPerformance.inl
            Source
                GteAMDPerformance.cpp

The sample application that uses the AMD GPU performance library is

    #Samples
        Basics
            PerformanceAMD

The file locations in the project file reference the appropriate locations
in the #Tools hierarchy.
