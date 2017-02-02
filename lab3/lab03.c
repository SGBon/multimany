#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define QUEUE_LENGTH 10
#define NUM_THREADS 4
#define BUF_LEN 20

struct queue{
  double values[QUEUE_LENGTH];
  unsigned head;
  unsigned tail;
};

enum queue_state{
  SUCCESS,
  FAIL
};

/* initialize the queue */
void queue_init(struct queue *q);

/* check state of queue */
int queue_is_full(struct queue *q);
int queue_is_empty(struct queue *q);

/* put value into queue */
void enqueue(struct queue *q, const double value);

/* remove value from queue */
enum queue_state dequeue(struct queue *q, double *ret);

int main(){
  struct queue lats;
  struct queue longs;

  queue_init(&lats);
  queue_init(&longs);

  omp_set_num_threads(NUM_THREADS);

  const char *lat_file = "latitude.csv";
  const char *long_file = "longitude.csv";

  int lat_finish = 1;
  int long_finish = 1;

  #pragma omp parallel
  {
    const int threadid = omp_get_thread_num();
    FILE *file = NULL;
    if(threadid == 0)
      file = fopen(lat_file,"r");
    else if (threadid == 1)
      file = fopen(long_file,"r");

    /* if file isn't null, perform reading */
    if(file != NULL){
      struct queue *q = threadid == 0 ? &lats : &longs;
      int *finish_ref = threadid == 0 ? &lat_finish : &long_finish;
      double value;
      int read_state = 1;
      do{
        if(queue_is_full(q)){
          continue;
        }else{
          read_state = fscanf(file,"%lf\n",&value);
          #pragma omp critical
          {
            enqueue(q,value);
            printf("ENQUEUEING: %f\n",value);
          }
        }
      }while(read_state != EOF);
      *finish_ref = 0;
      fclose(file);
    }

    printf("Thread %d arriving at process\n",threadid);

    /* divide the queues amongst the threads based on
     * the thread id being even/odd
     */
    struct queue *q = threadid & 1 ? &lats : &longs;
    int *finish_ref = threadid & 1 ? &lat_finish : &long_finish;

    /* process the queues */
    while(*finish_ref || !queue_is_empty(q)){
      double value;
      enum queue_state qs;
      #pragma omp critical
      {
        qs = dequeue(q,&value);
      }
      if(qs == FAIL){
        continue;
      }else if (qs == SUCCESS){
        printf("DEQUEUEING: %lf\n",value);
      }
    }
  }

  return 0;
}


void queue_init(struct queue *q){
  memset(q,0,sizeof(struct queue));
}

int queue_is_full(struct queue *q){
  return (q->tail == q->head - 1) || (q->tail == (q->head + QUEUE_LENGTH - 1));
}

int queue_is_empty(struct queue *q){
  return q->head == q->tail;
}

void enqueue(struct queue *q, const double value){
  if(!queue_is_full(q)){
    q->values[q->tail] = value;
    q->tail += 1;
    /* wrap tail around */
    if(q->tail == QUEUE_LENGTH){
      q->tail = 0;
    }
  }
}

enum queue_state dequeue(struct queue *q, double *ret){
  if(queue_is_empty(q)){
    return FAIL;
  }else{
    *ret = q->values[q->head];
    q->head += 1;
    /* wrap head around */
    if(q->head == QUEUE_LENGTH){
      q->head = 0;
    }
    return SUCCESS;
  }
}