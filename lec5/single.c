#include <stdio.h>
#include <omp.h>

int main() {
  int i;
  int a;
  int x[10];

  #pragma omp parallel shared(a,x) private(i)
  {
    #pragma omp single
    {
      a = 20;
      printf("single construct! (thread %d)\n",omp_get_thread_num());
    }
    #pragma omp for
    for(i = 0;i < 10; ++i){
      x[i] = a*i;
    }
  }

  printf("Results:\n");
  for(i = 0;i < 10; ++i){
    printf("x[%d] = %d\n",i,x[i]);
  }

  return 0;
}
