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
int  read_subject_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SUBJECTS);

//! read the total word count file
int read_total_file(std::string OUTPUT_DIR, size_t & TOTAL_COUNT );

//! read the statistics on verb subject object counts
int  read_sim_stats(std::string OUTPUT_DIR, std::set<std::string> & VERB_TRANSITIVE, std::set<std::string> & VERB_INTRANSITIVE );

//! read the dictionary
int   read_dictionary(std::string OUTPUT_DIR, std::map<std::string,int> & DICTIONARY_FAST, std::vector<std::string> & DICTIONARY, size_t & VOCAB_SIZE);

//! read the similarity file (Sim3500 for example)
int  read_sim_file(std::string simverb_file, std::vector<VerbPair> & VERBS_TO_CHECK);

//! read the frequency count
int   read_freq(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED, std::set<std::string> & ALLOWED_BASIS_WORDS);

//! read in the count vectors
int  read_count(std::string OUTPUT_DIR, std::map<std::string, size_t> & FREQ, std::vector<std::string> & DICTIONARY, std::vector<int>  & BASIS_VECTOR, std::vector< std::vector<float> > & WORD_VECTORS, size_t TOTAL_COUNT, std::set<int> & WORDS_TO_CHECK );

//! read in the count vectors raw
int  read_count_raw(std::string OUTPUT_DIR, std::vector<std::string> & DICTIONARY, std::vector<int>  & BASIS_VECTOR, std::vector< std::vector<float> > & WORD_VECTORS, std::set<int> & WORDS_TO_CHECK );

//! read in the insist words
int  read_insist_words(std::string OUTPUT_DIR, std::set<std::string> & INSIST_BASIS_WORDS);

//! read the subject object pairs for verbs
int  read_subject_object_file(std::string OUTPUT_DIR, std::vector< std::vector<int> > & VERB_SBJ_OBJ);

//! Read all the words in our verbs to check and their subjects objects to restrict the set for transitive
void generate_words_to_check(std::set<int> & WORDS_TO_CHECK, std::vector< std::vector<int> > & VERB_SBJ_OBJ, std::vector< std::vector<int> > & VERB_SUBJECTS, std::vector< std::vector<int> > & VERB_OBJECTS, std::vector<VerbPair> & VERBS_TO_CHECK, std::map<std::string,int> DICTIONARY_FAST );

#endif
