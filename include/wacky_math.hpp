/**
* @brief wacky_math.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 03/02/2017
*
*/

#ifndef WACKY_MATH_HPP
#define WACKY_MATH_HPP

#ifdef _USE_MKL
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "mkl.h"
#include "mkl_vml.h"
#else
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#endif

#include <limits>
#include <iostream>
#include <exception>

#ifdef _USE_MKL
void mul_vec_slow( std::vector<float> & v0, std::vector<float> & v, std::vector<float> &r);
void krn_mul( std::vector<float> & a, std::vector<float> & b, std::vector<float> &r);
float cosine_sim(std::vector<float> & v0, std::vector<float> & v1, int size);
#else
boost::numeric::ublas::vector<float> mul_vec( boost::numeric::ublas::vector<float> & v0, boost::numeric::ublas::vector<float> & v1);
boost::numeric::ublas::vector<float> krn_mul( boost::numeric::ublas::vector<float> & a, boost::numeric::ublas::vector<float> & b);
float cosine_sim(boost::numeric::ublas::vector<float> & v0, boost::numeric::ublas::vector<float> & v1);

#endif

#endif


