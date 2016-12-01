

#ifndef fft_stuff_hpp
#define fft_stuff_hpp

#include <stdio.h>
void apple_fft_inplace( std::complex<float> * ptr, int log2_r, int log2_c);
void apple_ifft_inplace( std::complex<float> * ptr, int log2_r, int log2_c);
void apple_fft_inplaceD( std::complex<double> * ptr, int log2_r, int log2_c);
void apple_ifft_inplaceD( std::complex<double> * ptr, int log2_r, int log2_c);
#endif /* fft_stuff_hpp */
