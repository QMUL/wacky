/**
* @brief wacky_read.hpp
* @file
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

int read_unk_file(std::string OUTPUT_DIR, size_t & UNK_COUNT);

void read_subject_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SUBJECTS);

int read_total_file(std::string OUTPUT_DIR, size_t & TOTAL_COUNT );

void read_sim_stats(std::string OUTPUT_DIR, std::set<std::string> & VERB_TRANSITIVE, std::set<std::string> & VERB_INTRANSITIVE );

int read_dictionary(std::string OUTPUT_DIR, std::map<std::string,int> & DICTIONARY_FAST, std::vector<std::string> & DICTIONARY, size_t & VOCAB_SIZE);

void read_sim_file(std::string OUTPUT_DIR, std::vector<VerbPair> & VERBS_TO_CHECK);

int read_freq(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED, std::set<std::string> & ALLOWED_BASIS_WORDS);

void read_count(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector<std::string> & DICTIONARY, std::vector<int>  & BASIS_VECTOR, std::vector< std::vector<float> > & WORD_VECTORS, size_t TOTAL_COUNT);

#endif
