#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <ctype.h>

#include "error.h"
#include "functions.h"

#define MAX_RAND_VALUE 500
#define ZERO_DIFF ((double) 1e-9) 

unsigned long initial_time;

unsigned long time_in_millis()
{
  struct timeval tv;
  if(gettimeofday(&tv,NULL))
  {
    print_error(__func__,__FILE__,__LINE__,errno);
    return 0;
  }
  return tv.tv_sec*1000+tv.tv_usec/1000;
}

typedef struct
{
  unsigned int id;
  double* vector1;
	double* vector2;
  int position;
  int elems;
  double sum;
	
} thread_ctx;

typedef struct
{
  pthread_mutex_t mtx;
  pthread_cond_t vc;

  unsigned int next_turn;
  unsigned int n_threads;
	unsigned int t;
} synchro_ctx;

synchro_ctx synchro_obj={
	.t=0,
  .mtx=PTHREAD_MUTEX_INITIALIZER,
  .vc=PTHREAD_COND_INITIALIZER,
  .next_turn=0
};

void* thread(void* thread_arg)
{
  thread_ctx* ctx=(thread_ctx*) thread_arg;

	if (synchro_obj.t){
		pthread_mutex_lock(&synchro_obj.mtx);
  	while(synchro_obj.next_turn!=ctx->id)
  	{
    	pthread_cond_wait(&synchro_obj.vc,&synchro_obj.mtx);
  	}
	}
	printf("[%06ld][%s-%u] starting\n",time_in_millis()-initial_time,__func__,ctx->id);
  printf("[%06ld][%s-%u] multiplying %d pairs of elements starting at position %d:\n",time_in_millis()-initial_time,__func__,ctx->id,ctx->elems,ctx->position);

  ctx->sum=0;
  for(int i=ctx->position; i<ctx->position+ctx->elems; i++)
  {
    printf("[%06ld][%s-%u] --> multiplying elements %f and %f at posicion: %d\n",time_in_millis()-initial_time,__func__,ctx->id,ctx->vector1[i],ctx->vector2[i], i);
    ctx->sum+=((ctx->vector1[i])*(ctx->vector2[i]));
  }

  printf("[%06ld][%s-%u] obtained sum: %f\n",time_in_millis()-initial_time,__func__,ctx->id,ctx->sum);
  printf("[%06ld][%s-%u] finishing\n",time_in_millis()-initial_time,__func__,ctx->id);

	if (synchro_obj.t){
		synchro_obj.next_turn=synchro_obj.next_turn+1;
		pthread_cond_broadcast(&synchro_obj.vc);
		pthread_mutex_unlock(&synchro_obj.mtx);
	}
  return thread_arg;
}

