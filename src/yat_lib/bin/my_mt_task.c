/* based_mt_task.c -- A basic multi-threaded real-time task skeleton.
 *
 * This (by itself useless) task demos how to setup a multi-threaded YAT^RT
 * real-time task. Familiarity with the single threaded example (base_task.c)
 * is assumed.
 *
 * Currently, yat_lib still lacks automated support for real-time
 * tasks, but internaly it is thread-safe, and thus can be used together
 * with pthreads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include gettid() */
#include <sys/types.h>

/* Include sysconf() */
#include <unistd.h>

/* Include threading support. */
#include <pthread.h>

/* Include the YAT^RT API.*/
#include "yat.h"
#include "core_and_priority.h"  // 我们的调度逻辑

#define PERIOD            1000
#define RELATIVE_DEADLINE 1000
#define EXEC_COST         50

/* Let's create 10 threads in the example,
 * for a total utilization of 1.
 */
#define NUM_THREADS      10

/* The information passed to each thread. Could be anything. */
struct thread_context {
	int id;
    int shed_priority;
    int partiton;
};

/* The real-time thread program. Doesn't have to be the same for
 * all threads. Here, we only have one that will invoke job().
 */
void* rt_thread(void *tcontext);

/* Declare the periodically invoked job.
 * Returns 1 -> task should exit.
 *         0 -> task should continue.
 */
int job(void);


/* Catch errors.
 */
