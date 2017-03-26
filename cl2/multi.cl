__kernel
void one_d_conv(__global float *image,
  __global float *filter,
  unsigned int filter_len,
  __global float *result)
{
  int gid = get_global_id(0);
  size_t maxlen = get_global_size(0)+get_global_offset(0);

  float sum = 0.0f;
  int offset = (filter_len - 1) / 2;
  for(unsigned int i = 0; i < filter_len; ++i){
    float source;
    float weight = filter[filter_len - i - 1];
    /* gid is on edge of image */
    if(gid < offset || (gid + i-offset) > (maxlen - offset)){
      source = image[gid]; /* just repeat pixel */
    }else{
      source = image[gid + i - offset];
    }
    sum = sum + (source * weight);
  }
  result[gid] = sum;
}
