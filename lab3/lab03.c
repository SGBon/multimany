#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define QUEUE_LENGTH 10
#define NUM_THREADS 2
#define BUF_LEN 20

struct queue{
  double values[QUEUE_LENGTH];
  unsigned head;
  unsigned tail;
};

void queue_init(struct queue *q);
int queue_is_full(struct queue *q);

int main(){
  struct queue lats;
  struct queue longs;

  queue_init(&lats);
  queue_init(&longs);

  omp_set_num_threads(NUM_THREADS);

  const char *lat_file = "latitude.csv";
  const char *long_file = "longitude.csv";

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
      double value;
      int read_state;
      do{
        if(queue_is_full(q)){
          continue();
        }else{
          #pragma omp critical
          {
            read_state = fscanf(file,"%lf\n",&value);
            printf("%f\n",value);
          }
        }
      }while(read_state != EOF);
      fclose(file);
    }
  }

  return 0;
}


void queue_init(struct queue *q){
  memset(q,0,sizeof(struct queue));
}

int queue_is_full(struct queue *q){
  return q->tail == q->head - 1;
}
