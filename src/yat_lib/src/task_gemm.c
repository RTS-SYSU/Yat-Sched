#include "task_gemm.h"

/*
  Declaration of global variables
*/

int matrix1_A[ X * Y ];
int matrix1_B[ Y * Z ];
int matrix1_C[ X * Z ];


/*
  Initialization functions
*/

void matrix1_pin_down( int A[  ], int B[  ], int C[  ] )
{
  int i;
  volatile int x = 1;

  _Pragma( "loopbound min 100 max 100" )
  for ( i = 0 ; i < X * Y; i++ )
    A[ i ] = x ;

  _Pragma( "loopbound min 100 max 100" )
  for ( i = 0 ; i < Y * Z ; i++ )
    B[ i ] = x ;

  _Pragma( "loopbound min 100 max 100" )
  for ( i = 0 ; i < X * Z ; i++ )
    C[ i ] = 0 ;
}


void matrix1_init( void )
{
  matrix1_pin_down( &matrix1_A[ 0 ], &matrix1_B[ 0 ], &matrix1_C[ 0 ] );
}

/*
  Return function
*/

int matrix1_return( void )
{
  int i;
  int checksum = 0;

  _Pragma( "loopbound min 100 max 100" )
  for ( i = 0; i < X * Z; i++ )
    checksum += matrix1_C[ i ];

  return ( checksum ==  1000 ? 0 : -1 );
}


/*
  Main functions
*/

void _Pragma ( "entrypoint" ) matrix1_main( void )
{
  register int *p_a = &matrix1_A[ 0 ];
  register int *p_b = &matrix1_B[ 0 ];
  register int *p_c = &matrix1_C[ 0 ];

  register int f, i, k;

  _Pragma( "loopbound min 10 max 10" )
  for ( k = 0; k < Z; k++ ) {
    p_a = &matrix1_A[ 0 ];                /* point to the beginning of array A */

    _Pragma( "loopbound min 10 max 10" )
    for ( i = 0; i < X; i++ ) {
      p_b = &matrix1_B[ k * Y ];          /* take next column */

      *p_c = 0;
      _Pragma( "loopbound min 10 max 10" )
      for ( f = 0; f < Y; f++ ) /* do multiply */
        *p_c += *p_a++ * *p_b++;

      p_c++;
    }
  }
}


int task_gemm( void )
{
  int result;
  matrix1_init();
  matrix1_main();

  result = matrix1_return();
  return result;
}