#define CALL( exp ) do { \
		int ret; \
		ret = exp; \
		if (ret != 0) \
			fprintf(stderr, "%s failed: %m\n", #exp);\
		else \
			fprintf(stderr, "%s ok.\n", #exp); \
	} while (0)


/* Basic setup is the same as in the single-threaded example. However,
 * we do some thread initiliazation first before invoking the job.
 */
int main(int argc, char** argv)
{
	int i;
	struct thread_context ctx[NUM_THREADS];
	pthread_t             task[NUM_THREADS];

	/* The task is in background mode upon startup. */


	/*****
	 * 1) Command line paramter parsing would be done here.
	 */
    // char str[200];
    // printf("please enter the path of csv:");
    // scanf("%s",str);

    int total_partition;
    // printf("please enter the number of partition:");
    // scanf("%d",&total_partition);
	total_partition = sysconf(_SC_NPROCESSORS_ONLN);
    if (total_partition == -1) {
        perror("sysconf");
        return 1;
    }
	printf("total_partition: %d\n", total_partition);

	/*****
	 * 2) Work environment (e.g., global data structures, file data, etc.) would
	 *    be setup here.
	 */
    const char* const_str="\/root\/yat_lib\/data.csv";
    TaskGroup* group_tasks;
    int num_threads;
    group_tasks = set_up(const_str, total_partition, &num_threads);


	/*****
	 * 3) Initialize YAT^RT.
	 *    Task parameters will be specified per thread.
	 */
	init_yat();


	/*****
	 * 4) Launch threads.
	 */
	int idx;
	// find the priority and partition
	for (int p = 0; p < total_partition; p++)
	{
		for (int j = 0; j < group_tasks[p].group_size; j++)
		{
			idx = group_tasks[p].tasks[j].id - 1;
			ctx[idx].id = idx;
			ctx[idx].shed_priority = group_tasks[p].tasks[j].priority;
			ctx[idx].partiton = group_tasks[p].tasks[j].partition;

			pthread_create(task + idx, NULL, rt_thread, (void *)(ctx + idx));
		}
	}

	// 	for (i = 0; i < NUM_THREADS; i++) {
	// 	ctx[i].id = i;
    //     // find the priority and partition
    //     for(int p=0;p<total_partition;p++){
    //         for (int j = 0; j < group_tasks[p].group_size; j++) {
    //             if(ctx[i].id==group_tasks[p].tasks[j].id-1) {
    //                 ctx[i].shed_priority = group_tasks[p].tasks[j].priority;
    //                 ctx[i].partiton=group_tasks[p].tasks[j].partition;
    //             }
    //         }
    //     }
	// 	pthread_create(task + i, NULL, rt_thread, (void *) (ctx + i));
	// }


	/*****
	 * 5) Wait for RT threads to terminate.
	 */
	for (i = 0; i < num_threads; i++)
		pthread_join(task[i], NULL);


	/*****
	 * 6) Clean up, maybe print results and stats, and exit.
	 */
    for (int i = 0; i < total_partition; i++) {
        for (int j = 0; j < group_tasks[i].group_size; j++) {
            free(group_tasks[i].tasks[j].resource_required_index);
            free(group_tasks[i].tasks[j].number_of_access_in_one_release);
        }
    }
    free(group_tasks);
	return 0;
}



/* A real-time thread is very similar to the main function of a single-threaded
 * real-time app. Notice, that init_rt_thread() is called to initialized per-thread
 * data structures of the YAT^RT user space libary.
 */
void* rt_thread(void *tcontext){
	int do_exit;
	struct thread_context *ctx = (struct thread_context *) tcontext;
	struct rt_task param;

	/* Set up task parameters */
	init_rt_task_param(&param);

	param.exec_cost = ms2ns(EXEC_COST);
	param.period = ms2ns(PERIOD);
	param.relative_deadline = ms2ns(RELATIVE_DEADLINE);

	/* What to do in the case of budget overruns? */
	param.budget_policy = NO_ENFORCEMENT;

	/* The task class parameter is ignored by most plugins. */
	param.cls = RT_CLASS_SOFT;

	/* The priority parameter is only used by fixed-priority plugins. */
	param.priority = ctx->shed_priority;

	/* Make presence visible. */
	printf("RT Thread %d active.\n", ctx->id);

	/* 1) Initialize real-time settings. */
	CALL( init_rt_thread() );

	/* To specify a partition, do
	 *
	 * param.cpu = CPU;
	 * be_migrate_to(CPU);
	 *
	 * where CPU ranges from 0 to "Number of CPUs" - 1 before calling
	 * set_rt_task_param().
	 */
    param.cpu = ctx->partiton;
    be_migrate_to_cpu(ctx->partiton);
	CALL( set_rt_task_param(gettid(), &param) );

	/* 2) Transition to real-time mode. */
	CALL( task_mode(YAT_RT_TASK) );
	/* The task is now executing as a real-time task if the call didn't fail. */

	/* 3) Invoke real-time jobs. */
	do {
		sleep_next_period();  /* Wait until the next job is released. */
		do_exit = job();  /* Invoke job. */
	} while (!do_exit);

	/* 4) Transition to background mode. */
	CALL( task_mode(BACKGROUND_TASK) );

	return NULL;
}


// job函数设置

// (1)====================== insertsort

// void insertsort_initialize( unsigned int *array );
// void insertsort_init( void );
// int insertsort_return( void );
// void insertsort_main( void );
// int job( void );

// /*
//   Declaration of global variables
// */
// unsigned int insertsort_a[ 11 ];
// int insertsort_iters_i, insertsort_min_i, insertsort_max_i;
// int insertsort_iters_a, insertsort_min_a, insertsort_max_a;

// /*
//   Initialization- and return-value-related functions
// */

// void insertsort_initialize( unsigned int *array )
// {

//   register volatile int i;
//   _Pragma( "loopbound min 11 max 11" )
//   for ( i = 0; i < 11; i++ )
//     insertsort_a[ i ] = array[ i ];

// }


// void insertsort_init()
// {
//   unsigned int a[ 11 ] = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};

//   insertsort_iters_i = 0;
//   insertsort_min_i = 100000;
//   insertsort_max_i = 0;
//   insertsort_iters_a = 0;
//   insertsort_min_a = 100000;
//   insertsort_max_a = 0;

//   insertsort_initialize( a );
// }

// int insertsort_return()
// {
//   int i, returnValue = 0;

//   _Pragma( "loopbound min 11 max 11" )
//   for ( i = 0; i < 11; i++ )
//     returnValue += insertsort_a[ i ];

//   return ( returnValue + ( -65 ) ) != 0;
// }


// /*
//   Main functions
// */


// void _Pragma( "entrypoint" ) insertsort_main()
// {
//   int  i, j, temp;
//   i = 2;

//   insertsort_iters_i = 0;

//   _Pragma( "loopbound min 9 max 9" )
//   while ( i <= 10 ) {

//     insertsort_iters_i++;

//     j = i;

//     insertsort_iters_a = 0;

//     _Pragma( "loopbound min 1 max 9" )
//     while ( insertsort_a[ j ] < insertsort_a[ j - 1 ] ) {
//       insertsort_iters_a++;

//       temp = insertsort_a[ j ];
//       insertsort_a[ j ] = insertsort_a[ j - 1 ];
//       insertsort_a[ j - 1 ] = temp;
//       j--;
//     }

//     if ( insertsort_iters_a < insertsort_min_a )
//       insertsort_min_a = insertsort_iters_a;
//     if ( insertsort_iters_a > insertsort_max_a )
//       insertsort_max_a = insertsort_iters_a;

//     i++;
//   }

//   if ( insertsort_iters_i < insertsort_min_i )
//     insertsort_min_i = insertsort_iters_i;
//   if ( insertsort_iters_i > insertsort_max_i )
//     insertsort_max_i = insertsort_iters_i;
// }

// int job( void )
// {
//   insertsort_init();
//   insertsort_main();
//   return ( insertsort_return() );
// }

// (2)====================== recursion

/*
   Global Variables
*/
// int recursion_result;
// int recursion_input;

// /*
//   Forward declaration of functions
// */
// int recursion_fib( int i );
// void recursion_main( void );
// void recursion_init( void );
// int recursion_return( void );
// int job ( void );


// void recursion_init()
// {
//   int volatile temp_input = 10;
//   recursion_input = temp_input;
// }


// int recursion_fib( int i )
// {
//   if ( i == 0 )
//     return 1;
//   if ( i == 1 )
//     return 1;

//   return recursion_fib( i - 1 ) + recursion_fib( i - 2 );
// }

// int recursion_return()
// {
//   return ( recursion_result  + ( -89 ) ) != 0;
// }

// void _Pragma( "entrypoint" ) recursion_main( void )
// {
//   _Pragma( "marker recursivecall" )
//   _Pragma( "flowrestriction 1*fib <= 177*recursivecall" )
//   recursion_result = recursion_fib( recursion_input );
// }

// int job( void )
// {
//   recursion_init();
//   recursion_main();
//   return ( recursion_return() );
// }


// (3)====================== filterbank

/*
  Forward declaration of functions
*/

// void filterbank_init( void );
// void filterbank_main( void );
// int filterbank_return( void );
// void filterbank_core( float r[ 256 ],
//                       float y[ 256 ],
//                       float H[ 8 ][ 32 ],
//                       float F[ 8 ][ 32 ] );


// /*
//   Declaration of global variables
// */

// static int filterbank_return_value;
// static int filterbank_numiters;


// /*
//   Initialization- and return-value-related functions
// */

// void filterbank_init( void )
// {
//   filterbank_numiters = 2;
// }


// int filterbank_return( void )
// {
//   return filterbank_return_value;
// }


// /*
//   Core benchmark functions
// */

// void _Pragma( "entrypoint" ) filterbank_main( void )
// {
//   float r[ 256 ];
//   float y[ 256 ];
//   float H[ 8 ][ 32 ];
//   float F[ 8 ][ 32 ];

//   int i, j;

//   _Pragma( "loopbound min 256 max 256" )
//   for ( i = 0; i < 256; i++ )
//     r[ i ] = i + 1;

//   _Pragma( "loopbound min 32 max 32" )
//   for ( i = 0; i < 32; i++ ) {

//     _Pragma( "loopbound min 8 max 8" )
//     for ( j = 0; j < 8; j++ ) {
//       H[ j ][ i ] = i * 32 + j * 8 + j + i + j + 1;
//       F[ j ][ i ] = i * j + j * j + j + i;
//     }
//   }

//   _Pragma( "loopbound min 2 max 2" )
//   while ( filterbank_numiters-- > 0 )
//     filterbank_core( r, y, H, F );

//   filterbank_return_value = ( int )( y[ 0 ] ) - 9408;
// }


// /* the FB core gets the input vector (r) , the filter responses H and F and */
// /* generates the output vector(y) */
// void filterbank_core( float r[ 256 ],
//                       float y[ 256 ],
//                       float H[ 8 ][ 32 ],
//                       float F[ 8 ][ 32 ] )
// {
//   int i, j, k;

//   _Pragma( "loopbound min 256 max 256" )
//   for ( i = 0; i < 256; i++ )
//     y[ i ] = 0;

//   _Pragma( "loopbound min 8 max 8" )
//   for ( i = 0; i < 8; i++ ) {
//     float Vect_H[ 256 ]; /* (output of the H) */
//     float Vect_Dn[ ( int ) 256 / 8 ]; /* output of the down sampler; */
//     float Vect_Up[ 256 ]; /* output of the up sampler; */
//     float Vect_F[ 256 ]; /* this is the output of the */

//     /* convolving H */
//     _Pragma( "loopbound min 256 max 256" )
//     for ( j = 0; j < 256; j++ ) {
//       Vect_H[ j ] = 0;
//       _Pragma( "loopbound min 1 max 32" )
//       for ( k = 0; ( ( k < 32 ) & ( ( j - k ) >= 0 ) ); k++ )
//         Vect_H[ j ] += H[ i ][ k ] * r[ j - k ];
//     }

//     /* Down Sampling */
//     _Pragma( "loopbound min 32 max 32" )
//     for ( j = 0; j < 256 / 8; j++ )
//       Vect_Dn[ j ] = Vect_H[ j * 8 ];

//     /* Up Sampling */
//     _Pragma( "loopbound min 256 max 256" )
//     for ( j = 0; j < 256; j++ )
//       Vect_Up[ j ] = 0;
//     _Pragma( "loopbound min 32 max 32" )
//     for ( j = 0; j < 256 / 8; j++ )
//       Vect_Up[ j * 8 ] = Vect_Dn[ j ];

//     /* convolving F */
//     _Pragma( "loopbound min 256 max 256" )
//     for ( j = 0; j < 256; j++ ) {
//       Vect_F[ j ] = 0;
//       _Pragma( "loopbound min 1 max 32" )
//       for ( k = 0; ( ( k < 32 ) & ( ( j - k ) >= 0 ) ); k++ )
//         Vect_F[ j ] += F[ i ][ k ] * Vect_Up[ j - k ];
//     }

//     /* adding the results to the y matrix */

//     _Pragma( "loopbound min 256 max 256" )
//     for ( j = 0; j < 256; j++ )
//       y[ j ] += Vect_F[ j ];
//   }
// }


// /*
//   Main function
// */

// int job( void )
// {
//   filterbank_init();
//   filterbank_main();

//   return filterbank_return();
// }

// (4)====================== simple++

int i = 0;
int job(void)
{
	/* Do real-time calculation. */
    i++;
    if (i >= 5) {
        return 1;
    }
	/* Don't exit. */
	return 0;
}
