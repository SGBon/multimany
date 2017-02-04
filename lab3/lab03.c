/* lab03 timings:
 * 1 thread: 0.021s
 * 2 threads: 0.027s
 * 4 threads: 0.024s
 */

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

enum queue_state{
  SUCCESS,
  FAIL
};

enum latlong_desc{
  LATITUDE,
  LONGITUDE
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

/* double to degrees minutes seconds */
void dtdms(char *buffer, size_t length, double value, enum latlong_desc lld);

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

  #pragma omp parallel shared(lats, longs, lat_finish, long_finish)
  {
    const int threadid = omp_get_thread_num();
    const int numthreads = omp_get_num_threads();
    #pragma omp sections nowait
    {
      /* latitude file reader thread */
      #pragma omp section
      {
        FILE *file = fopen(lat_file,"r");
        double value;
        while(1){
          if(queue_is_full(&lats)){
            /* if we have less than 3 threads, producers must consume as well */
            if(numthreads < 3){
              double value;
              enum queue_state qs;
              #pragma omp critical (lat_lock)
              {
                qs = dequeue(&lats,&value);
              }
              if (qs == SUCCESS){
                char buffer[BUF_LEN];
                dtdms(buffer,BUF_LEN,value,LATITUDE);
                printf("%lf converted to %s\n",value,buffer);
              }
            }
          }else{
            const int read_state = fscanf(file,"%lf\n",&value);
            if(read_state == EOF){
              break;
            }
            #pragma omp critical (lat_lock)
            {
              enqueue(&lats,value);
            }
          }
        }
        fclose(file);
        lat_finish = 0;
      }

      /* longitude file reader thread */
      #pragma omp section
      {
        FILE *file = fopen(long_file,"r");
        double value;
        while(1){
          if(queue_is_full(&longs)){
            /* if we have less than 3 threads, producers must consume as well */
            if(numthreads < 3){
              double value;
              enum queue_state qs;
              #pragma omp critical (long_lock)
              {
                qs = dequeue(&longs,&value);
              }
              if (qs == SUCCESS){
                char buffer[BUF_LEN];
                dtdms(buffer,BUF_LEN,value,LATITUDE);
                printf("%lf converted to %s\n",value,buffer);
              }
            }
          }else{
            const int read_state = fscanf(file,"%lf\n",&value);
            if(read_state == EOF){
              break;
            }
            #pragma omp critical (long_lock)
            {
              enqueue(&longs,value);
            }
          }
        }
        fclose(file);
        long_finish = 0;
      }

    }

    #pragma omp sections
    {
      /* latitude queue consumer */
      #pragma omp section
      {
        while(lat_finish || !queue_is_empty(&lats)){
          double value;
          enum queue_state qs;
          #pragma omp critical (lat_lock)
          {
            qs = dequeue(&lats,&value);
          }
          if (qs == SUCCESS){
            char buffer[BUF_LEN];
            dtdms(buffer,BUF_LEN,value,LATITUDE);
            printf("%lf converted to %s\n",value,buffer);
          }
        }
      }

      /* longitude queue consumer */
      #pragma omp section
      {
        while(long_finish || !queue_is_empty(&longs)){
          double value;
          enum queue_state qs;
          #pragma omp critical (long_lock)
          {
            qs = dequeue(&longs,&value);
          }
          if (qs == SUCCESS){
            char buffer[BUF_LEN];
            dtdms(buffer,BUF_LEN,value,LONGITUDE);
            printf("%lf converted to %s\n",value,buffer);
          }
        }
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


void dtdms(char *buffer, size_t length, double value, enum latlong_desc lld){
  int first = (int) value;
  double second = ((value - first)*100);
  double third = ((second - (int) second)*100);
  char dir;
  char neg = first < 0;
  if(lld == LATITUDE){
    dir = neg ? 'S' : 'N';
  }else{
    dir = neg ? 'W' : 'E';
  }

  snprintf(buffer,length,"%d\xc2\xb0 %d\' %d\" %c",first,(int)second,(int)third,dir);
}
