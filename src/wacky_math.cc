
#include "wacky_math.hpp"

using namespace boost::numeric;

ublas::vector<float> _mul_vec( ublas::vector<float> & v0, ublas::vector<float> & v1) {
    
  ublas::vector<float> r  (v0.size());
  for (int i = 0; i < v0.size(); ++i){
    r[i] = v0[i] * v1[i];
  }
  return r;
}
 
// Special case of the Kronecker product for single vectors
ublas::vector<float> _krn_mul( ublas::vector<float> & a, ublas::vector<float> & b) {
  
  ublas::vector<float> r ( a.size() * a.size());

  for (int i=0; i < a.size(); ++i){
    for (int j=0; j < a.size(); ++j){
      r[(i*a.size())+j] = a[i] * b[j];
    } 
  }

  return r;
}


float cosine_sim(ublas::vector<float> & v0, ublas::vector<float> & v1) {
  float dist = -1.0;
  
  float dot = 0;
  float l0 = 0;
  float l1 = 0;
  for (int i =0 ; i< BASIS_SIZE; ++i){
    dot += v0[i] * v1[i];
    l0 += v0[i] * v0[i];
    l1 += v1[i] * v1[i];
  }

  float n0 = sqrt(l0);
  float n1 = sqrt(l1);

  float d = n0 * n1;

  // TODO - EPSILON CHECK HERE
  if (d != 0.0f) {
    float sim = dot / d;
    dist = acos(sim) / M_PI;
  }

  return 1.0 - dist;

}


