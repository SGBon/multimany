__kernel void matmul(
  const int m_dim, const int k_dim, const int n_dim,
  __global float *a, __global float *b, __global float *result)
{
  int rid, cid = get_global_id(0),get_global_id(1);
  float temp = 0.0;
  for(int k = 0; k < k_dim; ++k){
    temp += a[]
  }
  result[gid] = temp;
}

0 1 2 3
4 5 6 7
8 9 10 11
