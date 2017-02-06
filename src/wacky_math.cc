/**
* @brief Functions for things like cosine distance and kronecker product
* @file wacky_math.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#include "wacky_math.hpp"

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
    r[i] = v0[i] * v1[i];
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
      r[(i*a.size())+j] = a[i] * b[j];
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
    dot += v0[i] * v1[i];
    l0 += v0[i] * v0[i];
    l1 += v1[i] * v1[i];
  }

	// I really dont like using exceptions but here is an (*arf*) exceptional case! :D 
	try {
  	float n0 = sqrt(l0);
  	float n1 = sqrt(l1);

  	float d = n0 * n1;

  	if (std::abs(d) <= std::numeric_limits<float>::epsilon()) {
    	float sim = dot / d;
    	dist = acos(sim) / M_PI;
  	}
	} catch (std::exception &e){
		std::cout << "Exception occured at cosine_similarity: " << e.what() <<  std::endl;
	}

  return 1.0 - dist;

}
