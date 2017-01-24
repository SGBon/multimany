#include <stdio.h>
#include <omp.h>

static long num_steps = 1000000;
double step;

int main() {
  double pi;
  double sum = 0.0;

  //change in x (i.e. width of rectangle)
  step = 1.0/(double)num_steps;

  omp_set_num_threads(2);

  //calculate the summation of F(x)
  // (i.e. sum of rectangles)
  //in the approximation of pi
  printf("Number of threads in workgroup: %d\n",omp_get_num_threads());
  #pragma omp parallel
  {
    #pragma omp for reduction(+:sum)
    for (int i=0; i < num_steps; i++) {
      //calculate height
      const double x = (i+0.5)*step;
      sum += 4/(1.0+x*x); //sum F(x)
      printf("Thread #%d\r",omp_get_thread_num());
    }
  }
  pi = step * sum;
  printf("pi = %.50lf\n", pi);
}
