/**
* @brief wacky_verb.hpp
* @file
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#ifndef WACKY_VERB_HPP
#define WACKY_VERB_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

void create_verb_objects(std::string str_buffer, std::vector<int> & verb_obj_pairs,
    std::map<string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_OBJECTS,
    bool UNIQUE_OBJECTS,
    bool LEMMA_TIME );

void create_verb_subjects(std::string str_buffer, std::vector<int> & verb_sbj_pairs,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME );


int create_verb_subject_object(std::vector<std::string> filenames,
    std::string OUTPUT_DIR, 
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SBJ_OBJ,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    std::vector< std::vector<int> > & VERB_OBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME );

int create_simverbs(std::vector<std::string> filenames, string simverb_path,
    vector<string> & SIMVERBS,
    vector<int> & SIMVERBS_COUNT,
    vector<int> & SIMVERBS_OBJECTS,
    vector<int> & SIMVERBS_ALONE,
    bool LEMMA_TIME );


#endif
