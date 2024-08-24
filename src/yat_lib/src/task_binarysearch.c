#include "task_binarysearch.h"
/*
  Declaration of global variables
*/

volatile int binarysearch_seed;

struct binarysearch_DATA {
  int key;
  int value;
};

struct binarysearch_DATA binarysearch_data[ 15 ];

int binarysearch_result;


/*
  Initialization- and return-value-related functions
*/

/*
  binarysearch_initSeed initializes the seed used in the "random" number
  generator.
*/
void binarysearch_initSeed( void )
{
  binarysearch_seed = 0;
}


/*
  binarysearch_RandomInteger generates "random" integers between 0 and 8094.
*/
long binarysearch_randomInteger( void )
{
  binarysearch_seed = ( ( binarysearch_seed * 133 ) + 81 ) % 8095;
  return ( binarysearch_seed );
}


void binarysearch_init( void )
{
  int i;

  binarysearch_initSeed();

  _Pragma( "loopbound min 15 max 15" )
  for ( i = 0; i < 15; ++i ) {
    binarysearch_data[ i ].key = binarysearch_randomInteger();
    binarysearch_data[ i ].value = binarysearch_randomInteger();
  }
}


int binarysearch_return( void )
{
  return ( binarysearch_result );
}


/*
  Algorithm core functions
*/

int binarysearch_binary_search( int x )
{
  int fvalue, mid, up, low;

  low = 0;
  up = 14;
  fvalue = -1;

  _Pragma( "loopbound min 1 max 4" )
  while ( low <= up ) {
    mid = ( low + up ) >> 1;

    if ( binarysearch_data[ mid ].key == x ) {
      /* Item found */
      up = low - 1;
      fvalue = binarysearch_data[ mid ].value;
    } else

      if ( binarysearch_data[ mid ].key > x )
        /* Item not found */
        up = mid - 1;
      else
        low = mid + 1;
  }

  return ( fvalue );
}


/*
  Main functions
*/

void _Pragma( "entrypoint" ) binarysearch_main( void )
{
  binarysearch_result = binarysearch_binary_search( 8 );
}


int task_binarysearch( void )
{
  int result;
  binarysearch_init();
  binarysearch_main();

  result = binarysearch_return() - ( -1 ) != 0;
  return result;
}