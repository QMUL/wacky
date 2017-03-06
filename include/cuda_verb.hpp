#ifndef __cuda_verb_hpp__
#define __cuda_verb_hpp__

#include <cuda_runtime.h>

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


void all_count_cuda( std::vector<VerbPair> & VERBS_TO_CHECK,
  std::set<std::string> & VERB_TRANSITIVE,
  std::set<std::string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  std::map<std::string,int> & DICTIONARY_FAST,
  std::vector< std::vector<int> > & VERB_SBJ_OBJ,
	std::vector< std::vector<int> > & VERB_SUBJECTS,
  std::vector< std::vector<float> > & WORD_VECTORS);


#endif
