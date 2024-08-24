#include "task_insertsort.h"

/*
  Declaration of global variables
*/
unsigned int insertsort_a[ 11 ];
int insertsort_iters_i, insertsort_min_i, insertsort_max_i;
int insertsort_iters_a, insertsort_min_a, insertsort_max_a;

/*
  Initialization- and return-value-related functions
*/

void insertsort_initialize( unsigned int *array )
{

  register volatile int i;
  _Pragma( "loopbound min 11 max 11" )
  for ( i = 0; i < 11; i++ )
    insertsort_a[ i ] = array[ i ];

}


void insertsort_init()
{
  unsigned int a[ 11 ] = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};

  insertsort_iters_i = 0;
  insertsort_min_i = 100000;
  insertsort_max_i = 0;
  insertsort_iters_a = 0;
  insertsort_min_a = 100000;
  insertsort_max_a = 0;

  insertsort_initialize( a );
}

int insertsort_return()
{
  int i, returnValue = 0;

  _Pragma( "loopbound min 11 max 11" )
  for ( i = 0; i < 11; i++ )
    returnValue += insertsort_a[ i ];

  return ( returnValue + ( -65 ) ) != 0;
}


/*
  Main functions
*/


void _Pragma( "entrypoint" ) insertsort_main()
{
  int  i, j, temp;
  i = 2;

  insertsort_iters_i = 0;

  _Pragma( "loopbound min 9 max 9" )
  while ( i <= 10 ) {

    insertsort_iters_i++;

    j = i;

    insertsort_iters_a = 0;

    _Pragma( "loopbound min 1 max 9" )
    while ( insertsort_a[ j ] < insertsort_a[ j - 1 ] ) {
      insertsort_iters_a++;

      temp = insertsort_a[ j ];
      insertsort_a[ j ] = insertsort_a[ j - 1 ];
      insertsort_a[ j - 1 ] = temp;
      j--;
    }

    if ( insertsort_iters_a < insertsort_min_a )
      insertsort_min_a = insertsort_iters_a;
    if ( insertsort_iters_a > insertsort_max_a )
      insertsort_max_a = insertsort_iters_a;

    i++;
  }

  if ( insertsort_iters_i < insertsort_min_i )
    insertsort_min_i = insertsort_iters_i;
  if ( insertsort_iters_i > insertsort_max_i )
    insertsort_max_i = insertsort_iters_i;
}

int task_insertsort( void )
{
  int result;
  insertsort_init();
  insertsort_main();
  result = insertsort_return();
  return result;
}