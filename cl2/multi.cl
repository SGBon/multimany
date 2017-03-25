__kernel
void one_d_conv(__global float *image,
  __global float *filter,
  unsigned int filter_len,
  __global float *result)
{
  int gid = get_global_id(0);
  printf("Hello from kernel %d: data: %f %d %f\n",gid,image[gid],filter_len,result[gid]);
}
