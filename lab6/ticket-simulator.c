/* Run times of the program
 *  Agents  Seats Oversell  Real    CPU
 *  5       100   10        0.002s  0.001s
 *  5       2500  25        0.009s  0.008s
 *  50      2500  25        0.011s  0.010s
 */

#define _XOPEN_SOURCE 600 /* required for barriers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* globals for threads */
unsigned int tickets_sold = 0;
pthread_mutex_t ticket_mutex;
pthread_barrier_t arg_barrier; /* barrier for passing arguments */

#define BARRIER_SIZE 2

struct agent_params{
  unsigned int tid;
  unsigned int tickets_available;
};

void *ticket_agent(void *param);

int main(int argc, char** argv){
  if(argc < 4){
    printf("Usage: %s [Agents] [Seats] [Oversell]\n",argv[0]);
  }

  /* parse command line arguments */
  const unsigned int numthreads = atoi(argv[1]);
  const unsigned int numseats = atoi(argv[2]);
  const double oversell = atof(argv[3]);
  const unsigned int tickets_available = numseats * (oversell/100.0 + 1.0);

  pthread_t threads[numthreads];
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

  pthread_mutex_init(&ticket_mutex,NULL);
  pthread_barrier_init(&arg_barrier,NULL,BARRIER_SIZE);

  for(unsigned int i = 0; i < numthreads; ++i){
    struct agent_params params = {i,tickets_available};
    pthread_create(&threads[i],&attr,ticket_agent,&params);
    /* wait until thread has the arguments before looping */
    pthread_barrier_wait(&arg_barrier);
  }
  pthread_barrier_destroy(&arg_barrier);

  for(unsigned int i = 0; i < numthreads; ++i){
    pthread_join(threads[i],NULL);
  }

  printf("%u tickets sold out of %u available tickets\n",tickets_sold,tickets_available);

  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&ticket_mutex);
  return 0;
}

void *ticket_agent(void *param){
  struct agent_params thread_info;
  memcpy(&thread_info,param,sizeof(struct agent_params));
  pthread_barrier_wait(&arg_barrier); /* signal that we are done getting the arguments */

  const float threshold = (thread_info.tid & 1) ? 0.35 : 0.45;

  while(thread_info.tickets_available > tickets_sold){
    const float chance = ((float)rand()/RAND_MAX);
    if(chance < threshold){
      /* perform successful transaction */
      unsigned int to_sell = (rand() % 4) + 1;

      /* critical section */
      pthread_mutex_lock(&ticket_mutex);
      {
        if(tickets_sold + to_sell > thread_info.tickets_available){
          to_sell = thread_info.tickets_available - tickets_sold;
        }
        tickets_sold += to_sell;
      }
      pthread_mutex_unlock(&ticket_mutex);
      /* end of critical section */

      printf("Ticket Agent <%u>: Successful transaction - <%u> tickets sold\n",thread_info.tid,to_sell);
    }else{
      /* report unsuccessful transaction */
      printf("Ticket Agent <%u>: Unsuccessful transaction\n",thread_info.tid);
    }
  }

  pthread_exit(NULL);
}
