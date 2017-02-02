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
#include <string>

#include "wacky_sbj_obj.hpp"

int read_unk_file(std::string OUTPUT_DIR, int & UNK_COUNT);
void read_subject_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SUBJECTS);
int read_total_file(std::string OUTPUT_DIR, int & TOTAL_COUNT );
void read_sim_stats(std::string OUTPUT_DIR, std::vector<std::string> & VERB_TRANSITIVE, std::vector<std::string> & VERB_INTRANSITIVE );
int read_dictionary(std::string OUTPUT_DIR, std::map<std::string,int> & DICTIONARY_FAST, std::vector<std::string> & DICTIONARY, int & VOCAB_SIZE);
void read_sim_file(std::string OUTPUT_DIR, std::vector<VerbPair> & VERB_TO_CHECK);
int read_freq(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED, std::set<std::string> & ALLOWED_BASIS_WORDS);
void read_count(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector<std::string> & DICTIONARY, std::vector<int>  & BASIS_VECTOR, std::vector< std::vector<float> > & WORD_VECTORS, int TOTAL_COUNT);

#endif
