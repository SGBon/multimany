/* timing - 4.455s average */

//==============================================================================
//C Code for fork() creation test
//==============================================================================
#include <stdio.h> /* the standard I/O library for C, used for printf */
#include <stdlib.h> /* contains various functions/macros, incldued here for the exit() function */
#include <unistd.h> /* POSIX library, included here for fork() */
#include <sys/types.h> /* POSIX type definitions, included here for pid_t */
#include <sys/wait.h> /* defines wait functions for processes, included here for waitpid() */

#define NFORKS 50000 /* number of processes to spawn */

/* function that does nothing productive, only declares a variable then assigns it */
void do_nothing() {
  int i;
  i= 0;
}

/* create NTHREADS processes and wait for them to finish
 * after creation. Program ends after*/
int main(int argc, char *argv[]) {
  int pid, j, status;
  pid_t processID; /* id of the process using the type from the librarys */

  /* iterate NTHREADS times. on each iteration fork a process
   * if the process is the child process, call the do_nothing() function
   * if the process is the owner process, wait for the child process to end
   * if the process has a negative value, then there was an error creating it
   */
  for (j=0; j<NFORKS; j++) {
    /* fork a new process and assign the return value of the fork() to the processID,
     * a positive non-zero process id indicates that the current process is the master
     * and the processID is of the child
     */
    if ((processID = fork()) < 0 ) {
      printf ("fork failed with error code= %d\n", processID);
      exit(0);
    }
    else if (processID ==0) {
      /* child process, call do_nothing() */
      do_nothing();
      /* exit the process with return code of 0 (success) */
      exit(0);
    }
    else {
      /* This is where the master process will be, it waits for the child process to terminated
       * using the waitpid() function. processID argument specifies the process
       * to wait for, &status is the reference to the variable for the exit status
       * of the process, finally the third argument is the options argument which is
       * a bit string of different options, set to 0 here to indicate no options
       */
      waitpid(processID, &status, 0);
    }
  }
}
