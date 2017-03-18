#include <stdio.h>
#include <stdlib.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

#define LENGTH 10

const char *kernel_source = "__kernel void vadd(__global const float *a, __global const float *b, __global float *c){int gid = get_global_id(0);c[gid] = a[gid] + b[gid];}";

int main(void){

  cl_int err;
  cl_platform_id fplat_id;
  cl_uint num_platforms;
  err = clGetPlatformIDs(1,&fplat_id,&num_platforms);

  cl_device_id device_id;
  cl_uint num_devices;
  err = clGetDeviceIDs(fplat_id,CL_DEVICE_TYPE_ALL,1,&device_id,&num_devices);

  cl_context context;
  context = clCreateContext(NULL,1,&device_id,NULL,NULL,&err);

  /* create command queue. Command queue is in order, so commands will be executed
   * in the order that they are given.
   */
  cl_command_queue commands;
  commands = clCreateCommandQueue(context,device_id,0,&err);

  /* compile kernel program */
  cl_program program;
  program = clCreateProgramWithSource(context,1,&kernel_source,NULL,&err);
  err = clBuildProgram(program,0,NULL,NULL,NULL,NULL);

  /* print error message of compilation if there are any */
  if(err != CL_SUCCESS){
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(program,device_id,CL_PROGRAM_BUILD_LOG,sizeof(buffer),buffer,&len);
    printf("%s\n",buffer);
    exit(-1);
  }

  printf("Creating vectors\n");
  float h_a[LENGTH], h_b[LENGTH], h_c[LENGTH];
  for(int i = 0; i < LENGTH; ++i){
    h_a[i] = rand()/(float)RAND_MAX;
    h_b[i] = rand()/ (float) RAND_MAX;
  }

  printf("[");
  for(int i = 0; i < LENGTH; ++i){
    printf("%f, ",h_a[i]);
  }
  printf("]\n");

  printf("[");
  for(int i = 0; i < LENGTH; ++i){
    printf("%f, ",h_b[i]);
  }
  printf("]\n");

  cl_mem d_a,d_b,d_c;
  const unsigned int count = LENGTH;

  /* create buffer objects */
  d_a = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*count,h_a,NULL);
  d_b = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*count,h_b,NULL);
  d_c = clCreateBuffer(context,CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float)*count,h_c,NULL);

  cl_kernel kernel;
  kernel = clCreateKernel(program,"vadd",&err);

  /* send arguments for kernel */
  err = clSetKernelArg(kernel,0,sizeof(cl_mem),&d_a);
  err |= clSetKernelArg(kernel,1,sizeof(cl_mem),&d_b);
  err |= clSetKernelArg(kernel,2,sizeof(cl_mem),&d_c);
  err |= clSetKernelArg(kernel,3,sizeof(unsigned int),&count);

  /* write buffers to global memory */
  err = clEnqueueWriteBuffer(commands,d_a,CL_FALSE,0,sizeof(float)*count,h_a,0,NULL,NULL);
  err = clEnqueueWriteBuffer(commands,d_b,CL_FALSE,0,sizeof(float)*count,h_b,0,NULL,NULL);

  /* enqueue kernel for execution */
  const size_t global = count;
  err = clEnqueueNDRangeKernel(commands,kernel,1,NULL,&global,NULL,0,NULL,NULL);

  /* read result of kernel execution (blocking read) */
  err = clEnqueueReadBuffer(commands,d_c,CL_TRUE,0,sizeof(float)*count,h_c,0,NULL,NULL);

  printf("final vector:\n[");
  for(int i = 0;i < LENGTH; ++i){
    printf("%f, ",h_c[i]);
  }
  printf("]\n");

  return 0;
}
