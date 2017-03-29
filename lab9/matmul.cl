__kernel void matmul(
  const int m_dim, const int k_dim, const int n_dim,
  __global float *a, __global float *b, __global float *result)
{
  int rid = get_global_id(0);
  int cid = get_global_id(1);
  float temp = 0.0;
  for(int k = 0; k < k_dim ; ++k){
    temp += a[rid*k_dim + k] * b[cid + k*n_dim];
  }
  result[rid*n_dim+cid] = temp;
}
