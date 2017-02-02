/**
* @brief wacky_create.hpp
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


int create_dictionary(std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<std::string> & DICTIONARY);


void create_basis(std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<int> & BASIS_VECTOR,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    int BASIS_SIZE,
    int IGNORE_WINDOW );

int create_freq(std::vector<std::string> filenames, 
    std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<std::string> & DICTIONARY
    std::set<std::string> & WORD_IGNORES,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    bool LEMMA_TIME);


int create_word_vectors(std::vector<std::string> filenames,
    std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<std::string> & DICTIONARY 
    std::vector<int> & BASIS_VECTOR,
    std::set<std::string> & WORD_IGNORES,
    std::vector< std::vector<float> > & WORD_VECTORS,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    int VOCAB_SIZE,
    int BASIS_SIZE,
    int WINDOW_SIZE,
    bool LEMMA_TIME);

int create_integers(std::vector<std::string> filenames,
    std::string OUTPUT_DIR,
    std::map<std::string,int> & DICTIONARY_FAST,
    int VOCAB_SIZE,
    bool LEMMA_TIME);

#endif
