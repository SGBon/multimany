#include <stdio.h>
#include <omp.h>

void output (int ID);

int main() {

  //openmp usually takes the form of...
  // #pragma omp <construct> [<clause> [clause] ...]
  omp_set_num_threads(8);
  #pragma omp parallel
  {
    int threadID = omp_get_thread_num();
    output(threadID);
  }
  printf("program end\n");
}

void output(int ID) {
  printf("hello (thread %d)\n", ID);
  printf("world (thread %d)\n", ID);
}
