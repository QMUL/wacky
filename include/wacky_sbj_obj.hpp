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
#include <omp.h>

#include "string_utils.hpp"
#include "wacky_math.hpp"
#include "wacky_misc.hpp"

//! given a verb, peform the statistics on its subjects
void read_subjects(std::string verb, std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    std::vector< std::vector<float> > & WORD_VECTORS,
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

//! return all the intranstive stats
void intrans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SUBJECTS,
  std::vector< std::vector<float> > & WORD_VECTORS);
 
//! return the transitive stats
void trans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SBJ_OBJ,
  std::vector< std::vector<float> > & WORD_VECTORS);
 
#endif
