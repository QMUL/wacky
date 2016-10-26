/**
* @brief Utilities header
* @file utils.hpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 26/04/2012
*
*/
#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include <stdlib.h>
#include <cstdlib>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifdef _USE_GLM
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#endif

namespace s9 {
  
  template<class T> inline std::string ToString(const T& t) {
    std::ostringstream stream;
    stream << t;
    return stream.str();
  }

  template<class T> inline T FromString(const std::string& s) {
    std::istringstream stream (s);
    T t;
    stream >> t;
    return t;
  }

  static inline std::string ToLower(const std::string input) {
    std::string tt = input;
    std::transform(tt.begin(), tt.end(), tt.begin(), ::tolower);
    return tt;
  }


  static inline std::string FilenameFromPath(const std::string &input) {
    return input.substr(input.find_last_of("\\/")+1);
  }


  static inline std::string PathFromPath(const std::string &input) {
    return input.substr(0, input.find_last_of("\\/"));
  }

  static inline bool IsAsciiString(std::string &input) {
    for(std::string::iterator it = input.begin(); it != input.end(); ++it) {
      int c = static_cast<int>(*it);
      if (c > 127){ return false; }
    }
    return true;
  }

  static inline bool IsAsciiPrintableString(std::string &input) {
    for(std::string::iterator it = input.begin(); it != input.end(); ++it) {
      char c = static_cast<char>(*it);
      if (c > 126 || c < 32){ return false; }
    }
    return true;
  }


  /**
  * String tokenize with STL
  * http://www.cplusplus.com/faq/sequences/strings/split/
  * \todo test split
  */

  static inline std::vector<std::string> SplitStringChars(const std::string& input, const std::string& delimiters) {
    size_t current;
    size_t next = -1;
    std::vector<std::string> tokens;
    do {
      current = next + 1;
      next = input.find_first_of( delimiters, current );
      tokens.push_back(input.substr( current, next - current ));
    } while (next != std::string::npos);
    return tokens;
  }


  static inline std::vector<std::string> SplitStringWhitespace(const std::string& input ) { 
    size_t start = 0;
    size_t end = 0;
    std::vector<std::string> tokens;
    
    auto w = []( char ch ) { return std::isspace<char>( ch, std::locale::classic() ); };

    while (end < input.length()) {
         
      if (start != end && w(input.at(end))){
        tokens.push_back(input.substr( start, end ));
        start = end;
      }

      if (w(input.at(start))){
        start++;
      }
      end++;
    }
    
    if (start != end) {
      tokens.push_back(input.substr( start, end ));
    }
   
    return tokens;
  }


  static inline std::vector<std::string> SplitStringString(const std::string& input, const std::string& delimiter) {
    size_t current;
    size_t next = -1;
    std::vector<std::string> tokens;
    do {
      current = next + 1;
      next = input.find( delimiter, current );
      tokens.push_back(input.substr( current, next - current ));
    } while (next != std::string::npos);
    return tokens;
  }

  static inline bool StringContains (const std::string& input, const std::string& contains){
    size_t found = input.find(contains);
    return found != std::string::npos;
  }

  static inline bool StringBeginsWith (const std::string& input, const std::string& contains){
    size_t found = input.find(contains);
    return found != std::string::npos && found == 0;
  }

  /**
  * Remove a char from a string - returns a copy
  */

  static inline std::string RemoveChar (const std::string s, const char c) {
    std::string str (s);
    str.erase (std::remove(str.begin(), str.end(), c), str.end());
    return str;
  }

  /*
  * Basic text file reading
  */

  std::string inline TextFileRead(std::string filename) {
    std::string line;
    std::string rval;
    std::ifstream myfile (filename.c_str());
    if (myfile.is_open()){
      while ( myfile.good() ) {
        getline (myfile,line);
        rval += line +"\n";
      }
      myfile.close();
    } else 
      std::cerr << "SEBURO - Unable to open text file " << filename << std::endl;
    return rval;
  }

  /*
  * Print Binary Data
  */

  inline char *itob(int x) {
    static char buff[sizeof(int) * CHAR_BIT + 1];
    int i;
    int j = sizeof(int) * CHAR_BIT - 1;
    buff[j] = 0;
    for(i=0;i<sizeof(int) * CHAR_BIT; i++) {
      if(x & (1 << i))
      buff[j] = '1';
    else
      buff[j] = '0';
      j--;
    }
    return buff;
  }


  /*
  * String Trimming functions
  */
  // trim from start

  static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends
  static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }


#ifdef _USE_GLM
  /**
  * Print out a GLM Matrix
  */

  static inline std::string MatrixToString(const glm::mat4 &mat){
    std::stringstream s;
    int i,j;
    for (j=0; j<4; ++j){
      for (i=0; i<4; ++i){
        s << std::setprecision(2) << std::setfill('0') << std::setw(5) << mat[i][j] << " ";
      }
      s << std::endl;
    }
    return s.str();
  }


  /// return a string from a GLM Vector 2
  static inline std::string VecToString (const glm::vec2 &vec) {
    std::stringstream s;
    s << vec.x << ", " << vec.y;
    return s.str();
  }

  /// return a string from a GLM Vector 3
  static inline std::string VecToString (const glm::vec3 &vec) {
    std::stringstream s;
    s << vec.x << ", " << vec.y << ", " << vec.z;
    return s.str();
  }

  /// return a string from a GLM Vector 4
  static inline std::string VecToString (const glm::vec4 &vec) {
    std::stringstream s;
    s << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
    return s.str();
  }
#endif
}
#endif
