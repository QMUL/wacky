/**
* @brief wacky_sbj_obj.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#ifndef WACKY_SBJ_OBJ_MKL_HPP
#define WACKY_SBJ_OBJ_MKL_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

#include <omp.h>

#include "mkl_vml.h"

#include "string_utils.hpp"
#include "wacky_math.hpp"
#include "wacky_misc.hpp"

//! given a verb, peform the statistics on its subjects
void read_subjects(std::string verb, std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    std::vector< std::vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    std::vector<float> & base_vector,
    std::vector<float> & add_vector,
    std::vector<float> & min_vector,
    std::vector<float> & max_vector,
    std::vector<float> & krn_vector);

//! return all the intranstive stats
void intrans_count( std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SUBJECTS,
  std::vector< std::vector<float> > & WORD_VECTORS);
 
//! return the transitive stats
void trans_count(  std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SBJ_OBJ,
  std::vector< std::vector<float> > & WORD_VECTORS);

//! Return all the stats
void all_count(std::string results_file,
    std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SBJ_OBJ,
	std::vector< std::vector<int> > & VERB_SUBJECTS,
  std::vector< std::vector<float> > & WORD_VECTORS);
 
#endif
