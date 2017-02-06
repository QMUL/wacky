/**
* @brief Functions that deal with all the verb subject object stuff
* @file wacky_verb.hpp
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

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/filesystem.hpp>

#include <omp.h>

#include "string_utils.hpp"
#include "wacky_misc.hpp"

//! create a set of verb objects
void create_verb_objects(std::string str_buffer, std::vector<int> & verb_obj_pairs,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_OBJECTS,
    bool UNIQUE_OBJECTS,
    bool LEMMA_TIME );

//! create a set of verb subjects
void create_verb_subjects(std::string str_buffer, std::vector<int> & verb_sbj_pairs,
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME );

//! create the set of verb subject object pairs
int create_verb_subject_object(std::vector<std::string> filenames,
    std::string OUTPUT_DIR, 
    std::map<std::string,int> & DICTIONARY_FAST,
    std::vector< std::vector<int> > & VERB_SBJ_OBJ,
    std::vector< std::vector<int> > & VERB_SUBJECTS,
    std::vector< std::vector<int> > & VERB_OBJECTS,
    bool UNIQUE_OBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME );

//! create the set of statistics for how many times a verb has a subject or object
int create_simverbs(std::vector<std::string> filenames, std::string simverb_path,
    std::string OUTPUT_DIR,
    std::vector<std::string> & SIMVERBS,
    std::vector<int> & SIMVERBS_COUNT,
    std::vector<int> & SIMVERBS_OBJECTS,
    std::vector<int> & SIMVERBS_ALONE,
    bool LEMMA_TIME );


#endif
