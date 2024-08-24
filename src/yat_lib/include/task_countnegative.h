/*

  This program is part of the TACLeBench benchmark suite.
  Version V 2.0

  Name: countnegative

  Author: unknown

  Function: Counts negative and non-negative numbers in a
    matrix. Features nested loops, well-structured code.

  Source: MRTC
          http://www.mrtc.mdh.se/projects/wcet/wcet_bench/cnt/cnt.c

  Changes: Changed split between initialization and computation

  License: May be used, modified, and re-distributed freely

*/

/*
  The dimension of the matrix
*/
#define MAXSIZE 20

/*
  Type definition for the matrix
*/
typedef int matrix [ MAXSIZE ][ MAXSIZE ];

/*
  Forward declaration of functions
*/
void countnegative_initSeed( void );
int countnegative_randomInteger( void );
void countnegative_initialize( matrix );
void countnegative_init( void );
int countnegative_return( void );
void countnegative_sum( matrix );
void countnegative_main( void );

int task_countnegative( void );

