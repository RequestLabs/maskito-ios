
#include <Accelerate/Accelerate.h>

#include "fft_stuff.hpp"
#include <iostream>

namespace dlib {
    FFTSetup fft_setup = vDSP_create_fftsetup(5, kFFTRadix2);
    FFTSetupD fft_setupD = vDSP_create_fftsetupD(5, kFFTRadix2);
    void apple_fft_inplace( std::complex<float> * ptr, int log2_r, int log2_c) {
        
        float * real_ptr = (float *)(ptr);
        float * imag_ptr = real_ptr + 1;
        
        int log2 = std::max(log2_r, log2_c);
        
        FFTSetup fft_setup = vDSP_create_fftsetup(log2, kFFTRadix2);
        DSPSplitComplex fft_data{real_ptr, imag_ptr};
        vDSP_fft2d_zip(fft_setup, &fft_data, 2, 0, log2_r, log2_c, kFFTDirection_Forward);
        vDSP_destroy_fftsetup(fft_setup);
    }
    
    void apple_ifft_inplace( std::complex<float> * ptr, int log2_r, int log2_c) {
        
        float * real_ptr = (float *)(ptr);
        float * imag_ptr = real_ptr + 1;
        
        int log2 = std::max(log2_r, log2_c);
        
        FFTSetup fft_setup = vDSP_create_fftsetup(log2, kFFTRadix2);
        DSPSplitComplex fft_data{real_ptr, imag_ptr};
        vDSP_fft2d_zip(fft_setup, &fft_data, 2, 0, log2_r, log2_c, kFFTDirection_Inverse);
        vDSP_destroy_fftsetup(fft_setup);
    }
    
    void apple_fft_inplaceD( std::complex<double> * ptr, int log2_r, int log2_c) {
        
        double * real_ptr = (double *)(ptr);
        double * imag_ptr = real_ptr + 1;
        
        int log2 = std::max(log2_r, log2_c);
        
        FFTSetupD fft_setup = vDSP_create_fftsetupD(log2, kFFTRadix2);
        DSPDoubleSplitComplex fft_data{real_ptr, imag_ptr};
        vDSP_fft2d_zipD(fft_setupD, &fft_data, 2, 0, log2_r, log2_c, kFFTDirection_Forward);
        vDSP_destroy_fftsetupD(fft_setup);
    }
    
    void apple_ifft_inplaceD( std::complex<double> * ptr, int log2_r, int log2_c) {
        
        double * real_ptr = (double *)(ptr);
        double * imag_ptr = real_ptr + 1;
        
        int log2 = std::max(log2_r, log2_c);
        
        FFTSetupD fft_setup = vDSP_create_fftsetupD(log2, kFFTRadix2);
        DSPDoubleSplitComplex fft_data{real_ptr, imag_ptr};
        vDSP_fft2d_zipD(fft_setupD, &fft_data, 2, 0, log2_r, log2_c, kFFTDirection_Inverse);
        vDSP_destroy_fftsetupD(fft_setup);
    }
}
