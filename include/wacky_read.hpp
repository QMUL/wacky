/**
* @brief Functions for reading in existing various wacky files
* @file wacky_read.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#ifndef WACKY_READ_HPP
#define WACKY_READ_HPP

#include <vector>
#include <map>
#include <set>
#include <string>

#include "wacky_misc.hpp"
#include "string_utils.hpp"

//! read the unknown count file
int read_unk_file(std::string OUTPUT_DIR, size_t & UNK_COUNT);

//! read the file containing verb subjects
void  read_subject_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SUBJECTS);

//! read the total word count file
int read_total_file(std::string OUTPUT_DIR, size_t & TOTAL_COUNT );

//! read the statistics on verb subject object counts
void  read_sim_stats(std::string OUTPUT_DIR, std::set<std::string> & VERB_TRANSITIVE, std::set<std::string> & VERB_INTRANSITIVE );

//! read the dictionary
int   read_dictionary(std::string OUTPUT_DIR, std::map<std::string,int> & DICTIONARY_FAST, std::vector<std::string> & DICTIONARY, size_t & VOCAB_SIZE);

//! read the similarity file (Sim3500 for example)
void  read_sim_file(std::string OUTPUT_DIR, std::vector<VerbPair> & VERBS_TO_CHECK);

//! read the frequency count
int   read_freq(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED, std::set<std::string> & ALLOWED_BASIS_WORDS);

//! read in the count vectors
void  read_count(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector<std::string> & DICTIONARY, std::vector<int>  & BASIS_VECTOR, std::vector< std::vector<float> > & WORD_VECTORS, size_t TOTAL_COUNT);

//! read the subject object pairs for verbs
void  read_subject_object_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SBJ_OBJ);

#endif
