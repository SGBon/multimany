/* timing - 0.600s average */

//==============================================================================
//C Code for pthread_create() test
//==============================================================================
#include <pthread.h> /* the library for posix threads, required for threading */
#include <stdio.h> /* the standard I/O library for C, used for printf */
/*
 * Contains various functions and macros, used in this program for NULL macro
 * and the exit() function
 */
#include <stdlib.h>

#define NTHREADS 50000 /* the amount of threads to create */

/*
 * function that does no productive work, only declares and assigns a variable
 * then exits the thread using pthread_exit(). The function returns a void pointer
 * so that it can be used as a callback function with the pthreads library. The parameter
 * void *null is the pointer to the arguments passed to the function, in this function
 * we are not using it
 */
void *do_nothing(void *null) {
  int i;
  i=0;
  pthread_exit(NULL);
}

/* creates NTHREADS pthreads and joins each after creation, then exits */
int main(int argc, char *argv[]) {
  int rc, i, j, detachstate;
  pthread_t tid; /* id of the thread */
  pthread_attr_t attr; /* attributes for the threads */

  /* initialize the attributes for the thread. Attribute we set is the detachstate
   * as PTHREAD_CREATE_JOINABLE, which specifies that the thread can be joined
   * by another thread
   */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* iterate NTHREADS times, in each iteration create a pthread then join
   * the pthread
   */
  for (j=0; j<NTHREADS; j++) {

    /* create and run a thread. &tid is the reference to the variable to put the thread id into
     * &attr is the pthread_attr_t reference where the thread attributes are stored.
     * do_nothing is the function the thread will run when it is created. NULL
     * is the value we are sending as an argument to the function, it is usually a pointer
     * to the variable we are sending but in this case we don't want to send anything
     * to the function.
     */
    rc = pthread_create(&tid, &attr, do_nothing, NULL);
    /* if rc is a non zero value (error occured), print rc then exit prematurely */
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }

    /* join the thread with thread id of tid, the second argument is the pointer
     * to the variable for the return value. In this case we give NULL because
     * we are not expecting a return value
     */
    rc = pthread_join(tid, NULL);
    /* if rc is a non zero value (error occured), print rc then exit prematurely */
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
      }
    }

    /* frees the resources held by the attribute variable */
    pthread_attr_destroy(&attr);
    /* exit the master thread */
    pthread_exit(NULL);

}
