/**
* @brief CUDA Math functions like matrices and vectors
* @file cuda_math.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 18/10/2016
*
*/

#ifndef __CUDA_MATH_HPP__
#define __CUDA_MATH_HPP__


#include <vector_types.h>

// CUDA Based Matrix and vector functions implementation 

// TODO - I think we can also use cutil_math.h

__host__ __device__ float3 normalize(float3 &v);

__host__ __device__ float3 subtract(float3 &lhs, float3 &rhs);

__host__ __device__ float dot(float3 &lhs, float3 &rhs);

__host__ __device__ float3 multiply(float3 &lhs, float3 &rhs);

__host__ __device__ float3 multiplyScalar(float3 &lhs, float rhs);

__host__ __device__ float4 multiply(float4 &lhs, float4 &rhs);

__host__ __device__ float4 multiplyScalar(float4 &lhs, float rhs);

__host__ __device__ float3 cross(float3 &lhs, float3 &rhs);

struct Matrix4{
  // TODO - could just have a func that takes both sides - faster? No this pointer needed?
  __host__ __device__ Matrix4& operator =(const Matrix4& rhs){
    cols[0] = rhs.cols[0];
    cols[1] = rhs.cols[1];
    cols[2] = rhs.cols[2];
    cols[3] = rhs.cols[3];
    return *this;
  }

  __host__ __device__ void Zero(void){
    cols[0].x = cols[0].y = cols[0].z = cols[0].w = 0;
    cols[1].x = cols[1].y = cols[1].z = cols[1].w = 0;
    cols[2].x = cols[2].y = cols[2].z = cols[2].w = 0;
    cols[3].x = cols[3].y = cols[3].z = cols[3].w = 0;
  }

  __host__ __device__ void Identity(void) {
    cols[0].y = cols[0].z = cols[0].w = 0;
    cols[1].x = cols[1].z = cols[1].w = 0;
    cols[2].x = cols[2].y = cols[2].w = 0;
    cols[3].x = cols[3].y = cols[3].z = 0;
    cols[0].x = cols[1].y = cols[2].z = cols[3].w = 1.0f;
  }

  __host__ __device__ float4 MultiplyVec(float4 &v) {
    float4 r;
    r.x = cols[0].x * v.x + cols[1].x * v.y + cols[2].x * v.z + cols[3].x * v.w;
    r.y = cols[0].y * v.x + cols[1].y * v.y + cols[2].y * v.z + cols[3].y * v.w;
    r.z = cols[0].z * v.x + cols[1].z * v.y + cols[2].z * v.z + cols[3].z * v.w;
    r.w = cols[0].w * v.x + cols[1].w * v.y + cols[2].w * v.z + cols[3].w * v.w;
    return r;
  }

