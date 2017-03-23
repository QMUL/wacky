/**
* @brief wacky_create.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#ifndef WACKY_CREATE_HPP
#define WACKY_CREATE_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

#include <omp.h>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/filesystem.hpp>

#include "string_utils.hpp"
#include "wacky_misc.hpp"

std::vector<std::string>::iterator find_in_dictionary(std::vector<std::string> & DICTIONARY, std::string s);

//! create a dictionary
int create_dictionary(std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<std::string> & DICTIONARY,
    size_t VOCAB_SIZE);

//! create our basis vector
void create_basis(std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<int> & BASIS_VECTOR,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    std::set<std::string> & INSIST_WORDS,
    size_t BASIS_SIZE,
    size_t IGNORE_WINDOW );

//! create a frequency of all the words in ukwac
int create_freq(std::vector<std::string> filenames, 
    std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::set<std::string> & WORD_IGNORES,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    bool LEMMA_TIME);

//! create our word vectors for later testing
int create_word_vectors(std::vector<std::string> filenames,
    std::string OUTPUT_DIR,
    std::map<std::string, size_t> & FREQ, 
    std::vector< std::pair<std::string,size_t> > & FREQ_FLIPPED,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector<std::string> & DICTIONARY,
    std::vector<int> & BASIS_VECTOR,
    std::set<std::string> & WORD_IGNORES,
    std::vector< std::vector<float> > & WORD_VECTORS,
    std::set<std::string> & ALLOWED_BASIS_WORDS,
    size_t VOCAB_SIZE,
    size_t BASIS_SIZE,
    size_t WINDOW_SIZE,
    bool LEMMA_TIME);

//! create files of numbers for the tensorflow version
int create_integers(std::vector<std::string> filenames,
    std::string OUTPUT_DIR,
    std::map<std::string,int> & DICTIONARY_FAST,
    size_t VOCAB_SIZE,
    bool LEMMA_TIME);

#endif
