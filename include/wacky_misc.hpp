/**
* @brief Misc functions that get used in various places
* @file wacky_misc.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 03/02/2017
*
*/

#ifndef WACKY_MISC_HPP
#define WACKY_MISC_HPP

#include <iostream>
#include <string>
#include <omp.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

//! represents two verbs to compare and their human ranking
struct VerbPair {
  std::string v0;
  std::string v1;
  float s;
};

//! sort our frequency file
inline bool sort_freq (std::pair<std::string,size_t> i, std::pair<std::string, size_t> j) { return (i.second > j.second); }

//! breakup the ukwac into managable chunks
int breakup ( char ** block_pointer, size_t * block_size, boost::interprocess::file_mapping &m_file, boost::interprocess::mapped_region &region);

#endif