  // TODO - OpenMP / Vectorised potentially, although this is likely to go onto
  // the graphics card via cuda instead
  __host__ __device__ Matrix4 operator *(const Matrix4& rhs) {
    Matrix4 m;

    // Row 0
    m.cols[0].x = cols[0].x * rhs.cols[0].x + cols[1].x * rhs.cols[0].y + cols[2].x * rhs.cols[0].z + cols[3].x * rhs.cols[0].w; 
    m.cols[1].x = cols[0].x * rhs.cols[1].x + cols[1].x * rhs.cols[1].y + cols[2].x * rhs.cols[1].z + cols[3].x * rhs.cols[1].w; 
    m.cols[2].x = cols[0].x * rhs.cols[2].x + cols[1].x * rhs.cols[2].y + cols[2].x * rhs.cols[2].z + cols[3].x * rhs.cols[2].w; 
    m.cols[3].x = cols[0].x * rhs.cols[3].x + cols[1].x * rhs.cols[3].y + cols[2].x * rhs.cols[3].z + cols[3].x * rhs.cols[3].w; 
    
    // Row 1
    m.cols[0].y = cols[0].y * rhs.cols[0].x + cols[1].y * rhs.cols[0].y + cols[2].y * rhs.cols[0].z + cols[3].y * rhs.cols[0].w; 
    m.cols[1].y = cols[0].y * rhs.cols[1].x + cols[1].y * rhs.cols[1].y + cols[2].y * rhs.cols[1].z + cols[3].y * rhs.cols[1].w; 
    m.cols[2].y = cols[0].y * rhs.cols[2].x + cols[1].y * rhs.cols[2].y + cols[2].y * rhs.cols[2].z + cols[3].y * rhs.cols[2].w; 
    m.cols[3].y = cols[0].y * rhs.cols[3].x + cols[1].y * rhs.cols[3].y + cols[2].y * rhs.cols[3].z + cols[3].y * rhs.cols[3].w; 
    
    // Row 2
    m.cols[0].z = cols[0].z * rhs.cols[0].x + cols[1].z * rhs.cols[0].y + cols[2].z * rhs.cols[0].z + cols[3].z * rhs.cols[0].w; 
    m.cols[1].z = cols[0].z * rhs.cols[1].x + cols[1].z * rhs.cols[1].y + cols[2].z * rhs.cols[1].z + cols[3].z * rhs.cols[1].w; 
    m.cols[2].z = cols[0].z * rhs.cols[2].x + cols[1].z * rhs.cols[2].y + cols[2].z * rhs.cols[2].z + cols[3].z * rhs.cols[2].w; 
    m.cols[3].z = cols[0].z * rhs.cols[3].x + cols[1].z * rhs.cols[3].y + cols[2].z * rhs.cols[3].z + cols[3].z * rhs.cols[3].w; 
    
    // Row 3
    m.cols[0].w = cols[0].w * rhs.cols[0].x + cols[1].w * rhs.cols[0].y + cols[2].w * rhs.cols[0].z + cols[3].w * rhs.cols[0].w; 
    m.cols[1].w = cols[0].w * rhs.cols[1].x + cols[1].w * rhs.cols[1].y + cols[2].w * rhs.cols[1].z + cols[3].w * rhs.cols[1].w; 
    m.cols[2].w = cols[0].w * rhs.cols[2].x + cols[1].w * rhs.cols[2].y + cols[2].w * rhs.cols[2].z + cols[3].w * rhs.cols[2].w; 
    m.cols[3].w = cols[0].w * rhs.cols[3].x + cols[1].w * rhs.cols[3].y + cols[2].w * rhs.cols[3].z + cols[3].w * rhs.cols[3].w; 
 
    return m; 
  }

  // This results in the same as gluPerspective
  __host__ __device__ void Perspective(float fov, float ratio, float near, float far) {
    Identity();
    float d  = 1.0f/tanf(fov/2.0f);
    cols[0].x = d/ratio;
    cols[1].y = d;
    cols[2].z = (near+far)/(near-far);
    cols[2].w = -1;
    cols[3].z = (2.0f*near*far) / (near -far);
    cols[3].w = 0.0f;

  }

  __host__ __device__ void LookAt(float3 &pos, float3 &look, float3 &up) {
    float3 dirv = subtract(look,pos);
    dirv = normalize(dirv);
    float3 tt = multiplyScalar(dirv,dot(up,dirv));
    float3 upv = subtract(up,tt);
    float3 sidev = cross(dirv,upv);
    Matrix4 mm;
    mm.Identity();
    mm.cols[0].x = sidev.x;
    mm.cols[1].x = sidev.y;
    mm.cols[2].x = sidev.z;

    mm.cols[0].y = upv.x;
    mm.cols[1].y = upv.y;
    mm.cols[2].y = upv.z;

    mm.cols[0].z = -dirv.x;
    mm.cols[1].z = -dirv.y;
    mm.cols[2].z = -dirv.z;

    float4 eyein;
    eyein.x = pos.x;
    eyein.y = pos.y;
    eyein.z = pos.z;

    eyein = mm.MultiplyVec(eyein);
    eyein = multiplyScalar(eyein,-1.0f);  

    cols[0] = mm.cols[0];
    cols[1] = mm.cols[1];
    cols[2] = mm.cols[2];  
    
    cols[3].x = eyein.x;
    cols[3].y = eyein.y;
    cols[3].z = eyein.z;
    
  }

  float4 cols[4];
};


#endif


