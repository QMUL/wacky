/**
* @brief CUDA Math Matrix and Vector functions
* @file cuda_math.cu
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 18/10/2016
*
*/


#include "cuda_math.hpp"

// CUDA Based Matrix and vector functions implementation 

__device__ float3 normalize(float3 &v) { 
  float l = 1.0f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  float3 r;
  r.x = v.x * l;
  r.y = v.y * l;
  r.z = v.z * l;
  return r;
}

__device__ float3 subtract(float3 &lhs, float3 &rhs) {
  float3 r;
  r.x = lhs.x - rhs.x;
  r.y = lhs.y - rhs.y;
  r.z = lhs.z - rhs.z;
  return r;
}

__device__ float dot(float3 &lhs, float3 &rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

__device__ float3 multiply(float3 &lhs, float3 &rhs) {
  float3 r;
  r.x = lhs.x * rhs.x;
  r.y = lhs.y * rhs.y;
  r.z = lhs.z * rhs.z;
  return r;
}

__device__ float3 multiplyScalar(float3 &lhs, float rhs) {
  float3 r;
  r.x = lhs.x * rhs;
  r.y = lhs.y * rhs;
  r.z = lhs.z * rhs;
  return r;
}

__device__ float4 multiply(float4 &lhs, float4 &rhs) {
  float4 r;
  r.x = lhs.x * rhs.x;
  r.y = lhs.y * rhs.y;
  r.z = lhs.z * rhs.z;
  r.w = lhs.w * rhs.w;
  return r;
}

__device__ float4 multiplyScalar(float4 &lhs, float rhs) {
  float4 r;
  r.x = lhs.x * rhs;
  r.y = lhs.y * rhs;
  r.z = lhs.z * rhs;
  r.w = lhs.w * rhs;
  return r;
}

__device__ float3 cross(float3 &lhs, float3 &rhs) {
  float3 r;
  r.x = lhs.y*rhs.z - lhs.z*rhs.y;
  r.y = lhs.z*rhs.x - lhs.x*rhs.z;
  r.z = lhs.x*rhs.y - lhs.y*rhs.x;
  return r;
}



