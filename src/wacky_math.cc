/**
* @brief Functions for things like cosine distance and kronecker product
* @file wacky_math.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#include "wacky_math.hpp"

#ifdef _USE_MKL

using namespace std;


/**
 * Create the Kronecker product for the special case of two 1 dimensional vectors
 * @param a a std vector of float
 * @param b a std vector of float
 */

void krn_mul( vector<float> & a, vector<float> & b, vector<float> & r) {
  
/*  float alpha = 1, beta = 1;
  CBLAS_LAYOUT layout = CblasRowMajor;
  CBLAS_TRANSPOSE transA = CblasNoTrans, transB = CblasNoTrans;

  MKL_INT m,n,k;
  m = a.size();
  n = b.size();
  k = 1;

  MKL_INT lda, ldb, ldc;
  lda = n;
  ldb = n;
  ldc = n;

  cblas_sgemm(layout, transA, transB, m, n, k, alpha, &a[0], lda, &b[0], ldb, beta, &r[0], ldc );
*/
  //for (int i=0; i < a.size(); ++i){
  //  for (int j=0; j < a.size(); ++j){
  //    r[(i*a.size())+j] = a[i] * b[j];
  //  } 
  //}

  std::fill(r.begin(), r.end(), 0.);

  MKL_INT n = a.size();

  for (int i = 0; i < b.size(); ++i){
    cblas_saxpy(n, b[i], &a[0], 1, &r[i*b.size()], 1); 
  }

}

/**
 * Find the cosine similarity between two vectors
 * @param v0 a std vector of float
 * @param v1 a std vector of float
 * @return a float from 1.0 to 0.0 or 2.0 if there was an error
 */

float cosine_sim(vector<float> & v0, vector<float> & v1, int size) {
  float dist = -1.0;  
  float dot = 0;
  float l0 = 0;
  float l1 = 0;

  for (int i =0; i < size; ++i){
    dot += v0[i] * v1[i];
    l0 += v0[i] * v0[i];
    l1 += v1[i] * v1[i];
  }

  // I really dont like using exceptions but here is an (*arf*) exceptional case! :D 
  try {
    float n0 = sqrt(l0);
    float n1 = sqrt(l1);
    float d = n0 * n1;

    //cout << "n0,n1,d,s0,s1,l0,l1 " << n0 << "," << n1 << "," << d << "," << v0.size() << "," << v1.size() << "," << l0 << "," << l1 <<  endl;

    // I removed the epsilon check here but not sure if that was right to do
    if (d != 0.0f) {
      float sim = dot / d;
      dist = acos(sim) / M_PI;
    }
  } catch (std::exception &e){
    std::cout << "Exception occured at cosine_similarity: " << e.what() <<  std::endl;
  }

  return 1.0 - dist;

}

#else

using namespace boost::numeric;

/**
 * Multiply vectors together
 * @param v0 a ublas vector of float
 * @param v1 a ublas vector of float
 * @return a new ublas vector of float where v0 * v1
 */

ublas::vector<float> mul_vec( ublas::vector<float> & v0, ublas::vector<float> & v1) {
    
  ublas::vector<float> r  (v0.size());
  for (int i = 0; i < v0.size(); ++i){
    r(i) = v0(i) * v1(i);
  }
  return r;
}

/**
 * Create the Kronecker product for the special case of two 1 dimensional vectors
 * @param a a ublas vector of float
 * @param b a ublas vector of float
 * @return a new ublas vector of float where a (x) b
 */

ublas::vector<float> krn_mul( ublas::vector<float> & a, ublas::vector<float> & b) {
  
  ublas::vector<float> r ( a.size() * a.size());

  for (int i=0; i < a.size(); ++i){
    for (int j=0; j < a.size(); ++j){
      r((i*a.size())+j) = a(i) * b(j);
    } 
  }

  return r;
}

/**
 * Find the cosine similarity between two vectors
 * @param v0 a ublas vector of float
 * @param v1 a ublas vector of float
 * @return a float from 1.0 to 0.0 or 2.0 if there was an error
 */

float cosine_sim(ublas::vector<float> & v0, ublas::vector<float> & v1) {
  float dist = -1.0;  
  float dot = 0;
  float l0 = 0;
  float l1 = 0;

  for (int i =0; i < v0.size(); ++i){
    dot += v0(i) * v1(i);
    l0 += v0(i) * v0(i);
    l1 += v1(i) * v1(i);
  }

  // I really dont like using exceptions but here is an (*arf*) exceptional case! :D 
  try {
    float n0 = sqrt(l0);
    float n1 = sqrt(l1);

    float d = n0 * n1;

    // I removed the epsilon check here but not sure if that was right to do
    if (d != 0.0f) {
      float sim = dot / d;
      dist = acos(sim) / M_PI;
    }
  } catch (std::exception &e){
    std::cout << "Exception occured at cosine_similarity: " << e.what() <<  std::endl;
  }

  return 1.0 - dist;

}
#endif
