#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/* real times over thread count
 *
 1 thread - 0.005s
2 threads - 0.004s
4 threads - 0.006s
8 threads - 0.002s
*/

#define NUM_FILES 8
#define FILENAME_LENGTH 8
#define BUFFER_SIZE 20
#define MAX_ARRAY 1000

struct student{
  char first[BUFFER_SIZE];
  char last[BUFFER_SIZE];
  float gpa;
};

/* sorts arrays of struct student */
void student_sort(struct student students[], const unsigned int length);

int main(int argc, char** argv){
  int num_threads = 2;
  if(argc > 1){
    num_threads = atoi(argv[1]);
  }

  struct student top_students[NUM_FILES]; /* array of structs of top students */
  omp_set_num_threads(num_threads);
  #pragma omp parallel
  {
    #pragma omp for
    for(unsigned int i = 1; i < NUM_FILES + 1; ++i){
      /* read in file */
      char filename[FILENAME_LENGTH];
      snprintf(filename,FILENAME_LENGTH,"%u.csv",i);
      FILE *file = fopen(filename,"r");

      struct student top;
      top.gpa = 0.0f;

      /* iterate over each line in the file */
      int read_state;
      do{
        char first[BUFFER_SIZE];
        char last[BUFFER_SIZE];
        float gpa;
        read_state = fscanf(file,"%[^,],%[^,],%f\n",first,last,&gpa);

        /* save the top student in this file */
        if(top.gpa < gpa){
          strncpy(top.first,first,BUFFER_SIZE);
          strncpy(top.last,last,BUFFER_SIZE);
          top.gpa = gpa;
        }
      }while(read_state != EOF);
      fclose(file); /* free the file's resources */

      /* copy over the top students to the array */
      memcpy(&top_students[i-1],&top,sizeof(struct student));
    }
    printf("Thread #%d finished\n",omp_get_thread_num());
    /* wait for all threads to finish */
    #pragma omp barrier
  }

  /* sort the list of top students */
  student_sort(top_students,NUM_FILES);
  for(unsigned int i = 0; i < NUM_FILES;i++){
    printf("%s %s %f\n",top_students[i].first,top_students[i].last,top_students[i].gpa);
  }

  return 0;
}

/* swap two students */
void student_swap(struct student *first, struct student *second);

void student_sort(struct student students[], const unsigned int length){
  int swapped;
  unsigned int end = length - 1;
  do{
    swapped = 0;
    for(unsigned int i = 0; i < end; ++i){
      if(students[i].gpa < students[i+1].gpa){
        student_swap(&students[i],&students[i+1]);
        swapped = 1;
      }
    }
    --end;
  }while(swapped);
}

void student_swap(struct student *first, struct student *second){
  struct student temp;
  memcpy(&temp,first,sizeof(struct student));
  memcpy(first,second,sizeof(struct student));
  memcpy(second,&temp,sizeof(struct student));
}
