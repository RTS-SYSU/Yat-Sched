/*

  This program is part of the TACLeBench benchmark suite.
  Version V 2.0

  Name: fft

  Author: Juan Martinez Velarde

  Function: benchmarking of an integer stage scaling FFT
    To avoid errors caused by overflow and bit growth,
    the input data is scaled. Bit growth occurs potentially
    at butterfly operations, which involve a complex
    multiplication, a complex addition and a complex
    subtraction. Maximal bit growth from butterfly input
    to butterfly output is two bits.

    The input data includes enough extra sign bits, called
    guard bits, to ensure that bit growth never results in
    overflow (Rabiner and Gold, 1975). Data can grow by a
    maximum factor of 2.4 from butterfly input to output
    (two bits of grow). However, a data value cannot grow by
    maximum amount in two consecutive stages.
    The number of guard bits necessary to compensate the
    maximum bit growth in an N-point FFT is (log_2 (N))+1).

    In a 16-point FFT (requires 4 stages), each of the
    input samples should contain 5 guard bits. The input
    data is then restricted to 10 bits, one sign bit and
    nine magnitude bits, in order to prevent an
    overflow from the integer multiplication with the
    precalculed twiddle coefficients.

    Another method to compensate bit growth is to scale the
    outputs down by a factor of two unconditionally after
    each stage. This approach is called unconditional scaling

    Initially, 2 guard bits are included in the input data to
    accomodate the maximum overflow in the first stage.
    In each butterfly of a stage calculation, the data can
    grow into the guard bits. To prevent overflow in the next
    stage, the guard bits are replaced before the next stage is
    executed by shifting the entire block of data one bit
    to the right.

    Input data should not be restricted to a 1.9 format.
    Input data can be represented in a 1.13 format,that is
    14 significant bits, one sign and 13 magnitude bits. In
    the FFT calculation, the data loses a total of (log2 N) -1
    bits because of shifting. Unconditional scaling results
    in the same number of bits lost as in the input data scaling.
    However, it produces more precise results because the
    FFT starts with more precise input data. The tradeoff is
    a slower FFT calculation because of the extra cycles needed
    to shift the output of each stage.

  Source: DSP-Stone
    http://www.ice.rwth-aachen.de/research/tools-projects/entry/detail/dspstone/

  Original name: fft_1024_13
    (merged main1024_bit_reduct and fft_bit_reduct from DSP-Stone)

  Changes: no major functional changes

  License: may be used, modified, and re-distributed freely

*/

/*
  Forward declaration of functions
*/

float fft_exp2f( float x );
float fft_modff( float x, float *intpart );
int fft_convert( float value );
void fft_bit_reduct( register int *int_pointer );
void fft_pin_down( int input_data[  ] );
void fft_init( void );
void fft_main( void );
int fft_return( void );

int task_fft( void );

