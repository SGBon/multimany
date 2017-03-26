/* query multiplatforms and execute kernel on all of them */
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "clutil.h"

#define IMAGE_LENGTH (1000)
#define FILTER_LENGTH (3)

int main(int argc, char **argv){
  cl_device_type device_type = CL_DEVICE_TYPE_ALL;
  if(argc > 1){
    switch(atoi(argv[1])){
    default:
    case 0:
      device_type = CL_DEVICE_TYPE_ALL;
      break;
    case 1:
      device_type = CL_DEVICE_TYPE_CPU;
      break;
    case 2:
      device_type = CL_DEVICE_TYPE_GPU;
      break;
    }
  }

  cl_int err;

  /* platform and devices */
  cl_platform_id *platforms = NULL;
  cl_uint num_platforms;
  cl_device_id **devices = NULL;
  cl_uint *num_devices;

  /* context and context specific variables */
  cl_context *contexts = NULL;
  cl_command_queue **commands = NULL;
  cl_program *programs = NULL;
  cl_kernel *kernels = NULL;

  char *kernel_source = NULL;
  size_t **workloads = NULL;

  read_cl_source("multi.cl",&kernel_source);

  /* get platforms on machine */
  clGetPlatformIDs(0,NULL,&num_platforms);
  platforms = malloc(sizeof(cl_platform_id)*num_platforms);
  err = clGetPlatformIDs(num_platforms,platforms,&num_platforms);
  if(err){
    print_error("clGetPlatformIDs()",err);
    exit(EXIT_FAILURE);
  }

  /* get devices on each platform */
  devices = malloc(sizeof(cl_device_id *)*num_platforms);
  num_devices = malloc(sizeof(cl_uint)*num_platforms);

  for(unsigned int i = 0; i < num_platforms; ++i){
    clGetDeviceIDs(platforms[i],device_type,0,NULL,&num_devices[i]);
    if(num_devices[i] > 0){
      devices[i] = malloc(sizeof(cl_device_id)*num_devices[i]);
      err = clGetDeviceIDs(platforms[i],device_type,num_devices[i],
        devices[i],&num_devices[i]);

      if(err){
        print_error("clGetDeviceIDs()",err);
        exit(EXIT_FAILURE);
      }
    }
  }

  /* allocate memory for contexts */
  contexts = malloc(sizeof(cl_context)*num_platforms);
  commands = malloc(sizeof(cl_command_queue *)*num_platforms);
  programs = malloc(sizeof(cl_program)*num_platforms);
  kernels = malloc(sizeof(cl_kernel)*num_platforms);

  /* create context for each platform */
  for(unsigned int i = 0; i < num_platforms;++i){
    if(num_devices[i] > 0){
      contexts[i] = clCreateContext(NULL,num_devices[i],devices[i],NULL,NULL,&err);
      if(err){
        print_error("clCreateContext",err);
        exit(EXIT_FAILURE);
      }

      /* create command queue for each device on platform */
      commands[i] = malloc(sizeof(cl_command_queue)*num_devices[i]);
      for(unsigned int j = 0; j < num_devices[i];++j){
        commands[i][j] = clCreateCommandQueue(contexts[i],devices[i][j],0,&err);
        if(err){
          print_error("clCreateCommandQueue()",err);
          exit(EXIT_FAILURE);
        }
      }

      programs[i] = clCreateProgramWithSource(contexts[i],1,(const char **)&kernel_source,NULL,&err);
      err = clBuildProgram(programs[i],0,NULL,NULL,NULL,NULL);
      if(err){
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(programs[i],devices[i][0],CL_PROGRAM_BUILD_LOG,sizeof(buffer),buffer,&len);
        printf("%s\n",buffer);
        exit(EXIT_FAILURE);
      }
    }
  }
  free_cl_source(kernel_source);

  /* create random 1 dimensional image and filter */
  float h_image[IMAGE_LENGTH];
  float h_filter[FILTER_LENGTH];
  float h_result[IMAGE_LENGTH];

  printf("creating image:\n[");
  for(unsigned int i = 0; i < IMAGE_LENGTH; ++i){
    h_image[i] = (float)(rand() % 255);
    printf("%f ",h_image[i]);
    h_result[i] = 0;
  }
  printf("]\n");

  /* filter will compute derivative of image */
  printf("creating fitler:\n[");
  for(int i = 0; i < FILTER_LENGTH; ++i){
    float val = i - (FILTER_LENGTH - 1) / 2;
    h_filter[i] = val;
    printf("%f ",h_filter[i]);
  }
  printf("]\n");

  /* create buffers for each context */
  cl_mem *d_image;
  cl_mem *d_filter;
  cl_mem *d_result;

  d_image = malloc(sizeof(cl_mem)*num_platforms);
  d_filter = malloc(sizeof(cl_mem)*num_platforms);
  d_result = malloc(sizeof(cl_mem)*num_platforms);
  kernels = malloc(sizeof(cl_kernel)*num_platforms);

  const size_t image_len = IMAGE_LENGTH;
  const unsigned int filter_len = FILTER_LENGTH;

  /* set aside workloads */
  workloads = malloc(sizeof(size_t *) * num_platforms);
  size_t last_work_offset = 0;
  size_t num_total_workers = 0;

  for(unsigned int i = 0; i < num_platforms; ++i){
    workloads[i] = malloc(sizeof(size_t) * num_devices[i]);
    for(unsigned int j = 0; j < num_devices[i];++j){
      ++num_total_workers;
    }
  }

  printf("Total workers in cluster %lu\n",num_total_workers);

  for(unsigned int i = 0; i < num_platforms; ++i){
    if(num_devices[i] > 0){
      d_image[i] = clCreateBuffer(contexts[i],CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*IMAGE_LENGTH,h_image,NULL);
      d_filter[i] = clCreateBuffer(contexts[i],CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*FILTER_LENGTH,h_filter,NULL);
      d_result[i] = clCreateBuffer(contexts[i],CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*IMAGE_LENGTH,h_result,NULL);
      kernels[i] = clCreateKernel(programs[i],"one_d_conv",&err);
      if(err){
        print_error("clCreateKernel()",err);
        exit(EXIT_FAILURE);
      }
      /* send kernel arguments */
      err = clSetKernelArg(kernels[i],0,sizeof(cl_mem),&d_image[i]);
      err = clSetKernelArg(kernels[i],1,sizeof(cl_mem),&d_filter[i]);
      err = clSetKernelArg(kernels[i],2,sizeof(unsigned int),&filter_len);
      err = clSetKernelArg(kernels[i],3,sizeof(cl_mem),&d_result[i]);

      /* write buffers to global memory on each device */
      for(unsigned int j = 0; j < num_devices[i]; ++j){
        workloads[i][j] = image_len/num_total_workers; /* equal division scheme */
        if(i == num_platforms - 1 && j == num_devices[i] - 1){
          workloads[i][j] = image_len - last_work_offset;
        }
        err = clEnqueueWriteBuffer(commands[i][j],d_image[i],CL_FALSE,0,sizeof(float)*image_len,h_image,0,NULL,NULL);
        err = clEnqueueWriteBuffer(commands[i][j],d_filter[i],CL_FALSE,0,sizeof(float)*filter_len,h_filter,0,NULL,NULL);

        /* enqueue kernel for execution */
        err = clEnqueueNDRangeKernel(commands[i][j],kernels[i],1,&last_work_offset,&workloads[i][j],NULL,0,NULL,NULL);

        last_work_offset += workloads[i][j];
      }
    }
  }

  last_work_offset = 0;
  /* wait for all commands to finish execution */
  for(unsigned int i = 0; i < num_platforms;++i){
    for(unsigned int j = 0; j < num_devices[i]; ++j){
      /* enqueue a read that's blocking to the same kernel */
      err = clEnqueueReadBuffer(commands[i][j],d_result[i],CL_TRUE,sizeof(float)*last_work_offset,sizeof(float)*workloads[i][j],h_result+last_work_offset,0,NULL,NULL);
      last_work_offset += workloads[i][j];
    }
  }

  /* check final result */
  printf("Result of convolution:\n[");
  for(unsigned int i = 0; i < image_len; ++i){
    printf("%f ",h_result[i]);
  }
  printf("]\n");

  /* free all of the memory */
  for(unsigned int i = 0; i < num_platforms; ++i){
    if(num_devices[i] > 0){
      clReleaseProgram(programs[i]);
      clReleaseContext(contexts[i]);
      clReleaseMemObject(d_image[i]);
      clReleaseMemObject(d_filter[i]);
      clReleaseMemObject(d_result[i]);
      for(unsigned int j = 0; j < num_devices[i]; ++j){
        clReleaseDevice(devices[i][j]);
        clReleaseCommandQueue(commands[i][j]);
      }
      free(workloads[i]);
      free(commands[i]);
      free(devices[i]);
    }
  }
  free(workloads);
  free(kernels);
  free(programs);
  free(contexts);
  free(commands);
  free(devices);
  free(num_devices);
  free(platforms);
  return EXIT_SUCCESS;
}