int main(int argc, char *argv[])
{
  initial_time=time_in_millis();
  char* program_name=strrchr(argv[0],'/')+1;

	unsigned int n_threads=5;
	synchro_obj.t=0;
  int n_elems=8;

	if (argc>1){
		for (int i=1; i<argc; i++){
			if (strcmp(argv[i], "-n")==0){
					if (i+1<argc && check_dig(argv[i+1])!=0){
						n_elems = atoi(argv[i+1]);
					}
					else printf("No has introducido un entero para el numero de elementos. Valor por defecto: 16.\n");
			}
			else if (strcmp(argv[i], "-h")==0){
				if(i+1<argc && check_dig(argv[i+1])!=0){
					n_threads = atoi(argv[i+1]);
				}
				else printf("No has introducido un entero para el número de hilos. Valor por defecto: 5.\n");
			}
			else if (strcmp(argv[i], "-t")==0)
				synchro_obj.t=1;
		}
	}

  if(n_elems<n_threads) 
  {
    print_error_warning_msg(
      "ERROR",
      __func__,
      __FILE__,
      __LINE__,
      "how many elements do you want to sum? At least must be %d!",
      n_threads
    );
    printf("\nusage: ./%s <numbers_of_elements>\n\n",program_name);
    return EXIT_FAILURE;
  }

  srand(time(NULL));
  double v1[n_elems];
	double v2[n_elems];

  printf(
    "[%06ld][%s] random vector (%d elements):\n",
    time_in_millis()-initial_time,
    __func__,
    n_elems
  );
  for(int i=0; i<n_elems; i++)
  {
    v1[i]=((double)(2*MAX_RAND_VALUE)*rand())/((double) RAND_MAX)-MAX_RAND_VALUE;
		v2[i]=((double)(2*MAX_RAND_VALUE)*rand())/((double) RAND_MAX)-MAX_RAND_VALUE;
    printf(
      "[%06ld][%s][v1] --> element %d: %f\n",
      time_in_millis()-initial_time,
      __func__,i,v1[i] 
    );
		printf(
      "[%06ld][%s][v2] --> element %d: %f\n",
      time_in_millis()-initial_time,
      __func__,i,v2[i] 
    );
  }

  pthread_t the_threads[n_threads];
  thread_ctx thread_ctxs[n_threads];
  int thread_elems=n_elems/(n_threads);
	int resto=n_elems%n_threads;

  for(int i=0; i<n_threads; i++)
  {
    thread_ctxs[i].id=i; 
    thread_ctxs[i].vector1=v1;
		thread_ctxs[i].vector2=v2;
		thread_ctxs[i].position=(i-resto>=0)?(i*thread_elems)+resto:(i*(thread_elems+1)); 
    thread_ctxs[i].elems=(i-resto>=0)?thread_elems:thread_elems+1;

		if (i==(n_threads-1))
			break;

    if(pthread_create(&the_threads[i],NULL,thread,(void*) &thread_ctxs[i]))
    {
      print_error_and_msg(
        __func__,__FILE__,__LINE__,
        errno,strerror(errno),
        "thread %d cannot be created! Aborting!",
        i
      );
      return EXIT_FAILURE;
    }
    printf(
      "[%06ld][%s] thread %d launched (position: %d, elements: %d)\n",
      time_in_millis()-initial_time,
      __func__,i,thread_ctxs[i].position,thread_ctxs[i].elems
    );
  }

	printf(
      "[%06ld][%s](position: %d, elements: %d)\n",
      time_in_millis()-initial_time,
      __func__,thread_ctxs[n_threads-1].position,thread_ctxs[n_threads-1].elems
    );

	thread((void*) &thread_ctxs[n_threads-1]);


  for(int i=0; i<n_threads-1; i++)
  {
    thread_ctx* rptr;
    printf(
      "[%06ld][%s] waiting for thread %d\n",
      time_in_millis()-initial_time,
      __func__,
      i 
    );
    if(pthread_join(the_threads[i],(void**) &rptr))
    {
      print_error_and_msg(
        __func__,__FILE__,__LINE__,
        errno,strerror(errno),
        "thread %d cannot be joined! Aborting!",
        i
      );
      return EXIT_FAILURE;
    }
    printf(
      "[%06ld][%s] thread %d joined\n",
      time_in_millis()-initial_time,
      __func__,rptr->id
    );
  }

  printf("[%06ld][%s] results:\n",time_in_millis()-initial_time,__func__);
  double parallel_sum=0;
  for(int i=0; i<n_threads; i++) parallel_sum+=thread_ctxs[i].sum;
  printf("[%06ld][%s] --> parallel sum: %f\n",time_in_millis()-initial_time,__func__,parallel_sum);
  double sequential_sum=0;
  for(int i=0; i<n_elems; i++) sequential_sum+=(v1[i]*v2[i]);
  printf("[%06ld][%s] --> sequential sum: %f\n",time_in_millis()-initial_time,__func__,sequential_sum);
  printf(
    "[%06ld][%s] --> %s\n",
    time_in_millis()-initial_time,
    __func__,
    (fabs(parallel_sum-sequential_sum)<ZERO_DIFF? "are equal, RIGHT!" : "not equal, WRONG!!") 
  );

  return EXIT_SUCCESS;
}

