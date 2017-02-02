/**
* @brief wacky_sbj_obj.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#ifndef WACKY_SBJ_OBJ_HPP
#define WACKY_SBJ_OBJ_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "string_utils.hpp"

struct VerbPair {
  string v0;
  string v1;
  float s;
};

void read_subjects(std::string verb, std::map<string,int> & DICTIONARY_FAST,
    std::vector< vector<int> > & VERB_SUBJECTS,
    std::vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    boost::numeric::ublas::vector<float> & base_vector,
    boost::numeric::ublas::vector<float> & add_vector,
    boost::numeric::ublas::vector<float> & min_vector,
    boost::numeric::ublas::vector<float> & max_vector,
    boost::numeric::ublas::vector<float> & add_base_add_vector,
    boost::numeric::ublas::vector<float> & add_base_mul_vector,
    boost::numeric::ublas::vector<float> & min_base_add_vector,
    boost::numeric::ublas::vector<float> & min_base_mul_vector,
    boost::numeric::ublas::vector<float> & max_base_add_vector,
    boost::numeric::ublas::vector<float> & max_base_mul_vector,
    boost::numeric::ublas::vector<float> & krn_vector,
    boost::numeric::ublas::vector<float> & krn_base_add_vector,
    boost::numeric::ublas::vector<float> & krn_base_mul_vector );

void intrans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<string> & VERB_TRANSITIVE,
  std::set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<string,int> & DICTIONARY_FAST,
  std::vector< vector<int> > & VERB_SUBJECTS,
  std::vector< vector<float> > & WORD_VECTORS);
 

#endif
