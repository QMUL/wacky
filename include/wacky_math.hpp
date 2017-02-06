/**
* @brief wacky_math.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 03/02/2017
*
*/

#ifndef WACKY_MATH_HPP
#define WACKY_MATH_HPP

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <limits>
#include <iostream>
#include <exception>

boost::numeric::ublas::vector<float> mul_vec( boost::numeric::ublas::vector<float> & v0, boost::numeric::ublas::vector<float> & v1);

boost::numeric::ublas::vector<float> krn_mul( boost::numeric::ublas::vector<float> & a, boost::numeric::ublas::vector<float> & b);

float cosine_sim(boost::numeric::ublas::vector<float> & v0, boost::numeric::ublas::vector<float> & v1);

#endif


